#include "compiler/compiler.hpp"

#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>

#include <compiler_options.hpp>
#include <common.hpp>
#include <filesystem>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/script.hpp"
#include "nexus/type.hpp"
#include "nexus/symbol.hpp"
#include "nexus/inference.hpp"
#include "nexus/unresolved.hpp"
#include "nexus/resolved.hpp"

#include "nexus/pipeline.hpp"

#include "misc/error_output.hpp"


module::Graph     compiler::modules    = module::Graph();
ast::Arena        compiler::nodes      = ast::Arena();
type::Arena       compiler::types      = type::Arena();
symbol::Arena     compiler::symbols    = symbol::Arena();
scope::Graph      compiler::scopes     = scope::Graph();
unresolved::Arena compiler::unresolved = unresolved::Arena();
resolved::Arena   compiler::resolved   = resolved::Arena();
inference::Arena  compiler::inference  = inference::Arena();


pipeline::Pipeline compiler::pipeline = pipeline::Pipeline();

compiler::Compiler       compiler::COMPILER(compiler::modules, compiler::nodes, compiler::types, compiler::symbols,
                                            compiler::pipeline, compiler::scopes, compiler::unresolved, compiler::resolved,
                                            compiler::inference);
common::Compiler_Options compiler::COMPILER_OPTIONS;
llvm::LLVMContext        compiler::LLVM_CTX;

const std::string common::SOFTWARE_NAME = "velox-compiler";


constexpr std::string_view pipeline_info =
    R"(
===============================================================================
 [Toolchain]->configuration->[Preparer]
===============================================================================
 [Preparer]->lexer->preprocessor->parser->wait all->[Shipowner] 
===============================================================================
 [Shipowner]->binding generation->preparer->wait all->[Analyzer] 
===============================================================================
 [Analyzer]->symbol->inference->semantic->[Generator] 
===============================================================================
 [Generator]->codegen->optimisation->emitter->module linker->[Linker]
===============================================================================
)";

constexpr std::string_view binder_info = "%0 files + %1 binding files = %2 total files in the main pipeline";


void compiler::Compiler::add_error(Error_Diagnostic&& error)
{
  auto& vec = errors[error.elem_first.scr_id];
  vec.push_back(std::move(error));
}


bool compiler::Compiler::start_compilation()
{
  static const bool mute = compiler::COMPILER_OPTIONS.mute;

  auto start = std::chrono::high_resolution_clock::now();

  if (!compiler::COMPILER_OPTIONS.current_config_file.empty() && !mute) {
    std::cout << "\n[build:warning] Raw compilation command detected, "
                 "please use 'velox-toolchain' to develop proprely with the Velox programming language.\n"
              << std::endl;
  }

  if (!mute) {
    std::cout << "[velox-compiler] Compilation Started" << pipeline_info << std::endl;
    std::cout << "  Config file used: \"" << compiler::COMPILER_OPTIONS.current_config_file << "\"" << std::endl;
  }


  // filesystem
  std::filesystem::path target_dir = compiler::COMPILER_OPTIONS.get_dir_source();
  auto                  scr_infos  = pipeline.query_scripts_at_dir(target_dir.string());

  if (scr_infos.empty()) {
    if (!mute) {
      std::cout << "[build] No files found at the source folder path:\n  " << target_dir << "\n";
      std::cout << "  Check if the source folder path is correct.\n";
      std::cout << "  Or start your project by creating your first script in the source folder path.";
      std::cout << std::endl;
    }
    return false;
  }


  auto end = std::chrono::high_resolution_clock::now();
  actual_duration += std::chrono::duration<double, std::milli>(end - start).count();


  while (!pipeline.unprepared_scripts.empty()) {
    for (auto scr_id : pipeline.unprepared_scripts) pipeline.engage_preparer(script::_id(scr_id));

    for (auto scr_id : pipeline.prepared_scripts) pipeline.unprepared_scripts.erase(scr_id);
    for (auto& [scr_id, err] : errors) pipeline.unprepared_scripts.erase(scr_id);
  }

  pipeline.engage_shipowner();

  if (!errors.empty()) {
    for (auto& [scr_id, errs] : errors) {
      for (auto err = errs.rbegin(); err != errs.rend(); ++err) {
        std::cerr << err->print_error() << std::endl;
      }
    }
    return false;
  }

  for (auto scr_id : pipeline.prepared_scripts) pipeline.engage_analyzer(scr_id);

  for (auto scr_id : pipeline.analyzed_scripts) pipeline.engage_generator(scr_id);

  // if all scripts are analyzed without any errors
  if (pipeline.analyzed_scripts.size() == pipeline.compilation_scripts.size()) {
    pipeline.engage_module_linker(pipeline.get_script(script::_id(0)).id);
  }

  pipeline.engage_linker();

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
