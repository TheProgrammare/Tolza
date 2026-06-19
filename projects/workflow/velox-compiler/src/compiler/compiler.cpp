#include "compiler.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>

#include <common/compiler_options.hpp>
#include <common/common.hpp>

#include "binder/ffi_c_reader.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/type/type.hpp"
#include "nexus/symbol.hpp"
#include "nexus/inference.hpp"
#include "nexus/unresolved.hpp"
#include "nexus/resolved.hpp"
#include "nexus/pipeline.hpp"

#include "misc/error_output.hpp"


pipeline::Pipeline compiler::pipeline = pipeline::Pipeline();

unresolved::Arena compiler::unresolved = unresolved::Arena();
resolved::Arena   compiler::resolved   = resolved::Arena();
inference::Arena  compiler::inference  = inference::Arena();


compiler::Compiler        compiler::COMPILER = compiler::Compiler();
common::compiler::Options compiler::OPTIONS;
llvm::LLVMContext         compiler::LLVM_CTX;

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


void compiler::Compiler::add_error(const Error_Diagnostic& error)
{
  auto it = std::ranges::find_if(errors, [&](std::pair<cu::ID, std::vector<Error_Diagnostic>>& elem) -> bool {
    return elem.first == error.elem_first.cuid;
  });

  if (it != errors.end()) {
    auto& vec = it->second;
    vec.emplace_back(error);
  } else {
    errors.emplace_back(error.elem_first.cuid, std::vector({error}));
  }

  print_errors();
  std::abort();
}


compiler::Compiler::Compiler()
{
  module::initialization();
  type::initialization();
}


bool compiler::Compiler::start_compilation()
{
  static const bool mute = compiler::OPTIONS.mute;

  auto start = std::chrono::high_resolution_clock::now();

  if (!compiler::OPTIONS.dir.current_config_file.empty() && !mute) {
    std::cout << "\n[build:warning] Raw compilation command detected, "
                 "please use 'velox-toolchain' to develop proprely with the Velox programming language.\n"
              << "\n"; /*endl*/
  }

  if (!mute) {
    std::cout << "[velox-compiler] Compilation Started" << pipeline_info << "\n"; /*endl*/
    std::cout << "  Config file used: \"" << compiler::OPTIONS.dir.current_config_file << "\""
              << "\n"; /*endl*/
  }

  // filesystem
  std::filesystem::path target_dir = compiler::OPTIONS.dir.get_dir_source();
  auto                  CUs        = compiler::pipeline.query_CUs_at_dir(cu::ID::main(), target_dir.string());

  if (CUs.empty()) {
    if (!mute) {
      std::cout << "[build] No files found at the source folder path:\n  " << target_dir << "\n";
      std::cout << "  Check if the source folder path is correct.\n";
      std::cout << "  Or start your project by creating your first script in the source folder path.";
      std::cout << "\n"; /*endl*/
    }
    return false;
  }


  auto end = std::chrono::high_resolution_clock::now();
  actual_duration += std::chrono::duration<double, std::milli>(end - start).count();


  while (!compiler::pipeline.unprepared_compilation_units.empty()) {
    for (auto cuid : compiler::pipeline.unprepared_compilation_units) (void)compiler::pipeline.engage_preparer(cuid);

    for (auto cuid : compiler::pipeline.prepared_compilation_units)
      compiler::pipeline.unprepared_compilation_units.erase(cuid);
    for (const auto& [cuid, errs] : errors) {
      compiler::pipeline.unprepared_compilation_units.erase(cuid);
      for (const auto& err : errs) std::cout << err.print_error();
    }

    if (compiler::pipeline.unprepared_compilation_units.empty()) {
      (void)compiler::pipeline.engage_shipowner();

      if (!errors.empty()) {
        print_errors();
        return false;
      }

      (void)compiler::pipeline.engage_bindings();
    }
  }


  for (auto cuid : compiler::pipeline.prepared_compilation_units) (void)compiler::pipeline.engage_analyzer(cuid);

  for (auto cuid : compiler::pipeline.analyzed_compilation_units) (void)compiler::pipeline.engage_generator(cuid);

  // if all compilation_units are analyzed without any errors
  if (compiler::pipeline.analyzed_compilation_units.size() == compiler::pipeline.compilation_units.size()) {
    (void)compiler::pipeline.engage_module_linker(cu::ID::main());
  }

  (void)pipeline::Pipeline::engage_linker();

  return true;
}

void compiler::Compiler::print_errors() const
{
  for (const auto& [cuid, errs] : errors) {
    for (const auto& err : errs) std::cerr << err.print_error() << "\n"; /*endl*/
  }
}


std::string compiler::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case EPhase::filesystem:        return "FSYS";
  case EPhase::lexer:             return "LEXE";
  case EPhase::preprosessor:      return "PREP";
  case EPhase::parser:            return "PARS";
  case EPhase::binder:            return "EMBI";
  case EPhase::shipowner:         return "SHIP";
  case EPhase::resolver_symbol:   return "SYMB";
  case EPhase::resolver_type:     return "TYPE";
  case EPhase::resolver_semantic: return "SEMA";
  case EPhase::llvmir:            return "LLVM";
  case EPhase::linker:            return "LINK";
  }
}

std::string compiler::Phase_to_str(EPhase phase)
{
  switch (phase) {
  case EPhase::filesystem:        return "file rule";
  case EPhase::lexer:             return "lexer";
  case EPhase::preprosessor:      return "preprocessor";
  case EPhase::parser:            return "parser";
  case EPhase::binder:            return "external module binder";
  case EPhase::shipowner:         return "shipowner";
  case EPhase::resolver_symbol:   return "resolver symbol";
  case EPhase::resolver_type:     return "resolver type";
  case EPhase::resolver_semantic: return "resolver semantic";
  case EPhase::llvmir:            return "LLVM IR";
  case EPhase::linker:            return "linker";
  }
}
