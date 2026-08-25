#include "pipeline.hpp"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <functional>
#include <fstream>
#include <print>

#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>

// LLVM core
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/LLVMContext.h>

// Passes / pipeline
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Passes/OptimizationLevel.h>

// Target / codegen
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>

// Backend init
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Program.h>

// Analysis managers
#include <llvm/Analysis/LoopAnalysisManager.h>

// Linking / IR tools
#include <llvm/Linker/Linker.h>

// Project headers
#include "Neargye/magic_enum_flags.hpp"
#include "binder/ffi_c_reader.hpp"
#include "binder/binder_ffi.hpp"

#include "codegen/codegen.hpp"
#include "codegen/codegen_type.hpp"
#include "compiler/compiler.hpp"
#include <common/compiler_options.hpp>

#include "ast/ast_base.hpp"

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/definition.hpp"
#include "nexus/metacode/token_generator.hpp"
#include "nexus/module.hpp"
#include "nexus/unresolved.hpp"
#include "nexus/metacode/preprocessor.hpp"
#include "compiler/compilation_unit.hpp"

#include "lexer/lexer.hpp"
#include "parser/parser_context.hpp"

#include "resolver/resolver_semantic.hpp"
#include "resolver/resolver_symbol.hpp"
#include "resolver/resolver_inference.hpp"

namespace fs = std::filesystem;


pipeline::Pipeline::Pipeline()
{
  auto root_cu         = std::make_unique<cu::CU>();
  root_cu->status.root = true;

  compilation_units.emplace_back(std::move(root_cu));
}


std::unique_ptr<cu::CU> pipeline::Pipeline::build_CU_from_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  assert(common::fileutils::is_tolza_file(path));

  auto p = common::fileutils::get_tolza_file(path);

  std::ifstream f(std::string(p).c_str());

  const std::string   data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  std::vector<size_t> last_offset_line;
  size_t              nb_newlines = std::count(data.begin(), data.end(), '\n');
  last_offset_line.reserve(nb_newlines);
  size_t offset = 0;

  for (char c : data) {
    if (c == '\n') last_offset_line.emplace_back(offset);
    offset++;
  }

  auto cuid = cu::ID::make(compiler::pipeline.compilation_units.size());

  auto ptr = std::make_unique<cu::CU>(parent_cuid, cuid, path, data, last_offset_line);

  f.close();

  return ptr;
}

std::vector<cu::ID> pipeline::Pipeline::query_CUs_at_dir(cu::ID parent_cuid, std::string_view path) noexcept
{
  auto f_founds = common::fileutils::find_tolza_files(path, true);

  std::vector<cu::ID> compilation_units_ids;

  for (const auto& file : f_founds) {
    if (auto it = path_generated.find(file); it != path_generated.end()) {
      compilation_units_ids.emplace_back(it->second);
      continue;
    }

    auto cu   = build_CU_from_path(parent_cuid, file);
    auto cuid = cu->cuid;
    compilation_units_ids.emplace_back(cu->cuid);
    compilation_units.emplace_back(std::move(cu));

    path_generated.try_emplace(file, cuid);
    if (!prepared_compilation_units.contains(cuid)) unprepared_compilation_units.insert(cuid);
  }

  return compilation_units_ids;
}
cu::ID pipeline::Pipeline::query_CU_at_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  if (auto it = path_generated.find(path); it != path_generated.end()) return it->second;

  auto cu     = build_CU_from_path(parent_cuid, path);
  auto new_id = cu->cuid;
  compilation_units.emplace_back(std::move(cu));

  path_generated.try_emplace(std::string(path), new_id);

  return new_id;
}

double pipeline::Pipeline::timing(const std::function<void()>& f) noexcept
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}

bool pipeline::Pipeline::generate_libc_wrappers() noexcept
{
  auto p = fs::path(compiler::OPTIONS.get_dir_binding_profile()) / "C";
  fs::remove_all(p);
  p = common::fileutils::get_tolza_file(p.string());
  fs::remove(p);

  // native C language lib handler
  auto duration = timing([&]() { ffi::C_Reader::generate_libc_wrappers(); });

  print_log(::common::compiler::FPass::binder, "C",
            std::format("C wrappers generation completed | {:.2f} ms", duration));

  return true;
}

bool pipeline::Pipeline::engage_preparer(cu::ID cuid) noexcept
{
  const bool lexer_success        = pass_lexer(cuid);
  const bool preprocessor_success = pass_preprocessor(cuid);
  const bool parser_success       = pass_parser(cuid);

  auto [_, success] = prepared_compilation_units.insert(cuid);
  assert(success);

  return lexer_success && preprocessor_success && parser_success;
}

