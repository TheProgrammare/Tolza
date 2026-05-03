#include "pipeline.hpp"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <common.hpp>

// LLVM core
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>

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

// Analysis managers
#include <llvm/Analysis/LoopAnalysisManager.h>

// Linking / IR tools
#include <llvm/Linker/Linker.h>

// Project headers
#include "binder/c_binder.hpp"
#include "binder/ffi-json_reader.hpp"
#include "binder/binder_ffi.hpp"

#include "codegen/resolver_codegen.hpp"
#include "compiler/compiler.hpp"
#include "compiler_options.hpp"

#include "ast/ast_base.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/metacode/token_generator.hpp"
#include "nexus/module.hpp"
#include "nexus/unresolved.hpp"
#include "nexus/metacode/preprocessor.hpp"
#include "nexus/script.hpp"

#include "lexer/lexer.hpp"
#include "parser/parser_context.hpp"

#include "resolver/resolver_semantic.hpp"
#include "resolver/resolver_symbol.hpp"
#include "resolver/resolver_inference.hpp"

namespace fs = std::filesystem;


std::unique_ptr<script::ScriptInfo> pipeline::Pipeline::build_script_from_path(std::string_view path)
{
  assert(fs::exists(path));

  std::ifstream f(std::string(path).c_str());

  const std::string   data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  std::vector<size_t> last_offset_line;
  size_t              nb_newlines = std::count(data.begin(), data.end(), '\n');
  last_offset_line.reserve(nb_newlines);
  size_t offset = 0;

  for (char c : data) {
    offset++;
    if (c == '\n') last_offset_line.push_back(offset);
  }

  auto ptr = std::make_unique<script::ScriptInfo>(script::ScriptInfo(path, data, last_offset_line));

  ptr->metacodes = new metacode::ScriptGraph();
  ptr->nodes     = new ast::ScriptArena{.scr = *ptr.get()};
  ptr->nodes->add<ast::Unknown>();
  ptr->file_info.tokens = new token::Arena(*ptr.get());

  return ptr;
}

std::vector<script::_id> pipeline::Pipeline::query_scripts_at_dir(std::string_view path)
{
  auto f_founds = common::filesystem::find_velox_files(path, true);

  std::vector<script::_id> scripts_ids;

  for (auto file : f_founds) {
    if (auto it = path_generated.find(file); it != path_generated.end()) {
      scripts_ids.push_back(it->second);
      continue;
    }

    auto scr    = build_script_from_path(file);
    auto new_id = script::_id(compilation_scripts.size());
    scr->id     = new_id;
    scripts_ids.push_back(new_id);
    compilation_scripts.push_back(std::move(scr));

    path_generated.try_emplace(file, new_id);
    if (!prepared_scripts.contains(new_id)) unprepared_scripts.insert(new_id);
  }

  return scripts_ids;
}
script::_id pipeline::Pipeline::query_script_at_path(std::string_view path)
{
  if (auto it = path_generated.find(std::string(path)); it != path_generated.end()) return it->second;

  auto scr    = build_script_from_path(path);
  auto new_id = script::_id(compilation_scripts.size());
  scr->id     = new_id;
  compilation_scripts.push_back(std::move(scr));
  return new_id;
}

double pipeline::Pipeline::timing(std::function<void()> f)
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}


script::ScriptInfo& pipeline::Pipeline::get_script(script::_id scr_id)
{
  assert(scr_id.value() < compilation_scripts.size());
  auto& scr = compilation_scripts[scr_id.value()];
  return *scr.get();
}

bool pipeline::Pipeline::engage_preparer(script::_id scr_id)
{
  if (!pass_lexer(scr_id)) return false;
  if (!pass_preprocessor(scr_id)) return false;
  if (!pass_parser(scr_id)) return false;

  prepared_scripts.insert(scr_id);

  return true;
}

