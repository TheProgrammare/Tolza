#include "Pipeline.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

#include <llvm/TargetParser/Host.h>

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
    "[5  ] [EMBinder    ] external module binder to generate all external functions, types, "
    "globals used\n"
    "[5.1] [sub-comp 1-5] the sub-compilation of binders generated then insert them in the main "
    "pipeline\n"
    "[6  ] [Exporter    ] to export all code type and symbol imported in other scripts\n"
    "[7  ] [Resolver    ] to resolve and audit the code before the LLVM IR generation\n"
    "[7.1] [- symbol    ] to resolve symbols\n"
    "[7.2] [- type      ] to resolve types\n"
    "[7.3] [- semantic  ] to resolve semantics\n"
    "[mid-end]\n"
    "[8  ] [LLVM IR     ] generate to LLVM Intermediate Representation\n"
    "[back-end]\n"
    "[9  ] [Linker      ] link all .ll then .o scripts into one executable\n" color_RESET;

bool start_compilation(const std::string& target_file)
{
  if (in_binding_compilation) std::cout << "[EMBinder] ";
  std::cout << color_BLUE "[build] Compilation Started\n" color_RESET << std::endl;
  if (!in_binding_compilation) {
    std::cout << pipeline_info << std::endl;
  }

  if (in_binding_compilation) std::cout << "[EMBinder] ";
  std::cout << color_BLUE "[build] [1/9] File system begins" color_RESET << std::endl;

  auto scr_infos = pipeline_start_filesystem(target_file);

  if (scr_infos.empty()) {
    if (in_binding_compilation) {
      std::cout << color_BLUE "[EMBinder] [build] [info] no files found at the source folder path: " color_RESET
                << target_file << "\n";
      std::cout << color_BLUE "[EMBinder] [build] [info] no sub-compilation need without any file binding" << "\n";
      std::cout << color_BLUE "[EMBinder] [build] Compilation finish successfully !\n" color_RESET;
      return true;
    }
    std::cout << color_BLUE << "[build] [info] no files found at the source folder path: " << target_file << "\n";
    std::cout << color_BLUE << "[build] [hint] Check if the source folder path is correct." << target_file << "\n";
    std::cout << color_BLUE
              << "               Or start your project by creating your first script in the source "
                 "folder path."
              << target_file << std::endl;
    return false;
  }

  if (in_binding_compilation) std::cout << "[EMBinder] ";
  std::cout << color_BLUE "[build] [2/9] Lexer begins" color_RESET << std::endl;
  if (!pipeline_start_lexer(scr_infos)) return false;

  if (in_binding_compilation) std::cout << color_BLUE "[EMBinder] ";
  std::cout << color_BLUE "[build] [3/9] Preprocessor begins" color_RESET << std::endl;
  if (!pipeline_start_preprocessor(scr_infos)) return false;

  if (in_binding_compilation) std::cout << color_BLUE "[EMBinder] ";
  std::cout << color_BLUE "[build] [4/9] Parser begins" color_RESET << std::endl;
  // parsing tokens
  if (!pipeline_start_parser(scr_infos)) return false;

  if (DOT_PRINT) {
    if (in_binding_compilation) std::cout << color_BLUE "[EMBinder] ";
    std::cout << color_BLUE "[build] [debug] AST viewer begins" color_RESET << std::endl;
    generate_AST_View(scr_infos);
  }

  // get binded scripts from bindings compilation
  // to the main compilation workflow
  static std::vector<std::shared_ptr<ScriptInfo>> bind_files_info;

  // generate bindings
  if (!in_binding_compilation) {
    std::cout << color_BLUE "[build] [5/9] External Module Binder (EMBinder) begins" color_RESET << std::endl;
    if (!pipeline_start_EMBinder(scr_infos)) return false;
  } else {
    bind_files_info = scr_infos;
    // in binding generation
    // return to the normal compilation with the bindings added
    return true; // binding generation successful
  }

  // merge binds with user files
  scr_infos.reserve(scr_infos.size() + bind_files_info.size());
  for (auto inf : bind_files_info) {
    scr_infos.push_back(inf);
  }

  if (!bind_files_info.empty()) {
    std::cout << color_YELLOW "[build] [EMBinder] [summary] " << bind_files_info.size()
              << color_RESET " binding files added to the main pipeline !" << std::endl;
    std::cout << color_YELLOW "[build] [info] return to the main pipeline flow\n" << std::endl;
  }

  std::cout << color_BLUE "[build] [6/9] Exportation begins\n" color_RESET;
  // import and export modules (to have all symbols for the resolution)
  if (!pipeline_start_exporter(scr_infos)) return false;

  std::cout << color_BLUE "[build] [7/9] Resolver begins\n" color_RESET;
  // resolve symbols
  if (!pipeline_start_resolvers(scr_infos)) return false;

  std::cout << color_BLUE "[build] [8/9] LLVM IR begins\n" color_RESET;
  // generate LLVM IR code
  if (!pipeline_start_LLVM_IR(scr_infos)) return false;

  std::cout << color_BLUE "[build] [9/9] Linker begins\n" color_RESET;
  // link data
  if (!pipeline_start_linker(scr_infos)) return false;

  std::cout << color_BLUE "[build] Compilation finish successfully !\n" color_RESET;

  // easter eggs
  if (bFR_KEY) std::cout << "[note] " << FR_KEY_STR << std::endl;
  if (bCATH) std::cout << "[note] " << CATH_STR << std::endl;
  if (bCATH_10) std::cout << "[note] " << CATH_10_STR << std::endl;
  if (bFR_GAULLE) std::cout << "[note] " << FR_GAULLE_STR << std::endl;
  if (bFR_NAP) std::cout << "[note] " << FR_NAP_STR << std::endl;
  if (bSUM_GIL_ENK) std::cout << "[note] " << SUM_GIL_ENK_STR << std::endl;
  if (bCH_CONF) std::cout << "[note] " << CH_CONF_STR << std::endl;

  return true;
}

