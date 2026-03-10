#include "pipeline.hpp"

#include <iostream>
#include <ostream>
#include <string>

#include "pipeline.hpp"

#include "compiler.hpp"

#include "script_info.hpp"

#include "pipeline_binder.hpp"
#include "pipeline_exporter.hpp"
#include "pipeline_filesystem.hpp"
#include "pipeline_llvm-ir.hpp"
#include "pipeline_lexer.hpp"
#include "pipeline_linker.hpp"
#include "pipeline_parser.hpp"
#include "pipeline_preprocessor.hpp"
#include "pipeline_resolvers.hpp"
#include "visitor/visitor_print.hpp"


static const std::string pipeline_info = color_BLUE
    R"(
===============================================================================
 [Toolchain]->context->filesystem->[Compiler]
===============================================================================
 [Compiler]->lexer->preprocessor->parser->binder->exporter->resolver->[LLVM] 
===============================================================================
 [LLVM]->codegen->linker->[OUTPUT]
===============================================================================
)"
    // too big ?
    /*
[velox-compiler] Pipeline: by stage
[front-end]
[1  ] [File system ] to find all scripts
[2  ] [Lexer       ] to tokenize the code
[3  ] [Preprocessor] to prepare the code according to metacode inscription
[4  ] [Parser      ] to generate the AST from the tokens preprocessed
[5  ] [binder    ] external module binder to generate all external functions, types, globals used
[5.1] [sub-comp 1-4] the sub-compilation of binders generated then insert them in the main pipeline
[6  ] [Exporter    ] to export all code type and symbol imported in other scripts
[7  ] [Resolver    ] to resolve and audit the code before the LLVM IR generation
[7.1] [- symbol    ] to resolve symbols
[7.2] [- type      ] to resolve types
[7.3] [- semantic  ] to resolve semantics
[mid-end]
[8  ] [LLVM IR     ] generate to LLVM Intermediate Representation
[back-end]
[9  ] [Linker      ] link all .ll then .o scripts into one executable
        */
    color_RESET;

static const std::string binder_info = "%0 files + %1 binding files = %2 total files in the main pipeline";

bool start_compilation(int argc, const char* argv[])
{
  // if command == src/ build ...
  if (!compiler::in_binding_compilation && argc > 2)
    compiler::parse_args_for_compilation_context(compiler::COMP_CTX, argc, argv);

  if (compiler::in_binding_compilation)
    std::cout << color_BLUE "[binder] Binders Generated Compilation Started\n" color_RESET << std::endl;
  else {
    if (!compiler::command_from_velox_toolchain) {
      std::cout << color_BLUE
          "\n[build:warning] Raw compilation command detected, "
          "please use 'velox-toolchain' to develop proprely with the Velox programming language.\n"
                << std::endl;
    }

    std::cout << color_BLUE "[velox-compiler] Compilation Started" color_RESET << pipeline_info << std::endl;
  }


  // filesystem
  if (compiler::in_binding_compilation)
    std::cout << color_BLUE "[binder:1/4] File system begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build:1/9] File system begins" color_RESET << std::endl;

  fs::path target_dir =
      compiler::in_binding_compilation ? compiler::COMP_CTX.get_binding_dir() : compiler::COMP_CTX.get_source_dir();
  auto scr_infos = pipeline_start_filesystem(target_dir);

  if (scr_infos.empty()) {
    if (compiler::in_binding_compilation) {
      std::cout << color_BLUE "[binder] No files found at the source folder path: " color_RESET << target_dir << "\n";
      std::cout << color_BLUE "[binder] No sub-compilation need without any file binding" << "\n";
      std::cout << color_BLUE "[binder] Binders Generated Compilation finish successfully !\n" color_RESET << std::endl;
      return true;
    }
    std::cout << color_BLUE << "[build] No files found at the source folder path: " << target_dir << "\n";
    std::cout << color_BLUE << "[build] Check if the source folder path is correct." << target_dir << "\n";
    std::cout << color_BLUE
              << "Or start your project by creating your first script in the source "
                 "folder path."
              << target_dir << std::endl;
    return false;
  }


  // lexer
  if (compiler::in_binding_compilation)
    std::cout << color_BLUE "[binder:2/4] Lexer begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build:2/9] Lexer begins" color_RESET << std::endl;

  if (!pipeline_start_lexer(scr_infos)) return false;


  // preprocessor
  if (compiler::in_binding_compilation)
    std::cout << color_BLUE "[binder:3/4] Preprocessor begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build:3/9] Preprocessor begins" color_RESET << std::endl;

  if (!pipeline_start_preprocessor(scr_infos)) return false;


  // parser
  if (compiler::in_binding_compilation)
    std::cout << color_BLUE "[binder:4/4] Parser begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build:4/9] Parser begins" color_RESET << std::endl;

  if (!pipeline_start_parser(scr_infos)) return false;


  // printer
  if (compiler::COMP_CTX.print_ast) {
    if (compiler::in_binding_compilation)
      std::cout << color_BLUE "[binder:debug] Printing AST view at " << compiler::COMP_CTX.get_debug_graph_dir()
                << color_RESET "\n"
                << std::endl;
    else
      std::cout << color_BLUE "[debug] Printing AST view at " << compiler::COMP_CTX.get_debug_graph_dir()
                << color_RESET "\n"
                << std::endl;

    for (auto& info : scr_infos) Visitor_Print(*info).visit(*info->rootNode);
  }

  // get binded scripts from bindings compilation to the main compilation workflow
  static std::vector<std::shared_ptr<ScriptInfo>> bind_files_info;

  // generate bindings
  if (!compiler::in_binding_compilation) {
    std::cout << color_BLUE "[build:5/9] External Module Binder begins" color_RESET << std::endl;
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
  std::cout << color_BLUE "[build:6/9] Exportation begins" color_RESET << std::endl;
  // import and export modules (to have all symbols for the resolution)
  if (!pipeline_start_exporter(scr_infos)) return false;

  // resolvers
  std::cout << color_BLUE "[build:7/9] Resolver begins" color_RESET << std::endl;
  // resolve symbols - resolve types - semantic analyzer
  if (!pipeline_start_resolvers(scr_infos)) return false;

  // LLVM IR
  std::cout << color_BLUE "[build:8/9] LLVM IR begins" color_RESET << std::endl;
  // generate LLVM IR code
  if (!pipeline_start_LLVM_IR(scr_infos)) return false;

  // linker
  std::cout << color_BLUE "[build:9/9] Linker begins" color_RESET << std::endl;
  // link data
  if (!pipeline_start_linker(scr_infos)) return false;

  std::cout << color_BLUE R"(
[velox-compiler] Compilation finish successfully !
)" color_RESET
            << std::endl;

  return true;
}
