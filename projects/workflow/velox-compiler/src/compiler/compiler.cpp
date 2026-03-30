#include "compiler/compiler.hpp"

#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>

#include <compiler_context.hpp>
#include <common.hpp>
#include <filesystem>

#include <compiler_context.hpp>

#include "misc/script_info.hpp"

#include "pipeline/pipeline_exporter.hpp"
#include "pipeline/pipeline_binder.hpp"
#include "pipeline/pipeline_filesystem.hpp"
#include "pipeline/pipeline_codegen.hpp"
#include "pipeline/pipeline_lexer.hpp"
#include "pipeline/pipeline_linker.hpp"
#include "pipeline/pipeline_parser.hpp"
#include "pipeline/pipeline_preprocessor.hpp"
#include "pipeline/pipeline_resolvers.hpp"
#include "pipeline/pipeline_llvm_opti.hpp"
#include "pipeline/pipeline_emit.hpp"

#include "visitor/visitor_print.hpp"


Compiler          compiler::COMP;
common::CompCtx   compiler::COMP_CTX;
llvm::LLVMContext compiler::LLVM_CTX;

const std::string common::SOFTWARE_NAME = "velox-compiler";

namespace fs = std::filesystem;


static const std::string pipeline_info =
    R"(
===============================================================================
 [Toolchain]->configuration->[Preparator]
===============================================================================
 [Preparator]->filesystem->lexer->preprocessor->parser->[Analyser] 
===============================================================================
 [Analyzer]->exporter->check_symbols->check_types->check_sematics->[Generator] 
===============================================================================
 [Generator]->codegen->optimisation->emitter->[Linker]-> executable
===============================================================================
)";

static const std::string binder_info = "%0 files + %1 binding files = %2 total files in the main pipeline";

bool Compiler::start_compilation()
{
  auto start = std::chrono::high_resolution_clock::now();

  if (!compiler::COMP_CTX.current_config_file.empty()) {
    std::cout << "\n[build:warning] Raw compilation command detected, "
                 "please use 'velox-toolchain' to develop proprely with the Velox programming language.\n"
              << std::endl;
  }

  std::cout << "[velox-compiler] Compilation Started" << pipeline_info << std::endl;
  std::cout << "  Config file used: " << compiler::COMP_CTX.current_config_file << std::endl;


  // filesystem
  std::cout << "[build] File system begins" << std::endl;

  std::filesystem::path target_dir = compiler::COMP_CTX.get_dir_source();
  auto                  scr_infos  = pipeline_start_filesystem(target_dir.string());

  if (scr_infos.empty()) {
    std::cout << "[build] No files found at the source folder path: " << target_dir << "\n";
    std::cout << "[build] Check if the source folder path is correct." << target_dir << "\n";
    std::cout << "Or start your project by creating your first script in the source "
                 "folder path."
              << target_dir << std::endl;
    return false;
  }


  auto end = std::chrono::high_resolution_clock::now();
  actual_duration += std::chrono::duration<double, std::milli>(end - start).count();

  if (!prepare_scripts(scr_infos)) return false;

  std::vector<std::shared_ptr<ScriptInfo>> _prepared_scripts;
  _prepared_scripts.reserve(prepared_scripts.size());

  for (auto& [_, script] : prepared_scripts) _prepared_scripts.push_back(script);

  return analyze_scripts(_prepared_scripts);
}

bool Compiler::prepare_scripts(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  auto start = std::chrono::high_resolution_clock::now();

  // lexer
  if (!pipeline_start_lexer(scr_infos)) return false;


  // preprocessor
  if (!pipeline_start_preprocessor(scr_infos)) return false;


  // parser
  if (!pipeline_start_parser(scr_infos)) return false;


  for (auto& scr_info : scr_infos) prepared_scripts[scr_info->file_path] = scr_info;


  // printer
  static bool log_ast = compiler::COMP_CTX.debugs.contains("ast");
  if (log_ast) {
    for (auto& info : scr_infos) Visitor_Print(*info).visit(*info->rootNode);
  }

  // generate bindings
  if (!pipeline_start_binder(scr_infos)) return false;

  imported_modules.clear();

  for (auto& scr_info : scr_infos) {
    prepared_scripts[scr_info->file_path] = scr_info;

    for (auto& imp : scr_info->imported_mod) {
      auto path = imp->get_path();
      if (!prepared_scripts.contains(path)) {
        imported_modules.insert(path);
      }
    }
  }

  if (!imported_modules.empty()) {
    auto scrs = pipeline_start_filesystem_on_files(imported_modules);
    return prepare_scripts(scrs);
  }


  auto end = std::chrono::high_resolution_clock::now();
  actual_duration += std::chrono::duration<double, std::milli>(end - start).count();
  return true;
}

bool Compiler::analyze_scripts(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  auto start = std::chrono::high_resolution_clock::now();

  // exporter
  // import and export modules (to have all symbols for the resolution)
  if (!pipeline_start_exporter(scr_infos)) return false;

  // resolvers
  // resolve symbols - resolve types - semantic analyzer
  if (!pipeline_start_resolvers(scr_infos)) return false;

  // LLVM IR
  // generate LLVM IR code
  if (!pipeline_start_codegen(scr_infos)) return false;

  // optimisation
  if (!pipeline_start_llvm_opti(scr_infos)) return false;

  // emit
  if (!pipeline_start_emit(scr_infos)) return false;

  // linker
  // link data
  if (!pipeline_start_linker(scr_infos)) return false;


  auto end = std::chrono::high_resolution_clock::now();
  actual_duration += std::chrono::duration<double, std::milli>(end - start).count();

  constexpr char end_log[] =
      "[velox-compiler] Compilation finish successfully !\n"
      "Duration: " color_YELLOW "%0 ms\n" color_RESET "  Find the executable at " color_MAGENTA "%1" color_RESET;

  std::string fmt_end = end_log;
  common::fmt_template(fmt_end, {std::to_string(actual_duration), compiler::COMP_CTX.dir_build});

  std::cout << fmt_end << std::endl;

  return true;
}

std::string compiler::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "FSYS";
  case compiler::EPhase::lexer:             return "LEXE";
  case compiler::EPhase::preprosessor:      return "PREP";
  case compiler::EPhase::parser:            return "PARS";
  case compiler::EPhase::binder:            return "EMBI";
  case compiler::EPhase::resolver_symbol:   return "SYMB";
  case compiler::EPhase::resolver_type:     return "TYPE";
  case compiler::EPhase::resolver_semantic: return "SEMA";
  case compiler::EPhase::llvmir:            return "LLVM";
  case compiler::EPhase::linker:            return "LINK";
  }
}

std::string compiler::Phase_to_str(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "file system";
  case compiler::EPhase::lexer:             return "lexer";
  case compiler::EPhase::preprosessor:      return "preprocessor";
  case compiler::EPhase::parser:            return "parser";
  case compiler::EPhase::binder:            return "external module binder";
  case compiler::EPhase::resolver_symbol:   return "resolver symbol";
  case compiler::EPhase::resolver_type:     return "resolver type";
  case compiler::EPhase::resolver_semantic: return "resolver semantic";
  case compiler::EPhase::llvmir:            return "LLVM IR";
  case compiler::EPhase::linker:            return "linker";
  }
}