bool pipeline::Pipeline::pass_lexer(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  static size_t count = 1;
  if (cu.file_info.data.empty()) {
    print_err(common::compiler::FPass::lexer, std::format("{}:error", count),
              std::format("the file \"{}\" is empty.", cu.file_info.path));
    return false;
  }

  bool  success = false;
  Lexer lex(cu);

  auto duration = timing([&]() { success = lex.tokenize(); });

  if (success) {
    print_log(::common::compiler::FPass::lexer, std::format("{}", count),
              std::format("\"{}\" | {} characters | {:.2} ms", cu.file_info.path, lex.stream.data().size(), duration));
  }
  if (!success) {
    print_err(common::compiler::FPass::lexer, std::format("{}:error", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, duration));
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_preprocessor(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  bool                   pre_success = false;
  bool                   gen_success = false;
  metacode::Preprocessor pre(cu);
  metacode::Generator    gen(cu, pre);

  auto pre_duration = timing([&]() { pre_success = pre.start_preprocessor(); });
  auto gen_duration = timing([&]() { gen_success = gen.start_generator(); });

  // put the final generated tokens to the script tokens
  cu.file_info.tokens->tokens = gen.tokens_generated;

  static size_t count = 1;
  if (pre_success && gen_success) {
    print_log(common::compiler::FPass::preprocessor, std::format("{}", count),
              std::format("\"{}\" | {} tokens | {:.2} ms", cu.file_info.path, gen.tokens_generated.size(),
                          pre_duration + gen_duration));
  }
  if (!pre_success) {
    print_err(common::compiler::FPass::preprocessor, std::format("{}", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, pre_duration));
  }
  if (!gen_success) {
    print_err(common::compiler::FPass::preprocessor, std::format("generator:{}", count),
              std::format("\"{}\" {:.2}", cu.file_info.path, gen_duration));
  }

  count++;

  return pre_success && gen_success;
}
bool pipeline::Pipeline::pass_parser(cu::ID cuid) noexcept
{
  bool                   success = false;
  parser::Parser_Context parser(cuid);

  auto duration = timing([&]() {
    success = parser.start_parsing();
    parser.CU.definitions->inject_to_resolved_def();
  });

  auto& cu = cuid.get();

  static size_t count = 1;
  if (success) {
    print_log(common::compiler::FPass::parser, std::format("{}", count),
              std::format("\"{}\" {} nodes | {:.2} ms", cu.file_info.path, parser.node_count, duration));
  }

  if (!success) {
    print_err(common::compiler::FPass::parser, std::format("{}", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, duration));
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_binding_generation(const std::vector<std::string>& path, std::string_view alias) noexcept
{
  // path must specify the language, then the file
  assert(path.size() >= 2);
  fs::create_directories(compiler::OPTIONS.get_dir_binding_profile());
  size_t bind_count = 0;

  ffi::Bind_Package bind;
  bind.lang = path[0];
  bind.lib  = path[1];

  std::unordered_set<ast::ID, ast::ID::Hash> resolved_nodes;
  resolved_nodes.reserve(compiler::unresolved.nodes.size() / compilation_units.size());


  for (auto id : compiler::unresolved.nodes) {
    const auto* n = id.as<ast::Symbol_Qualified>();
    if (!n) continue;

    if (n->path[0] != alias) continue;

    bind.extern_items.try_emplace(std::string(n->path.back()), id);
    resolved_nodes.insert(id);
  }

  fs::path bind_path = compiler::OPTIONS.dir.get_dir_binding();
  for (const auto& i : path) bind_path /= i;
  bind_path.replace_extension(common::fileutils::TOLZA_FILE_EXTENSION);
  fs::create_directories(bind_path.parent_path());

  std::fstream f(bind_path);

  auto cu = build_CU_from_path(cu::ID::main(), bind_path.string());

  // // universal ffi json
  //  auto ast = ffi::JSON_Reader::parse_json_compilation_unit(ffi_path.string());
  //  ast->tolza_codegen(ast->bind.get_file_path());


  bind_path = common::fileutils::get_tolza_file(bind_path.string());

  compiler::pipeline.binding_compilation_units_to_prepare.insert(bind_path);


  return true;
}


size_t pipeline::Pipeline::engage_shipowner() noexcept
{
  std::unordered_set<cu::ID, cu::ID::Hash>   imported_nodes;
  std::unordered_set<ast::ID, ast::ID::Hash> exported_nodes;

  for (auto cuid : prepared_compilation_units) {
    auto& cu = cuid.get();

    imported_nodes.insert(cuid);

    exported_nodes.insert(cu.node_export);
  }

  for (auto cuid : imported_nodes) {
    auto& cu = cuid.get();

    for (const auto& [id, modid] : cu.imports) {
      // if (modid) continue;

      const auto* imp = id.as<ast::Import>();
      assert(imp);

      const auto* regex = imp->regex.as<ast::Path_Regex>();
      assert(regex);

      // binding generation query
      if (regex->source == cu::EFileSource::binding) (void)pass_binding_generation(regex->path, imp->alias);

      // resolve import module source
      auto modid_found = cu::resolve_regex_path(modid, regex->path, regex->source);
      if (!modid_found) {
        std::string out_err;
        modid_found = module::build_module_from_path(cuid, regex->path, regex->source, out_err);
        if (!modid_found) {
          auto& tok = cu.file_info.tokens->get(regex->header.start_tokid);

          auto err = Error_Diagnostic(cuid, 249, tok.begin, tok.begin + tok.length, compiler::EPhase::shipowner,
                                      std::format("Impossible to generate the file at \"{}.{}*\"",
                                                  cu::file_path_to_str(regex->path, regex->source),
                                                  std::string(common::fileutils::TOLZA_FILE_EXTENSION)),
                                      "");
          compiler::COMPILER.add_error(err);
        }
      }

      cu.imports[id] = modid_found;
    }
  }

  return true;
}

bool pipeline::Pipeline::engage_bindings() noexcept
{
  for (const auto& bind_path : binding_compilation_units_to_prepare) {
    auto cu = query_CU_at_path(cu::ID::main(), bind_path);
    if (!engage_preparer(cu)) return false;
  }

  return true;
}


bool pipeline::Pipeline::engage_analyzer(cu::ID cuid) noexcept
{
  size_t err_count  = compiler::COMPILER.errors.size();
  auto   have_error = [&]() { return compiler::COMPILER.errors.size() > err_count; };

  auto& cu = cuid.get();

  size_t sym = 0;
  size_t ty  = 0;
  size_t sem = 0;

  static size_t count = 1;

  auto sym_duration = timing([&]() { sym = pass_resolution_symbol(cuid); });
  print_log(common::compiler::FPass::resolver_symbol, std::format("{}", count),
            std::format("\"{}\" | {} references resolved | {:.2} ms", cu.file_info.path, sym, sym_duration));
  if (have_error()) {
    print_err(common::compiler::FPass::resolver_symbol, std::format("{}", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, sym_duration));
    compiler::COMPILER.print_errors();
    return false;
  }
  auto ty_duration = timing([&]() { ty = pass_resolution_inference(cuid); });
  print_log(common::compiler::FPass::resolver_type, std::format("{}", count),
            std::format("\"{}\" | {} inferences resolved | {:.2} ms", cu.file_info.path, ty, ty_duration));
  if (have_error()) {
    print_err(common::compiler::FPass::resolver_type, std::format("{}", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, ty_duration));
    compiler::COMPILER.print_errors();
    return false;
  }
  auto sem_duration = timing([&]() { sem = pass_resolution_semantic(cuid); });
  print_log(common::compiler::FPass::resolver_semantic, std::format("{}", count),
            std::format("\"{}\" | {:.2} ms", cu.file_info.path, sem_duration));
  if (have_error()) {
    print_err(common::compiler::FPass::resolver_semantic, std::format("{}", count),
              std::format("\"{}\" {:.2} ms", cu.file_info.path, sem_duration));
    compiler::COMPILER.print_errors();
    return false;
  }

  count++;

  if (!have_error()) analyzed_compilation_units.insert(cuid);
  return !have_error();
}
size_t pipeline::Pipeline::pass_resolution_symbol(cu::ID cuid) noexcept
{
  resolver::Symbol sym(cuid.get());
  return sym.start_resolver();
}
size_t pipeline::Pipeline::pass_resolution_inference(cu::ID cuid) noexcept
{
  resolver::Inference inf(cuid.get());
  return inf.start_resolver();
}
size_t pipeline::Pipeline::pass_resolution_semantic(cu::ID cuid) noexcept
{
  resolver::Semantic sem(cuid.get());
  return sem.start_resolver();
}
bool pipeline::Pipeline::engage_generator(cu::ID cuid) noexcept
{
  bool codegen_success = pass_code_generation(cuid);

  static bool once = true;
  if (once) {
    once = false;
    generate_target();
  }

  if (!codegen_success) return false;

  // llvm .ll file emission is before llvm optimization
  if (magic_enum::enum_flags_test(compiler::OPTIONS.target.emits, common::compiler::FEmit::llvm))
    (void)pass_llvm_emitter(cuid);

  if (!pass_llvm_optimization(cuid)) return false;

  return true;
}

void pipeline::Pipeline::generate_target() noexcept
{
  // Init backends
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::string target_triple = compiler::OPTIONS.target.triple.dump();

  std::string         err;
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);

  if (!target) {
    llvm::errs() << err;
    return;
  }

  llvm::Reloc::Model reloc;
  switch (compiler::OPTIONS.target.reloc_model) {
  case common::compiler::ERelocModel::STATIC:    reloc = llvm::Reloc::Static; break;
  case common::compiler::ERelocModel::PIC:       reloc = llvm::Reloc::PIC_; break;
  case common::compiler::ERelocModel::PIE:       reloc = llvm::Reloc::DynamicNoPIC; break;
  case common::compiler::ERelocModel::ROPI:      reloc = llvm::Reloc::ROPI; break;
  case common::compiler::ERelocModel::RWPI:      reloc = llvm::Reloc::RWPI; break;
  case common::compiler::ERelocModel::ROPI_RWPI: reloc = llvm::Reloc::ROPI_RWPI; break;
  case common::compiler::ERelocModel::DEFAULT:   reloc = llvm::Reloc::PIC_; break;
  }

  // Init target machine
  llvm::TargetOptions opt;
  compiler::TM =
      target->createTargetMachine(target_triple, compiler::OPTIONS.target.cpu,
                                  magic_enum::enum_flags_name(compiler::OPTIONS.target.features), opt, reloc);
}

bool pipeline::Pipeline::pass_code_generation(cu::ID cuid) noexcept
{
  static llvm::LLVMContext ctx = llvm::LLVMContext();

  static bool once = true;
  if (once) {
    once           = false;
    auto& cu       = cu::ID::main().get();
    cu.llvm_module = new llvm::Module(compiler::OPTIONS.get_project_name(), ctx);
  }

  auto& cu       = cuid.get();
  cu.llvm_module = new llvm::Module(cu.file_info.get_module_path(), ctx);


  if (compiler::OPTIONS.llvm.args.size() > 0) {
    llvm::cl::ParseCommandLineOptions(int(compiler::OPTIONS.llvm.args.size()), compiler::OPTIONS.llvm.args.data());
  }

  auto duration = timing([&]() {
    codegen::Codegen_AST cg(cu, ctx);
    (void)cg.start_codegen();
  });

  static size_t count = 1;
  print_log(common::compiler::FPass::codegen,
            std::format("{}:{}", count, compiler::pipeline.analyzed_compilation_units.size()),
            std::format("\"{}\" | {:.2} ms", cu.file_info.path, duration));

  count++;

  return true;
}
bool pipeline::Pipeline::pass_llvm_emitter(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  try {
    fs::create_directories(compiler::OPTIONS.get_dir_llvmir());
  } catch (const std::runtime_error& e) {
    print_err(common::compiler::FPass::emit, "", std::format("Directory creation failed: {}", e.what()));
    return false;
  }


  fs::path out_llvm_file(compiler::OPTIONS.get_dir_llvmir());
  fs::create_directories(out_llvm_file);
  out_llvm_file /= cu.file_info.get_file_name();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  static size_t count = 1;
  if (EC) {
    llvm::errs() << std::format("[emit:llvm:{}::ERROR] Cannot open the file: {}\n", count, EC.message());
    count++;
    return false;
  }

  cu.llvm_module->print(out_f, nullptr);

  print_log(common::compiler::FPass::emit, std::format("llvm:{}", count),
            std::format("IR emission at \"{}\"", out_llvm_file.string()));
  count++;

  return true;
}

bool pipeline::Pipeline::pass_llvm_optimization(cu::ID cuid) const noexcept
{
  if (analyzed_compilation_units.empty()) return true;

  llvm::Module* mod = cuid.get().llvm_module;
  assert(mod);

  bool success = false;

  auto duration = timing([&]() {
    // Init target
    std::string target_triple = compiler::OPTIONS.target.triple.dump();
    mod->setTargetTriple(target_triple);

    mod->setDataLayout(compiler::TM->createDataLayout());

    if (!mod) {
      print_err(common::compiler::FPass::optimization, "", "Module is nullptr!");
      success = false;
      return;
    }

    std::string verif_errs;
    {
      llvm::raw_string_ostream os(verif_errs);
      llvm::verifyModule(*mod, &os);
    } // auto flush

    if (!verif_errs.empty()) {
      llvm::errs() << std::format("[llvm:ERROR] Module verification failed\n  {}\n", verif_errs);
      success = false;
      return;
    }

    llvm::LoopAnalysisManager     LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager    CGAM;
    llvm::ModuleAnalysisManager   MAM;

    // Enregistre les analyses
    llvm::PassBuilder PB(compiler::TM);
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::OptimizationLevel opt_level = llvm::OptimizationLevel::O0;

    switch (compiler::OPTIONS.profile.optimization) {
    case common::compiler::EOptimization::O0:      opt_level = llvm::OptimizationLevel::O0; break;
    case common::compiler::EOptimization::O1:      opt_level = llvm::OptimizationLevel::O1; break;
    case common::compiler::EOptimization::O2:      opt_level = llvm::OptimizationLevel::O2; break;
    case common::compiler::EOptimization::O3:      opt_level = llvm::OptimizationLevel::O3; break;
    case common::compiler::EOptimization::Os:      opt_level = llvm::OptimizationLevel::Os; break;
    case common::compiler::EOptimization::Oz:      opt_level = llvm::OptimizationLevel::Oz; break;
    case common::compiler::EOptimization::DEFAULT: opt_level = llvm::OptimizationLevel::O0; break;
    }

    if (mod->empty()) {
      print_log(common::compiler::FPass::optimization, "", "Module is empty, stop generation");
      success = true;
      return;
    }

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt_level);
    MPM.addPass(llvm::VerifierPass());
    MPM.run(*mod, MAM);

    success = true;
    return;
  });

  print_log(common::compiler::FPass::optimization, "summary", std::format("duration: {:.2} ms", duration));

  return success;
}
bool pipeline::Pipeline::pass_script_emitter(cu::ID cuid) const noexcept
{
  if (compilation_units.empty()) return true;

  auto* mod = cu::ID::main().get().llvm_module;
  assert(mod);

  // Emit object
  if (magic_enum::enum_flags_test(compiler::OPTIONS.target.emits, common::compiler::FEmit::obj)) {
    fs::create_directories(compiler::OPTIONS.dir.get_dir_build());
    std::error_code err_c;
    fs::path        dest_path = fs::path(compiler::OPTIONS.dir.get_dir_build()) / cuid.get().file_info.get_file_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << std::format("[emitter:ERROR] File error: {}\n", err_c.message());
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit\n";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    print_log(common::compiler::FPass::emit, "", std::format("Object emitted at \"{}\"", dest_path.string()));
  }
  // Emit asm
  if (magic_enum::enum_flags_test(compiler::OPTIONS.target.emits, common::compiler::FEmit::Asm)) {
    fs::create_directories(compiler::OPTIONS.dir.get_dir_build());
    std::error_code err_c;
    fs::path        dest_path = fs::path(compiler::OPTIONS.dir.get_dir_build()) / cuid.get().file_info.get_file_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << std::format("[emitter:ERROR] File error: {}\n", err_c.message());
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    print_log(common::compiler::FPass::emit, "", std::format("Assembly emitted at \"{}\"", dest_path.string()));
  }

  return true;
}
bool pipeline::Pipeline::engage_general_emitter() noexcept
{
  auto* mod = cu::ID::main().get().llvm_module;
  assert(mod);

  fs::create_directories(compiler::OPTIONS.dir.get_dir_build());

  std::error_code err_c;
  fs::path        dest_path = fs::path(compiler::OPTIONS.dir.get_dir_build()) / compiler::OPTIONS.get_project_name();
  dest_path.replace_extension(".o");

  llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

  if (err_c) {
    llvm::errs() << std::format("[emitter:ERROR] File error: {}\n", err_c.message());
    return false;
  }


  llvm::legacy::PassManager pass;

  if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
    return false;
  }

  pass.run(*mod);
  dest.flush();

  print_log(common::compiler::FPass::emit, "", std::format("Program emitted at \"{}\"", dest_path.string()));
  return true;
}
bool pipeline::Pipeline::engage_module_linker() const noexcept
{
  if (compilation_units.empty()) return true;

  auto* main_mod = cu::ID::main().get().llvm_module;
  assert(main_mod);

  if (!main_mod) {
    print_err(common::compiler::FPass::linker, "", "Expected script file named 'main' to start the linking.");
    return false;
  }

  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (size_t i = 1; i < compilation_units.size(); ++i) {
    auto& cu = cu::ID::make(i).get();
    if (!cu.llvm_module) continue; // safe

    print_log(common::compiler::FPass::linker, "",
              std::format("Linking module: {}", cu.llvm_module->getModuleIdentifier()));

    auto module_to_link = std::unique_ptr<llvm::Module>(cu.llvm_module);
    if (llvm::Linker::linkModules(*main_mod, std::move(module_to_link))) {
      print_err(common::compiler::FPass::linker, "", std::format("Link failed on script \"{}\"", cu.file_info.path));
      failed = true;
    }
  }

  return !failed;
}

