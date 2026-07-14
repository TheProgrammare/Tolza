#include "pipeline.hpp"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <functional>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iostream>

#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <common/common.hpp>
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
#include "binder/ffi_json_reader.hpp"
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

#define can_log(_pass)                                                                                                 \
  magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, common::compiler::FPass::_pass)                              \
      || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, common::compiler::FPass::all)


pipeline::Pipeline::Pipeline()
{
  auto root_cu         = std::make_unique<cu::CU>();
  root_cu->status.root = true;

  compilation_units.emplace_back(std::move(root_cu));
}


std::unique_ptr<cu::CU> pipeline::Pipeline::build_CU_from_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  assert(common::fileutils::is_velox_file(path));

  auto p = common::fileutils::get_velox_file(path);

  std::ifstream f(std::string(p).c_str());

  const std::string   data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  std::vector<size_t> last_offset_line;
  size_t              nb_newlines = std::count(data.begin(), data.end(), '\n');
  last_offset_line.reserve(nb_newlines);
  size_t offset = 0;

  for (char c : data) {
    offset++;
    if (c == '\n') last_offset_line.emplace_back(offset);
  }

  auto cuid = cu::ID::make(compiler::pipeline.compilation_units.size());

  auto ptr = std::make_unique<cu::CU>(parent_cuid, cuid, path, data, last_offset_line);

  f.close();

  return ptr;
}