void applyDefaultAndDetectNative()
{
  if (COMP_CTX.target_arch == "native") {
    COMP_CTX.target_arch = llvm::sys::getDefaultTargetTriple();

    if (COMP_CTX.target_arch.find("64") != std::string::npos)
      COMP_CTX.target_arch_bits = 64;
    else if (COMP_CTX.target_arch.find("32") != std::string::npos)
      COMP_CTX.target_arch_bits = 32;
    else if (COMP_CTX.target_arch.find("16") != std::string::npos)
      COMP_CTX.target_arch_bits = 16;
    else
      COMP_CTX.target_arch_bits = 8; // fallback
  }

  if (COMP_CTX.target_os.empty()) {
    // extraire le nom d'OS simple depuis le triple
    auto pos1 = COMP_CTX.target_arch.find('-');
    auto pos2 = COMP_CTX.target_arch.find('-', pos1 + 1);
    COMP_CTX.target_os =
        (pos2 != std::string::npos) ? COMP_CTX.target_arch.substr(pos1 + 1, pos2 - pos1 - 1) : "unknown";
  }

  // synchroniser debug LLVM
  if (COMP_CTX.is_debug) COMP_CTX.LLVM_info_debug = true;
}

// simple parseur CLI minimal
void parseArgs(int argc, char** argv)
{
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.rfind("--src=", 0) == 0) {
      COMPILATION_ARGS.insert({"src", arg.substr(6)});
      COMP_CTX.src_file = arg.substr(6);
    } else if (arg.rfind("--dest=", 0) == 0) {
      COMPILATION_ARGS.insert({"dest", arg.substr(7)});
      COMP_CTX.dest_file = arg.substr(7);
    } else if (arg.rfind("--abi=", 0) == 0) {
      COMPILATION_ARGS.insert({"abi", arg.substr(6)});
      COMP_CTX.target_abi = arg.substr(6);
    } else if (arg.rfind("--arch=", 0) == 0) {
      COMPILATION_ARGS.insert({"arch", arg.substr(7)});
      COMP_CTX.target_arch = arg.substr(7);
    } else if (arg.rfind("--bits=", 0) == 0) {
      COMPILATION_ARGS.insert({"bits", arg.substr(7)});
      COMP_CTX.target_arch_bits = std::stoul(arg.substr(7));
    } else if (arg.rfind("--os=", 0) == 0) {
      COMPILATION_ARGS.insert({"os", arg.substr(5)});
      COMP_CTX.target_os = arg.substr(5);
    } else if (arg.rfind("--libc=", 0) == 0) {
      COMPILATION_ARGS.insert({"libc", arg.substr(7)});
      COMP_CTX.libc = arg.substr(7);
    } else if (arg == "--debug") {
      COMPILATION_ARGS.insert({"debug", "1"});
      COMP_CTX.is_debug = true;
      DEBUG_MODE        = true;
    } else if (arg == "--release") {
      COMPILATION_ARGS.insert({"debug", "0"});
      COMP_CTX.is_debug = false;
    } else if (arg.rfind("--opt-level=", 0) == 0) {
      COMPILATION_ARGS.insert({"opt-level", arg.substr(12)});
      COMP_CTX.opt_level = static_cast<uint8_t>(std::stoul(arg.substr(12)));
    } else if (arg == "--size-opt") {
      COMPILATION_ARGS.insert({"size-opt", "true"});
      COMP_CTX.is_size_opt = true;
    } else if (arg == "--debug-dot") {
      COMPILATION_ARGS.insert({"debug-dot", "true"});
      DOT_PRINT = true;
    } else if (arg == "--debug-dot-exposer") {
      COMPILATION_ARGS.insert({"debug-exposer", "true"});
      DOT_EXPOSER_PRINT = true;
    } else if (arg == "--debug-pp") {
      COMPILATION_ARGS.insert({"debug-pp", "true"});
      DEBUG_POSTPROCESSOR_OUTPUT = true;
    } else if (arg.rfind("-D", 0) == 0) {
      auto        def    = arg.substr(2);
      auto        eq_pos = def.find('=');
      std::string name, value;
      if (eq_pos != std::string::npos) {
        name  = def.substr(0, eq_pos);
        value = def.substr(eq_pos + 1);
      } else {
        name  = def;
        value = "1"; // implicit value
      }
      COMPILATION_ARGS.emplace(name, value);
      COMP_CTX.defines[name] = value;
    } else if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);
      COMPILATION_ARGS.emplace(name, ""); // no value for -UName
      COMP_CTX.undefines.push_back(name);
    } else if (arg.rfind("--output=", 0) == 0) {
      COMPILATION_ARGS.insert({"output", arg.substr(9)});
      COMP_CTX.output = std::filesystem::path(arg.substr(9));
    } else if (arg == "--emit-obj" || arg == "--emit=obj") {
      COMPILATION_ARGS.insert({"emit", "obj"});
      COMP_CTX.emit_mode = CompCtx::EEmitMode::OBJ;
    } else if (arg == "--emit-asm" || arg == "--emit=asm") {
      COMPILATION_ARGS.insert({"emit", "asm"});
      COMP_CTX.emit_mode = CompCtx::EEmitMode::ASM;
    } else if (arg == "--emit-bc" || arg == "--emit=bc") {
      COMPILATION_ARGS.insert({"emit", "bc"});
      COMP_CTX.emit_mode = CompCtx::EEmitMode::BC;
    } else if (arg == "--emit-bin" || arg == "--emit=bin") {
      COMPILATION_ARGS.insert({"emit", "bin"});
      COMP_CTX.emit_mode = CompCtx::EEmitMode::BIN;
    } else {
      std::cerr << "Warning: unknown argument '" << arg << "'\n";
    }
  }

  applyDefaultAndDetectNative();
}
