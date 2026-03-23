#include "pipeline.hpp"

#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>

#include <compiler_context.hpp>

#include "pipeline.hpp"

#include "compiler/compiler.hpp"

#include "misc/script_info.hpp"

#include "pipeline_binder.hpp"
#include "pipeline_exporter.hpp"
#include "pipeline_filesystem.hpp"
#include "pipeline_llvm-ir.hpp"
#include "pipeline_lexer.hpp"
#include "pipeline_linker.hpp"
#include "pipeline_parser.hpp"
#include "pipeline_preprocessor.hpp"
#include "pipeline_resolvers.hpp"
#include "pipeline_llvm_opti.hpp"
#include "pipeline_emit.hpp"

#include "visitor/visitor_print.hpp"


static const std::string pipeline_info =
    R"(
===============================================================================
 [Toolchain]->configuration->filesystem->[Compiler]
===============================================================================
 [Compiler]->lexer->preprocessor->parser->binder->exporter->resolver->[LLVM] 
===============================================================================
 [LLVM]->codegen->optimisation->emitter->linker->[Executable]
===============================================================================
)";

static const std::string binder_info = "%0 files + %1 binding files = %2 total files in the main pipeline";

bool start_compilation(int argc, const char* argv[])
{
  auto duration_start = std::chrono::high_resolution_clock::now();

  // if command == src/ build ...
  if (!compiler::in_binding_compilation && argc > 2) compiler::COMP_CTX.apply_args(argc, argv);

  if (compiler::in_binding_compilation)
    std::cout << "[binder] Binders Generated Compilation Started\n" << std::endl;
  else {
    if (!compiler::COMP_CTX.current_config_file.empty()) {
      std::cout << "\n[build:warning] Raw compilation command detected, "
                   "please use 'velox-toolchain' to develop proprely with the Velox programming language.\n"
                << std::endl;
    }

    std::cout << "[velox-compiler] Compilation Started" << pipeline_info << std::endl;
    std::cout << "  Config file used: " << compiler::COMP_CTX.current_config_file << std::endl;
  }


  // filesystem
  if (compiler::in_binding_compilation)
    std::cout << "[binder:1/4] File system begins" << std::endl;
  else
    std::cout << "[build:1/11] File system begins" << std::endl;

  std::filesystem::path target_dir =
      compiler::in_binding_compilation ? compiler::COMP_CTX.binding_dir : compiler::COMP_CTX.source_dir;
  auto scr_infos = pipeline_start_filesystem(target_dir.string());

  if (scr_infos.empty()) {
    if (compiler::in_binding_compilation) {
      std::cout << "[binder] No files found at the source folder path: " << target_dir << "\n";
      std::cout << "[binder] No sub-compilation need without any file binding" << "\n";
      std::cout << "[binder] Binders Generated Compilation finish successfully !\n" << std::endl;
      return true;
    }
    std::cout << "[build] No files found at the source folder path: " << target_dir << "\n";
    std::cout << "[build] Check if the source folder path is correct." << target_dir << "\n";
    std::cout << "Or start your project by creating your first script in the source "
                 "folder path."
              << target_dir << std::endl;
    return false;
  }


  // lexer
  if (compiler::in_binding_compilation)
    std::cout << "[binder:2/4] Lexer begins" << std::endl;
  else
    std::cout << "[build:2/11] Lexer begins" << std::endl;

  if (!pipeline_start_lexer(scr_infos)) return false;


  // preprocessor
  if (compiler::in_binding_compilation)
    std::cout << "[binder:3/4] Preprocessor begins" << std::endl;
  else
    std::cout << "[build:3/11] Preprocessor begins" << std::endl;

  if (!pipeline_start_preprocessor(scr_infos)) return false;


  // parser
  if (compiler::in_binding_compilation)
    std::cout << "[binder:4/4] Parser begins" << std::endl;
  else
    std::cout << "[build:4/11] Parser begins" << std::endl;

  if (!pipeline_start_parser(scr_infos)) return false;


  // printer
  if (compiler::COMP_CTX.print_ast) {
    if (compiler::in_binding_compilation)
      std::cout << "[binder:debug] Printing AST view at " << compiler::COMP_CTX.get_debug_graph_dir() << "\n"
                << std::endl;
    else
      std::cout << "[debug] Printing AST view at " << compiler::COMP_CTX.get_debug_graph_dir() << "\n" << std::endl;

    for (auto& info : scr_infos) Visitor_Print(*info).visit(*info->rootNode);
  }

  // get binded scripts from bindings compilation to the main compilation workflow
  static std::vector<std::shared_ptr<ScriptInfo>> bind_files_info;

  // generate bindings
  if (!compiler::in_binding_compilation) {
    std::cout << "[build:5/11] External Module Binder begins" << std::endl;
    if (!pipeline_start_binder(scr_infos)) return false;
  } else {
    bind_files_info = scr_infos;
    // in binding generation return to the normal compilation with the bindings added
    return true; // binding generation successful
  }

  // merge binds with user files
  scr_infos.reserve(scr_infos.size() + bind_files_info.size());
  for (auto inf : bind_files_info) {
    scr_infos.push_back(inf);
  }

  if (!bind_files_info.empty()) {
    auto   bind_txt = binder_info;
    size_t src_size = scr_infos.size() - bind_files_info.size();
    compiler::fmt_template(bind_txt, {color_YELLOW + std::to_string(src_size) + color_RESET,
                                      color_YELLOW + std::to_string(bind_files_info.size()) + color_RESET,
                                      color_YELLOW + std::to_string(scr_infos.size()) + color_RESET});
    std::cout << color_YELLOW "[build:binder:summary] " color_RESET << bind_txt << std::endl;
    std::cout << color_YELLOW "[build] Return to the main pipeline flow" color_RESET << std::endl;
  }

  // exporter
  std::cout << "[build:6/11] Exportation begins" << std::endl;
  // import and export modules (to have all symbols for the resolution)
  if (!pipeline_start_exporter(scr_infos)) return false;

  // resolvers
  std::cout << "[build:7/11] Resolver begins" << std::endl;
  // resolve symbols - resolve types - semantic analyzer
  if (!pipeline_start_resolvers(scr_infos)) return false;

  // LLVM IR
  std::cout << "[build:8/11] LLVM IR begins" << std::endl;
  // generate LLVM IR code
  if (!pipeline_start_LLVM_IR(scr_infos)) return false;

  // optimisation
  std::cout << "[build:9/11] LLVM optimisation" << std::endl;
  if (!pipeline_start_llvm_opti(scr_infos)) return false;

  // emit
  std::cout << "[build:10/11] Emitting" << std::endl;
  if (!pipeline_start_emit(scr_infos)) return false;

  // linker
  std::cout << "[build:11/11] Linker begins" << std::endl;
  // link data
  if (!pipeline_start_linker(scr_infos)) return false;


  auto   duration_end = std::chrono::high_resolution_clock::now();
  double milli        = std::chrono::duration<double, std::milli>(duration_end - duration_start).count();

  constexpr char end_log[] =
      "[velox-compiler] Compilation finish successfully !\n"
      "Duration: " color_YELLOW "%0 ms\n" color_RESET "  Find the executable at " color_MAGENTA "%1" color_RESET;

  std::string fmt_end = end_log;
  compiler::fmt_template(fmt_end, {std::to_string(milli), compiler::COMP_CTX.codegen_build_dir});

  std::cout << fmt_end << std::endl;

  return true;
}