bool pipeline::Pipeline::engage_linker() noexcept
{
  std::string extension = compiler::OPTIONS.target.triple.platform == common::env::EPlatform::windows ? ".exe" : "";

  fs::create_directories(compiler::OPTIONS.dir.get_dir_build());

  fs::path target_o = fs::path(compiler::OPTIONS.dir.get_dir_build()) / compiler::OPTIONS.get_project_name();
  target_o.replace_extension(".o");
  fs::path out_bin = fs::path(compiler::OPTIONS.dir.get_dir_build()) / compiler::OPTIONS.get_project_name();
  out_bin.replace_extension(extension);

  auto* main_mod = cu::ID::main().get().llvm_module;

  // const std::vector<std::string> args = {target_o.string(), "-lc", "-o", out_bin.string()};
  //
  // std::print("[linker] LLD linking command:\n  ld.lld";
  // for (const auto& arg : args) std::print(" {}", arg);
  // std::println();

  std::string cmd = std::format(R"(clang "{}" -o "{}")", target_o.string(), out_bin.string());
  print_log(common::compiler::FPass::linker, "", std::format("Clang linking command:\n  {}", cmd));

  // if (auto err_code = llvm::sys::ExecuteAndWait("ld.lld", {target_o.string(), "-lc", "-o", out_bin.string()});

  const std::string mode = (compiler::OPTIONS.profile.debug) ? "debug" : "release";

  if (auto err_code = std::system(cmd.c_str()); err_code != 0) {
    if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
      std::println(stderr, R"(@@TOLZA_EXORDIUM_RESULTATI@@
{{"success":false,"executable":"","mode":"{}"}}
@@TOLZA_CLAUSULA_RESULTATI@@)",
                   mode);
    } else {
      print_err(common::compiler::FPass::linker, "", std::format("Linker failed: system code error {}", err_code));
    }
    return false;
  }

  if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
    std::println(R"(@@TOLZA_EXORDIUM_RESULTATI@@
{{"success":true,"executable":"{}","mode":"{}"}}
@@TOLZA_CLAUSULA_RESULTATI@@)",
                 out_bin.string(), mode);
  } else {
    print_log(common::compiler::FPass::linker, "", std::format("Executable created at \"{}\"", out_bin.string()));
  }
  return true;
}


void pipeline::Pipeline::print_log(common::compiler::FPass phase, std::string_view sub_phase,
                                   std::string_view msg) noexcept
{
  const bool can_log = magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, phase)
                       || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, common::compiler::FPass::all);

  if (!can_log) return;

  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet:   return;
  case common::compiler::ELogLevel::DEFAULT:
  case common::compiler::ELogLevel::normal:  {
    std::print("\033[1A");   // cursor up line
    std::print("\r\033[2K"); // cursor start line and clear line
    std::println("[{}{}] {}", magic_enum::enum_flags_name(phase),
                 sub_phase.empty() ? "" : std::format(":{}", sub_phase), msg);
    return;
  }
  case common::compiler::ELogLevel::verbose:
    std::println("[{}{}] {}", magic_enum::enum_flags_name(phase),
                 sub_phase.empty() ? "" : std::format(":{}", sub_phase), msg);
    return;
  }
}
void pipeline::Pipeline::print_err(common::compiler::FPass phase, std::string_view sub_phase,
                                   std::string_view msg) noexcept
{
  std::println(stderr, "[{}{}:ERROR] {}", magic_enum::enum_flags_name(phase),
               sub_phase.empty() ? "" : std::format(":{}", sub_phase), msg);
}
