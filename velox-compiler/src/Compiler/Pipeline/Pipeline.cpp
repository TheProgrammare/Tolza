#include "Pipeline.hpp"

#include <iostream>
#include <ostream>
#include <string>

#include "Toolchain/Compilation.hpp"
#include "Globals.hpp"

#include "Compiler/ScriptInfo.hpp"

#include "Pipeline_DotAST.hpp"
#include "Pipeline_EMBinder.hpp"
#include "Pipeline_Exporter.hpp"
#include "Pipeline_FileSystem.hpp"
#include "Pipeline_LLVMIR.hpp"
#include "Pipeline_Lexer.hpp"
#include "Pipeline_Linker.hpp"
#include "Pipeline_Parser.hpp"
#include "Pipeline_Preprocessor.hpp"
#include "Pipeline_Resolvers.hpp"


static const std::string pipeline_info = color_BLUE
    "[build] Pipeline: by stage\n"
    "[front-end]\n"
    "[1  ] [File system ] to find all scripts\n"
    "[2  ] [Lexer       ] to tokenize the code\n"
    "[3  ] [Preprocessor] to prepare the code according to metacode inscription\n"
    "[4  ] [Parser      ] to generate the AST from the tokens preprocessed\n"
    "[5  ] [EMBinder    ] external module binder to generate all external functions, types, globals used\n"
    "[5.1] [sub-comp 1-4] the sub-compilation of binders generated then insert them in the main pipeline\n"
    "[6  ] [Exporter    ] to export all code type and symbol imported in other scripts\n"
    "[7  ] [Resolver    ] to resolve and audit the code before the LLVM IR generation\n"
    "[7.1] [- symbol    ] to resolve symbols\n"
    "[7.2] [- type      ] to resolve types\n"
    "[7.3] [- semantic  ] to resolve semantics\n[mid-end]\n"
    "[8  ] [LLVM IR     ] generate to LLVM Intermediate Representation\n[back-end]\n"
    "[9  ] [Linker      ] link all .ll then .o scripts into one executable\n" color_RESET;

bool start_compilation(const fs::path& target_file)
{
  if (Config::in_binding_compilation)
    std::cout << color_BLUE "[EMBinder] Binders Compilation Started\n" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build] Compilation Started\n" color_RESET << pipeline_info << std::endl;


  // filesystem
  if (Config::in_binding_compilation)
    std::cout << color_BLUE "[EMBinder] [sub-build] [1/4] File system begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build] [1/9] File system begins" color_RESET << std::endl;

  auto scr_infos = pipeline_start_filesystem(target_file);

  if (scr_infos.empty()) {
    if (Config::in_binding_compilation) {
      std::cout << color_BLUE "[EMBinder] [sub-build] [info] no files found at the source folder path: " color_RESET
                << target_file << "\n";
      std::cout << color_BLUE "[EMBinder] [sub-build] [info] no sub-compilation need without any file binding" << "\n";
      std::cout << color_BLUE "[EMBinder] [sub-build] Binders Compilation finish successfully !\n" color_RESET;
      return true;
    }
    std::cout << color_BLUE << "[build] [info] no files found at the source folder path: " << target_file << "\n";
    std::cout << color_BLUE << "[build] [hint] Check if the source folder path is correct." << target_file << "\n";
    std::cout << color_BLUE
              << "Or start your project by creating your first script in the source "
                 "folder path."
              << target_file << std::endl;
    return false;
  }


  // lexer
  if (Config::in_binding_compilation)
    std::cout << color_BLUE "[EMBinder] [sub-build] [2/4] Lexer begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build] [2/9] Lexer begins" color_RESET << std::endl;

  if (!pipeline_start_lexer(scr_infos)) return false;


  // preprocessor
  if (Config::in_binding_compilation)
    std::cout << color_BLUE "[EMBinder] [sub-build] [3/4] Preprocessor begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build] [3/9] Preprocessor begins" color_RESET << std::endl;

  if (!pipeline_start_preprocessor(scr_infos)) return false;


  // parser
  if (Config::in_binding_compilation)
    std::cout << color_BLUE "[EMBinder] [sub-build] [4/4] Parser begins" color_RESET << std::endl;
  else
    std::cout << color_BLUE "[build] [4/9] Parser begins" color_RESET << std::endl;

  if (!pipeline_start_parser(scr_infos)) return false;


  // debug dot print
  if (COMP_CTX.dot_ast) {
    if (Config::in_binding_compilation)
      std::cout << color_BLUE "[EMBinder] [debug] AST viewer begins" color_RESET << std::endl;
    else
      std::cout << color_BLUE "[debug] AST viewer begins" color_RESET << std::endl;

    generate_AST_View(scr_infos);
  }

  // get binded scripts from bindings compilation to the main compilation workflow
  static std::vector<std::shared_ptr<ScriptInfo>> bind_files_info;

  // generate bindings
  if (!Config::in_binding_compilation) {
    std::cout << color_BLUE "[build] [5/9] External Module Binder (EMBinder) begins" color_RESET << std::endl;
    if (!pipeline_start_EMBinder(scr_infos)) return false;
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
    std::cout << color_BLUE "[build] [EMBinder] " color_YELLOW "[summary]\n"
              << "  -> " color_YELLOW << scr_infos.size() - bind_files_info.size() << color_RESET " files in projects\n"
              << "  -> " color_YELLOW << bind_files_info.size()
              << color_RESET " binding files added to the main pipeline\n"
              << "  -> " color_YELLOW << scr_infos.size() << color_RESET " total files in main pipeline\n\n";
    std::cout << color_YELLOW "[build] [info] return to the main pipeline flow\n" << std::endl;
  }

  // exporter
  std::cout << color_BLUE "[build] [6/9] Exportation begins\n" color_RESET;
  // import and export modules (to have all symbols for the resolution)
  if (!pipeline_start_exporter(scr_infos)) return false;

  // resolvers
  std::cout << color_BLUE "[build] [7/9] Resolver begins\n" color_RESET;
  // resolve symbols
  if (!pipeline_start_resolvers(scr_infos)) return false;

  // LLVM IR
  std::cout << color_BLUE "[build] [8/9] LLVM IR begins\n" color_RESET;
  // generate LLVM IR code
  if (!pipeline_start_LLVM_IR(scr_infos)) return false;

  // linker
  std::cout << color_BLUE "[build] [9/9] Linker begins\n" color_RESET;
  // link data
  if (!pipeline_start_linker(scr_infos)) return false;

  std::cout << color_BLUE "\n[build] Compilation finish successfully !\n" color_RESET;

  return true;
}
