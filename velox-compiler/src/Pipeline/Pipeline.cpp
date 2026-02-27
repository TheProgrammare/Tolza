#include "Pipeline.hpp"

#include <iostream>
#include <ostream>
#include <string>

#include "Compilation.hpp"
#include "Globals.hpp"

#include "ScriptInfo.hpp"

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
  if (Config::dot_print) {
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


// simple parseur CLI minimal
void parse_args_for_compilation_context(CompCtx& ctx, int argc, const char* argv[])
{
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.rfind("--src=", 0) == 0) {
      auto str = arg.substr(6);

      COMPILATION_ARGS.emplace("src", str);
      ctx.source_dir = str;

    } else if (arg.rfind("--dest=", 0) == 0) {
      auto str = arg.substr(7);

      COMPILATION_ARGS.emplace("dest", str);
      ctx.codegen_dest_file = str;

    } else if (arg.rfind("--abi=", 0) == 0) {
      auto str = arg.substr(6);

      COMPILATION_ARGS.emplace("abi", str);
      ctx.target_abi = str;

    } else if (arg.rfind("--arch=", 0) == 0) {
      auto str = arg.substr(7);

      COMPILATION_ARGS.emplace("arch", str);
      ctx.target_arch = str;

    } else if (arg.rfind("--bits=", 0) == 0) {
      auto str = arg.substr(7);

      COMPILATION_ARGS.emplace("bits", str);
      ctx.target_bits = std::stoul(str);

    } else if (arg.rfind("--os=", 0) == 0) {
      auto str = arg.substr(5);

      COMPILATION_ARGS.emplace("os", str);
      ctx.target_os = str;

    } else if (arg.rfind("--libc=", 0) == 0) {
      auto str = arg.substr(7);

      COMPILATION_ARGS.emplace("libc", str);
      ctx.target_libc = str;

    } else if (arg == "--debug") {
      COMPILATION_ARGS.emplace("debug", "1");
      ctx.profile_debug  = true;
      Config::debug_mode = true;

    } else if (arg == "--release") {
      COMPILATION_ARGS.emplace("debug", "0");
      ctx.profile_debug = false;

    } else if (arg.rfind("--opt-level=", 0) == 0) {
      auto str = arg.substr(12);

      COMPILATION_ARGS.emplace("opt-level", str);
      ctx.profile_opt_level = std::stoi(str);

    } else if (arg == "--size-opt") {
      COMPILATION_ARGS.emplace("size-opt", "true");
      ctx.profile_size_opt = true;

    } else if (arg == "--debug-dot") {
      COMPILATION_ARGS.emplace("debug-dot", "true");
      Config::dot_print = true;

    } else if (arg == "--debug-dot-exposer") {
      COMPILATION_ARGS.emplace("debug-exposer", "true");
      Config::dot_exposer_print = true;

    } else if (arg == "--debug-pp") {
      COMPILATION_ARGS.emplace("debug-pp", "true");
      Config::debug_postprocessor_output = true;

    } else if (arg.rfind("-D", 0) == 0) {
      auto def    = arg.substr(2);
      auto eq_pos = def.find('=');

      std::string name, value;

      if (eq_pos != std::string::npos) {
        name  = def.substr(0, eq_pos);
        value = def.substr(eq_pos + 1);
      } else {
        name  = def;
        value = "1"; // implicit value
      }
      COMPILATION_ARGS.emplace(name, value);
      ctx.defines.emplace(name, value);

    } else if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);

      COMPILATION_ARGS.emplace(name, ""); // no value for -UName
      ctx.undefines.push_back(name);

    } else if (arg.rfind("--output=", 0) == 0) {
      auto str = arg.substr(9);

      COMPILATION_ARGS.emplace("output", str);
      ctx.codegen_output_dir = str;

    } else if (arg == "--emit-obj" || arg == "--emit=obj") {
      COMPILATION_ARGS.emplace("emit", "obj");
      ctx.codegen_emit_mode = CompCtx::EEmitMode::OBJ;

    } else if (arg == "--emit-asm" || arg == "--emit=asm") {
      COMPILATION_ARGS.emplace("emit", "asm");
      ctx.codegen_emit_mode = CompCtx::EEmitMode::ASM;

    } else if (arg == "--emit-bc" || arg == "--emit=bc") {
      COMPILATION_ARGS.emplace("emit", "bc");
      ctx.codegen_emit_mode = CompCtx::EEmitMode::BC;

    } else if (arg == "--emit-bin" || arg == "--emit=bin") {
      COMPILATION_ARGS.emplace("emit", "bin");
      ctx.codegen_emit_mode = CompCtx::EEmitMode::BIN;

    } else {
      std::cerr << "Warning: unknown argument '" << arg << "'\n";
    }
  }
}