std::vector<cu::ID> pipeline::Pipeline::query_CUs_at_dir(cu::ID parent_cuid, std::string_view path) noexcept
{
  auto f_founds = common::fileutils::find_velox_files(path, true);

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
  auto p = fs::path(compiler::OPTIONS.dir.get_dir_binding()) / "C";
  fs::remove_all(p);
  p = common::fileutils::get_velox_file(p.string());
  fs::remove(p);

  static bool log = can_log(binder);

  // native C language lib handler
  auto duration = timing([&]() { ffi::C_Reader::generate_libc_wrappers(); });

  if (log) {
    std::cout << "[binder:C] C wrappers generation completed  | " color_YELLOW << duration << " ms" color_RESET << "\n";
  }

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
  static bool log = can_log(lexer);

  auto& cu = cuid.get();

  static size_t count = 1;
  if (cu.file_info.data.empty()) {
    std::cerr << color_RED "[lexer:" << count << ":error] " color_RESET "the file \"" << cu.file_info.path
              << "\" is empty.\n";
    return false;
  }

  bool  success = false;
  Lexer lex(cu);

  auto duration = timing([&]() { success = lex.tokenize(); });

  if (log && success) {
    std::cout << "[lexer:" << count << "] \"" << cu.file_info.path << "\" | " << lex.stream.data().size()
              << " characters | " color_YELLOW << duration << " ms" color_RESET << "\n";
  }
  if (!success) {
    std::cerr << color_RED "[lexer:" << count << ":error] " color_RESET "\"" << cu.file_info.path << "\" " color_YELLOW
              << duration << " ms" color_RESET << "\n";
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_preprocessor(cu::ID cuid) noexcept
{
  static bool log = can_log(preprocessor);

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
  if (log && pre_success && gen_success) {
    std::cout << "[preprocessor:" << count << "] \"" << cu.file_info.path << "\" | " << gen.tokens_generated.size()
              << " tokens | " color_YELLOW << pre_duration + gen_duration << " ms" color_RESET << "\n";
  }
  if (!pre_success) {
    std::cerr << color_RED "[preprocessor:" << count << ":error] " color_RESET "\"" << cu.file_info.path
              << "\" " color_YELLOW << pre_duration << " ms" color_RESET << "\n";
  }
  if (!gen_success) {
    std::cerr << color_RED "[preprocessor:generator:" << count << ":error] " color_RESET "\"" << cu.file_info.path
              << "\" " color_YELLOW << gen_duration << " ms" color_RESET << "\n";
  }

  count++;

  return pre_success && gen_success;
}
bool pipeline::Pipeline::pass_parser(cu::ID cuid) noexcept
{
  static bool log = can_log(parser);

  bool                   success = false;
  parser::Parser_Context parser(cuid);

  auto duration = timing([&]() {
    success = parser.start_parsing();
    parser.CU.definitions->inject_to_resolved_def();
  });

  auto& cu = cuid.get();

  static size_t count = 1;
  if (log && success) {
    std::cout << "[parser:" << count << "] \"" << cu.file_info.path << "\" | " << parser.node_count
              << " nodes | " color_YELLOW << duration << " ms" color_RESET << "\n";
  }

  if (!success) {
    std::cerr << color_RED "[parser:" << count << ":error] " color_RESET "\"" << cu.file_info.path << "\" " color_YELLOW
              << duration << " ms" color_RESET << "\n";
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_binding_generation(const std::vector<std::string>& path, std::string_view alias) noexcept
{
  static bool log = can_log(binder);

  // path must specify the language, then the file
  assert(path.size() >= 2);
  fs::create_directories(compiler::OPTIONS.dir.get_dir_binding());
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
  bind_path.replace_extension(common::fileutils::VELOX_FILE_EXTENSION);
  fs::create_directories(bind_path.parent_path());

  std::fstream f(bind_path);

  auto cu = build_CU_from_path(cu::ID::main(), bind_path.string());

  // // universal ffi json
  //  auto ast = ffi::JSON_Reader::parse_json_compilation_unit(ffi_path.string());
  //  ast->velox_codegen(ast->bind.get_file_path());


  bind_path = common::fileutils::get_velox_file(bind_path.string());

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
          auto& tok = cu.file_info.tokens->get(regex->node_token_id);

          auto err = Error_Diagnostic(cuid, 249, tok.begin, tok.begin + tok.length, compiler::EPhase::shipowner,
                                      "Impossible to generate the file at \""
                                          + cu::file_path_to_str(regex->path, regex->source) + "."
                                          + std::string(common::fileutils::VELOX_FILE_EXTENSION) + "*\"",
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
  static bool log_sym = can_log(resolver_symbol);
  static bool log_ty  = can_log(resolver_type);
  static bool log_sem = can_log(resolver_semantic);

  size_t err_count  = compiler::COMPILER.errors.size();
  auto   have_error = [&]() { return compiler::COMPILER.errors.size() > err_count; };

  auto& cu = cuid.get();

  size_t sym = 0;
  size_t ty  = 0;
  size_t sem = 0;

  static size_t count = 1;

  auto sym_duration = timing([&]() { sym = pass_resolution_symbol(cuid); });
  if (log_sym) {
    std::cout << "[resolver:symbol:" << count << "] \"" << cu.file_info.path << "\" | " << sym
              << " references resolved | " << color_YELLOW << sym_duration << " ms" color_RESET << "\n";
  } else if (have_error()) {
    std::cerr << color_RED "[resolver:symbol:" << count << "] \"" << cu.file_info.path << "\" " color_YELLOW
              << sym_duration << " ms" color_RESET << "\n";
    compiler::COMPILER.print_errors();
    return false;
  }
  auto ty_duration = timing([&]() { ty = pass_resolution_inference(cuid); });
  if (log_ty) {
    std::cout << "[resolver:inference:" << count << "] \"" << cu.file_info.path << "\" | " << ty
              << " inferences resolved | " << color_YELLOW << ty_duration << " ms" color_RESET << "\n";
  } else if (have_error()) {
    std::cerr << color_RED "[resolver:inference:" << count << "] \"" << cu.file_info.path << "\" " color_YELLOW
              << ty_duration << " ms" color_RESET << "\n";
    compiler::COMPILER.print_errors();
    return false;
  }
  auto sem_duration = timing([&]() { sem = pass_resolution_semantic(cuid); });
  if (log_sem) {
    std::cout << "[resolver:semantic:" << count << "] \"" << cu.file_info.path << "\" | " << color_YELLOW
              << sem_duration << " ms" color_RESET << "\n";
  } else if (have_error()) {
    std::cerr << color_RED "[resolver:semantic:" << count << "] \"" << cu.file_info.path << "\" " color_YELLOW
              << sem_duration << " ms" color_RESET << "\n";
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
  static bool              log = can_log(codegen);
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
  if (log)
    std::cout << "[codegen:" << count << "/" << compiler::pipeline.analyzed_compilation_units.size() << "] \""
              << cu.file_info.path << "\" | " << color_YELLOW << duration << " ms" color_RESET << "\n";

  count++;

  return true;
}
bool pipeline::Pipeline::pass_llvm_emitter(cu::ID cuid) noexcept
{
  static bool log = can_log(emit);


  auto& cu = cuid.get();

  try {
    fs::create_directories(compiler::OPTIONS.dir.get_llvmir_dir());
  } catch (const std::runtime_error& e) {
    std::cerr << "[emit:ERROR] Directory creation failed: " << e.what() << "\n";
    return false;
  }


  fs::path out_llvm_file(compiler::OPTIONS.dir.get_llvmir_dir());
  fs::create_directories(out_llvm_file);
  out_llvm_file /= cu.file_info.get_file_name();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  static size_t count = 1;
  if (EC) {
    llvm::errs() << "[emit:llvm:" << count << "] Error cannot open the file: " << EC.message() << "\n";
    count++;
    return false;
  }

  cu.llvm_module->print(out_f, nullptr);

  if (log) std::cout << "[emit:llvm:" << count << "] emission of the llvm-ir to " << out_llvm_file << "\n";
  count++;

  return true;
}

bool pipeline::Pipeline::pass_llvm_optimization(cu::ID cuid) const noexcept
{
  static bool log = can_log(optimization);

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
      std::cerr << "[llvm-opti:ERROR] module is nullptr!"
                << "\n";
      success = false;
      return;
    }

    std::string verif_errs;
    {
      llvm::raw_string_ostream os(verif_errs);
      llvm::verifyModule(*mod, &os);
    } // auto flush

    if (!verif_errs.empty()) {
      llvm::errs() << "[llvm:ERROR] Module verification failed\n";
      llvm::errs() << verif_errs << "\n";
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
      std::cout << "[llvm-opti] Module is empty, stop generation";
      success = true;
      return;
    }

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt_level);
    MPM.addPass(llvm::VerifierPass());
    MPM.run(*mod, MAM);

    success = true;
    return;
  });

  if (log)
    std::cout << color_YELLOW "[llvm-opti:summary] " color_RESET "duration: " color_YELLOW << duration << " ms\n";

  return success;
}
bool pipeline::Pipeline::pass_script_emitter(cu::ID cuid) const noexcept
{
  static bool log = can_log(emit);

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
      llvm::errs() << "[emitter:ERROR] File error: " << err_c.message() << "\n";
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit\n";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    if (log) std::cout << "[emitter] Object emitted at " << dest_path << "\n";
  }
  // Emit object
  if (magic_enum::enum_flags_test(compiler::OPTIONS.target.emits, common::compiler::FEmit::Asm)) {
    fs::create_directories(compiler::OPTIONS.dir.get_dir_build());
    std::error_code err_c;
    fs::path        dest_path = fs::path(compiler::OPTIONS.dir.get_dir_build()) / cuid.get().file_info.get_file_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << "[emitter:ERROR] File error: " << err_c.message() << "\n";
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::AssemblyFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    if (log) std::cout << "[emitter] Object emitted at " << dest_path << "\n";
  }

  return true;
}
bool pipeline::Pipeline::engage_general_emitter() noexcept
{
  static bool log = can_log(emit);

  auto* mod = cu::ID::main().get().llvm_module;
  assert(mod);

  fs::create_directories(compiler::OPTIONS.dir.get_dir_build());

  std::error_code err_c;
  fs::path        dest_path = fs::path(compiler::OPTIONS.dir.get_dir_build()) / compiler::OPTIONS.get_project_name();
  dest_path.replace_extension(".o");

  llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

  if (err_c) {
    llvm::errs() << "[emitter:ERROR] File error: " << err_c.message() << "\n";
    return false;
  }


  llvm::legacy::PassManager pass;

  if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
    return false;
  }

  pass.run(*mod);
  dest.flush();

  if (log) std::cout << "[emitter] Object emitted at " << dest_path << "\n";
  return true;
}
bool pipeline::Pipeline::engage_module_linker() const noexcept
{
  static bool log = can_log(linker);

  if (compilation_units.empty()) return true;

  auto* main_mod = cu::ID::main().get().llvm_module;
  assert(main_mod);

  if (!main_mod) {
    std::cerr << "[linker:ERROR] Expected script file named 'main' to start the linking.\n";
    return false;
  }

  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (size_t i = 1; i < compilation_units.size(); ++i) {
    auto& cu = cu::ID::make(i).get();
    if (!cu.llvm_module) continue; // safe


    if (log) std::cout << "[linker] Linking module: " << cu.llvm_module->getModuleIdentifier() << "\n";

    auto module_to_link = std::unique_ptr<llvm::Module>(cu.llvm_module);
    if (llvm::Linker::linkModules(*main_mod, std::move(module_to_link))) {
      std::cerr << "[linker:ERROR] Link failed on script " << cu.file_info.path << "\n";
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
  // std::cout << "[linker] LLD linking command:\n  ld.lld";
  // for (const auto& arg : args) std::cout << " " << arg;
  // std::cout << "\n";

  std::ostringstream cmd;
  cmd << "clang " << target_o << " -o " << out_bin;
  std::cout << "[linker] clang linking command:\n  " << cmd.str() << "\n";

  // if (auto err_code = llvm::sys::ExecuteAndWait("ld.lld", {target_o.string(), "-lc", "-o", out_bin.string()});

  const std::string mode = (compiler::OPTIONS.profile.debug) ? "debug" : "release";

  if (auto err_code = std::system(cmd.str().c_str()); err_code != 0) {
    if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
      std::cerr << "@@VELOX_EXORDIUM_RESULTATI@@\n";
      std::cerr << R"({success:false,executable:"",mode:")" << mode << "\"}\n";
      std::cerr << "@@VELOX_CLAUSULA_RESULTATI@@\n";
    } else {
      std::cerr << "[linker:ERROR] Linker failed: system code error " << err_code << "\n";
    }
    return false;
  }

  if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
    std::cout << "@@VELOX_EXORDIUM_RESULTATI@@\n";
    std::cout << "{success:true,executable:" << out_bin << ",mode:\"" << mode << "\"}\n";
    std::cout << "@@VELOX_CLAUSULA_RESULTATI@@\n";
  } else {
    std::cout << "[linker] Executable created at " << out_bin << "\n";
  }
  return true;
}


#undef can_log