bool pipeline::Pipeline::pass_lexer(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("lexer") || compiler::COMPILER_OPTIONS.logs.contains("all");

  auto& scr = get_script(scr_id);

  bool  success = false;
  Lexer lex(scr);

  auto duration = timing([&]() { success = lex.tokenize(); });

  static size_t count = 1;
  if (log && success) {
    std::cout << "[lexer:" << count << "] \"" << scr.file_info.path << "\" | " << lex.stream.data().size()
              << " characters | " color_YELLOW << duration << " ms" color_RESET << std::endl;
  }
  if (!success) {
    std::cerr << color_RED "[lexer:" << count << ":error] " color_RESET "\"" << scr.file_info.path << "\" " color_YELLOW
              << duration << " ms" color_RESET << std::endl;
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_preprocessor(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("preprocessor") || compiler::COMPILER_OPTIONS.logs.contains("all");

  auto& scr = get_script(scr_id);

  bool                   pre_success = false;
  bool                   gen_success = false;
  metacode::Preprocessor pre(scr);
  metacode::Generator    gen(scr, pre);

  auto pre_duration = timing([&]() { pre_success = pre.start_preprocessor(); });
  auto gen_duration = timing([&]() { gen_success = gen.start_generator(); });

  // put the final generated tokens to the script tokens
  scr.file_info.tokens->tokens = gen.tokens_generated;

  static size_t count = 1;
  if (log && pre_success && gen_success) {
    std::cout << "[preprocessor:" << count << "] \"" << scr.file_info.path << "\" | " << gen.tokens_generated.size()
              << " tokens | " color_YELLOW << pre_duration + gen_duration << " ms" color_RESET << std::endl;
  }
  if (!pre_success) {
    std::cerr << color_RED "[preprocessor:" << count << ":error] " color_RESET "\"" << scr.file_info.path
              << "\" " color_YELLOW << pre_duration << " ms" color_RESET << std::endl;
  }
  if (!gen_success) {
    std::cerr << color_RED "[preprocessor:generator:" << count << ":error] " color_RESET "\"" << scr.file_info.path
              << "\" " color_YELLOW << gen_duration << " ms" color_RESET << std::endl;
  }

  count++;

  return pre_success && gen_success;
}
bool pipeline::Pipeline::pass_parser(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("parser") || compiler::COMPILER_OPTIONS.logs.contains("all");

  bool                   success = false;
  parser::Parser_Context parser(scr_id);

  auto duration = timing([&]() { success = parser.start_parsing(); });

  auto& scr = get_script(scr_id);

  static size_t count = 1;
  if (log && success) {
    std::cout << "[parser:" << count << "] \"" << scr.file_info.path << "\" | " << parser.node_count
              << " nodes | " color_YELLOW << duration << " ms" color_RESET << std::endl;
  }

  if (!success) {
    std::cerr << color_RED "[parser:" << count << ":error] " color_RESET "\"" << scr.file_info.path
              << "\" " color_YELLOW << duration << " ms" color_RESET << std::endl;
  }

  count++;

  return success;
}
bool pipeline::Pipeline::pass_binding_generation(std::vector<std::string_view> path, std::string_view alias)
{
  // path must specify the language, then the file
  assert(path.size() < 2);
  fs::create_directories(compiler::COMPILER_OPTIONS.get_dir_binding());
  size_t bind_count = 0;

  ffi::Bind_Package bind;
  bind.lang = path[0];
  bind.lib  = path[1];

  std::unordered_set<ast::_gnid, ast::_gnid_hash> resolved_nodes;
  resolved_nodes.reserve(compiler::COMPILER.unresolved.nodes.size() / compilation_scripts.size());


  for (auto gnid : compiler::COMPILER.unresolved.nodes) {
    auto n = compiler::COMPILER.nodes.get_as<ast::ID_Qualified>(gnid);
    if (!n) continue;

    if (n->path[0] != alias) continue;

    bind.extern_items.try_emplace(std::string(n->path.back()), gnid);
    resolved_nodes.insert(gnid);
  }


  fs::path bind_path = compiler::COMPILER_OPTIONS.get_dir_binding();
  fs::path ffi_path  = compiler::COMPILER_OPTIONS.get_dir_ffi_json();

  for (size_t i = 0; i < path.size() - 1; i++) {
    bind_path /= path[i];
    ffi_path /= path[i];
  }

  // clean binding
  std::fstream bind_file(bind_path);
  bind_file.clear();
  bind_file.close();


  if (path[0] == "C") {
    // native C language lib handler
    ffi::c::c_lib_to_velox_lib(bind);
  } else {
    // universal ffi json
    auto ast = ffi::JSON::read_ffi_json_file(ffi_path.string());
    ast.bind = bind; // will indicate what to generate with extern_items list
    ffi::write_ast(ast, bind_path.string());
  }

  return true;
}


size_t pipeline::Pipeline::engage_shipowner()
{
  std::unordered_set<script::_id, script::_id_hash> imported_nodes;
  std::unordered_set<ast::_gnid, ast::_gnid_hash>   exported_nodes;

  for (auto scr_id : prepared_scripts) {
    auto& scr = get_script(scr_id);

    imported_nodes.insert(scr_id);

    exported_nodes.insert(scr.node_export);
  }

  for (auto& scr_id : imported_nodes) {
    auto& scr = get_script(scr_id);

    for (auto& [node_id, mod_id] : scr.imports) {
      if (mod_id) continue;

      auto imp = compiler::COMPILER.nodes.get_as<ast::Import>(node_id);
      assert(imp);

      auto regex = compiler::COMPILER.nodes.get_as<ast::Path_Regex>(imp->regex);
      assert(regex);

      // binding generation query
      if (regex->source == script::EFileSource::binding)
        pass_binding_generation(regex->path, std::string(imp->aliases[0]));

      // resolve import module source
      auto mod_id_found = compiler::COMPILER.modules.crawler.find_module(regex->path, regex->source);
      if (!mod_id_found) {
        std::string out_err;
        mod_id_found =
            compiler::COMPILER.modules.tools.build_module_from_path(scr_id, regex->path, regex->source, out_err);
        if (!mod_id_found) {
          auto& tok = scr.file_info.tokens->get(regex->node_token_id);

          auto err = Error_Diagnostic(scr_id, 249, tok.begin, tok.begin + tok.length, compiler::EPhase::shipowner,
                                      "Impossible to generate the file at \""
                                          + script::file_path_to_str(regex->path, regex->source) + ".vlx*\"",
                                      "");
          compiler::COMPILER.add_error(std::move(err));
        }
      }


      scr.imports[node_id] = mod_id_found;
    }
  }

  return true;
}

bool pipeline::Pipeline::engage_analyzer(script::_id scr_id)
{
  static bool log_sym = compiler::COMPILER_OPTIONS.logs.contains("symbolic");
  static bool log_ty  = compiler::COMPILER_OPTIONS.logs.contains("type");
  static bool log_sem = compiler::COMPILER_OPTIONS.logs.contains("semantic");

  size_t err_count  = compiler::COMPILER.errors.size();
  auto   have_error = [&]() { return compiler::COMPILER.errors.size() > err_count; };

  auto& scr = get_script(scr_id);

  size_t sym = 0;
  size_t ty  = 0;
  size_t sem = 0;

  static size_t count = 1;

  auto sym_duration = timing([&]() { sym = pass_resolution_symbol(scr_id); });
  if (have_error())
    std::cerr << color_RED "[resolver:symbol:" << count << "] \"" << scr.file_info.path << "\" " color_YELLOW
              << sym_duration << " ms" color_RESET << std::endl;
  auto ty_duration = timing([&]() { ty = pass_resolution_inference(scr_id); });
  if (have_error())
    std::cerr << color_RED "[resolver:inference:" << count << "] \"" << scr.file_info.path << "\" " color_YELLOW
              << sym_duration << " ms" color_RESET << std::endl;
  auto sem_duration = timing([&]() { sem = pass_resolution_semantic(scr_id); });
  if (have_error())
    std::cerr << color_RED "[resolver:semantic:" << count << "] \"" << scr.file_info.path << "\" " color_YELLOW
              << sym_duration << " ms" color_RESET << std::endl;
  if (log_sym)
    std::cout << "[resolver:symbol:" << count << "] \"" << scr.file_info.path << "\" | " << sym
              << " references resolved | " << color_YELLOW << sym_duration << " ms" color_RESET << std::endl;
  if (log_ty)
    std::cout << "[resolver:inference:" << count << "] \"" << scr.file_info.path << "\" | " << ty
              << " inferences resolved | " << color_YELLOW << sym_duration << " ms" color_RESET << std::endl;
  if (log_sem)
    std::cout << "[resolver:semantic:" << count << "] \"" << scr.file_info.path << "\" | " << color_YELLOW
              << sym_duration << " ms" color_RESET << std::endl;

  count++;

  if (sym && ty && sem) analyzed_scripts.insert(scr_id);
  return sym && ty && sem;
}
size_t pipeline::Pipeline::pass_resolution_symbol(script::_id scr_id)
{
  auto&            scr = get_script(scr_id);
  resolver::Symbol sym(scr);
  return sym.start_resolver();
}
size_t pipeline::Pipeline::pass_resolution_inference(script::_id scr_id)
{
  auto&               scr = get_script(scr_id);
  resolver::Inference inf(scr);
  return inf.start_resolver();
}
size_t pipeline::Pipeline::pass_resolution_semantic(script::_id scr_id)
{
  auto&              scr = get_script(scr_id);
  resolver::Semantic sem(scr);
  return sem.start_resolver();
}
bool pipeline::Pipeline::engage_generator(script::_id scr_id)
{
  bool codegen_success = pass_code_generation(scr_id);

  if (codegen_success) {
    // llvm .ll file emission is before llvm optimization
    if (compiler::COMPILER_OPTIONS.target_emits.contains(common::Compiler_Options::EEmit::LLVM))
      pass_llvm_emitter(scr_id);

    pass_llvm_optimization(scr_id);

    // other kind of emission maked after optimization
    pass_script_emitter(scr_id);
  }

  return true;
}

bool pipeline::Pipeline::pass_code_generation(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("codegen") || compiler::COMPILER_OPTIONS.logs.contains("all");

  auto& scr = get_script(scr_id);

  if (compiler::COMPILER_OPTIONS.llvm_args.size() > 0) {
    llvm::cl::ParseCommandLineOptions(compiler::COMPILER_OPTIONS.llvm_args.size(),
                                      compiler::COMPILER_OPTIONS.llvm_args.data());
  }

  auto duration = timing([&]() {
    resolver::Codegen cg(scr);
    cg.start_resolver();
  });

  static size_t count = 1;
  if (log)
    std::cout << "[codegen:" << count << "/" << scr_id.value() << "] \"" << scr.file_info.path << "\"" << color_YELLOW
              << duration << " ms" color_RESET << std::endl;

  count++;

  return true;
}
bool pipeline::Pipeline::pass_llvm_emitter(script::_id scr_id)
{
  static bool log = compiler::COMPILER_OPTIONS.logs.contains("emit") || compiler::COMPILER_OPTIONS.logs.contains("all");

  auto& scr = get_script(scr_id);

  try {
    std::filesystem::create_directories(compiler::COMPILER_OPTIONS.get_llvmir_dir());
  } catch (const std::runtime_error& e) {
    std::cerr << "[emit:ERROR] Directory creation failed: " << e.what() << std::endl;
    return false;
  }


  std::filesystem::path out_llvm_file(compiler::COMPILER_OPTIONS.get_llvmir_dir());
  std::filesystem::create_directories(out_llvm_file);
  out_llvm_file /= scr.llvm_module->getName().str();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  static size_t count = 1;
  if (EC) {
    llvm::errs() << "[emit:llvm:" << count << "] Error cannot open the file: " << EC.message() << "\n";
    count++;
    return false;
  }

  scr.llvm_module->print(out_f, nullptr);
  if (log) std::cout << "[emit:llvm:" << count << "] emission of the llvm-ir to " << out_llvm_file << std::endl;
  count++;

  return true;
}

bool pipeline::Pipeline::pass_llvm_optimization(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("optimization") || compiler::COMPILER_OPTIONS.logs.contains("all");

  if (analyzed_scripts.empty()) return true;

  llvm::Module* mod = get_script(scr_id).llvm_module;
  assert(mod);

  auto duration = timing([&]() {
    // Init backends
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    // Init target
    std::string target_triple = compiler::COMPILER_OPTIONS.get_target_triple();
    mod->setTargetTriple(target_triple);

    std::string         err;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);

    if (!target) {
      llvm::errs() << err;
      return false;
    }

    llvm::Reloc::Model reloc;
    switch (compiler::COMPILER_OPTIONS.target_reloc_model) {
    case common::Compiler_Options::ERelocModel::Static:    reloc = llvm::Reloc::Static; break;
    case common::Compiler_Options::ERelocModel::PIC:       reloc = llvm::Reloc::PIC_; break;
    case common::Compiler_Options::ERelocModel::PIE:       reloc = llvm::Reloc::DynamicNoPIC; break;
    case common::Compiler_Options::ERelocModel::ROPI:      reloc = llvm::Reloc::ROPI; break;
    case common::Compiler_Options::ERelocModel::RWPI:      reloc = llvm::Reloc::RWPI; break;
    case common::Compiler_Options::ERelocModel::ROPI_RWPI: reloc = llvm::Reloc::ROPI_RWPI; break;
    }

    // Init target machine
    llvm::TargetOptions opt;
    compiler::TM = target->createTargetMachine(target_triple, compiler::COMPILER_OPTIONS.target_cpu,
                                               compiler::COMPILER_OPTIONS.target_features, opt, reloc);

    mod->setDataLayout(compiler::TM->createDataLayout());

    if (!mod) {
      std::cerr << "[llvm-opti:ERROR] module is nullptr!" << std::endl;
      return false;
    }

    if (compiler::COMPILER_OPTIONS.llvm_verify_module && llvm::verifyModule(*mod, &llvm::errs())) {
      llvm::errs() << "[llvm-opti:ERROR] Module verification failed";
      return false;
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

    switch (compiler::COMPILER_OPTIONS.profile_optimization) {
    case common::Compiler_Options::EOptimization::O0: opt_level = llvm::OptimizationLevel::O0; break;
    case common::Compiler_Options::EOptimization::O1: opt_level = llvm::OptimizationLevel::O1; break;
    case common::Compiler_Options::EOptimization::O2: opt_level = llvm::OptimizationLevel::O2; break;
    case common::Compiler_Options::EOptimization::O3: opt_level = llvm::OptimizationLevel::O3; break;
    case common::Compiler_Options::EOptimization::Os: opt_level = llvm::OptimizationLevel::Os; break;
    case common::Compiler_Options::EOptimization::Oz: opt_level = llvm::OptimizationLevel::Oz; break;
    }

    if (mod->empty()) {
      std::cout << "[llvm-opti] Module is empty, stop generation";
      return true;
    }

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt_level);
    MPM.addPass(llvm::VerifierPass());
    MPM.run(*mod, MAM);

    if (llvm::verifyModule(*mod, &llvm::errs())) {
      llvm::errs() << "[llvm-opti:ERROR] Module verification failed!\n";
      return false;
    }

    return true;
  });

  if (log)
    std::cout << color_YELLOW "[llvm-opti:summary] " color_RESET "duration: " color_YELLOW << duration << " ms\n"
              << std::endl;

  return true;
}
bool pipeline::Pipeline::pass_script_emitter(script::_id scr_id)
{
  static bool log = compiler::COMPILER_OPTIONS.logs.contains("emit") || compiler::COMPILER_OPTIONS.logs.contains("all");

  if (compilation_scripts.empty()) return true;

  auto mod = get_script(script::_id(0)).llvm_module;
  assert(mod);

  // Emit object
  if (compiler::COMPILER_OPTIONS.target_emits.contains(common::Compiler_Options::EEmit::Obj)) {
    std::error_code       err_c;
    std::filesystem::path dest_path = std::filesystem::path(compiler::COMPILER_OPTIONS.get_dir_build())
                                      / compiler::COMPILER_OPTIONS.get_project_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << "[emitter:ERROR] File error : " << err_c.message();
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine dosen't support obj emit";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    if (log) std::cout << "[emitter] Object emitted at " << dest_path << std::endl;
  }

  return true;
}
bool pipeline::Pipeline::engage_module_linker(script::_id scr_id)
{
  static bool log =
      compiler::COMPILER_OPTIONS.logs.contains("linker") || compiler::COMPILER_OPTIONS.logs.contains("all");

  if (compilation_scripts.empty()) return true;

  auto main_mod = get_script(script::_id(0)).llvm_module;
  assert(main_mod);

  if (!main_mod) {
    std::cerr << "[linker:ERROR] Expected script file named 'main' to start the linking." << std::endl;
    return false;
  }

  if (llvm::verifyModule(*main_mod, &llvm::errs())) {
    llvm::errs() << "[linker:ERROR] Module verification failed!\n";
    return false;
  }

  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (size_t i = 1; i < compilation_scripts.size(); ++i) {
    auto& scr = get_script(script::_id(i));
    if (!scr.llvm_module) continue; // safe


    llvm::outs() << "[linker] verify module " << scr.llvm_module->getName() << "\n"
                 << "  file: \"" << scr.file_info.path << "\"\n";
    if (llvm::verifyModule(*scr.llvm_module, &llvm::errs())) {
      llvm::errs() << "[linker:ERROR] Module verification \"" << scr.file_info.path << "\" failed !\n ";
      return false;
    }


    if (log) std::cout << "[linker] Linking module: " << scr.llvm_module->getModuleIdentifier() << std::endl;

    auto module_to_link = std::move(scr.llvm_module);
    if (linker.linkModules(*main_mod, std::unique_ptr<llvm::Module>(module_to_link))) {
      std::cerr << "[linker:ERROR] Link failed on script " << scr.file_info.path << std::endl;
      failed = true;
    }
  }

  if (failed) return false;

  if (llvm::verifyModule(*main_mod, &llvm::errs())) {
    llvm::errs() << "[linker:ERROR] Module verification failed!\n";
    return false;
  }

  return true;
}

bool pipeline::Pipeline::engage_linker()
{


  std::string extension = compiler::COMPILER_OPTIONS.target_os == "windows" ? ".exe" : "";

  std::filesystem::path dest_path =
      std::filesystem::path(compiler::COMPILER_OPTIONS.get_dir_build()) / compiler::COMPILER_OPTIONS.get_project_name();
  dest_path.replace_extension(".o");
  std::filesystem::path out_bin =
      std::filesystem::path(compiler::COMPILER_OPTIONS.get_dir_build()) / compiler::COMPILER_OPTIONS.get_project_name();
  out_bin.replace_extension(extension);

  std::string command = "clang \"" + dest_path.string() + "\" -o \"" + out_bin.string() + "\"";

  std::cout << "[linker] Clang linking command:\n  " << command << std::endl;
  if (auto err_code = system(command.c_str()); err_code != 0) {
    std::cerr << "[linker:ERROR] Linker failed: system code error " << err_code << std::endl;
    return false;
  }

  std::cout << "[linker] Executable created at " << out_bin << std::endl;
  return true;
}