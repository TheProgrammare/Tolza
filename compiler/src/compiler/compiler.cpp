#include "compiler.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <print>
#include <ostream>
#include <string>
#include <filesystem>
#include <chrono>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LLVMContext.h>

#include <common/compiler_options.hpp>
#include <common/common.hpp>

#include "binder/ffi_c_reader.hpp"
#include "codegen/codegen_type.hpp"
#include "misc/notification/notification.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/type.hpp"
#include "nexus/definition.hpp"
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
common::compiler::Options compiler::OPTIONS  = common::compiler::Options();

namespace fs = std::filesystem;


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
  auto it = std::ranges::find_if(errors, [&](const auto& elem) -> bool { return elem.first == error.elem_first.cuid; });

  if (it != errors.end()) {
    auto& vec = it->second;
    vec.emplace_back(error);
  } else {
    errors.emplace_back(error.elem_first.cuid, std::vector({error}));
  }


  if (compiler::OPTIONS.diagnostic.error_mode == common::compiler::EErrorMode::fail_fatal) {
    print_errors();
    if (!compiler::OPTIONS.is_check_mode) {
      std::println("[tolza-compiler] Compilation failed");
      notification::notify("Tolza-Compiler", "Compilation failed", false);
    }
    std::exit(1);
  }
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

  // if (compiler::OPTIONS.dir.current_profile.empty() && !mute) {
  //   std::println(
  //       "\n[build:warning] Raw compilation command detected, "
  //       "please use 'tolza-toolchain' to develop proprely with the Tolza programming language.\n");
  // }

  {
    auto p = fs::path(compiler::OPTIONS.get_dir_binding_profile()) / "C.tlz";
    if (!fs::exists(p)) {
      (void)pipeline.generate_libc_wrappers();
    } else {
      auto binding_time = fs::last_write_time(p);

      auto newest_profile = fs::last_write_time(fs::path(compiler::OPTIONS.dir.get_dir_project()) / "tolza.toml");
      for (const auto& profile : compiler::OPTIONS.profiles) {
        auto profile_t =
            fs::last_write_time(fs::path(compiler::OPTIONS.dir.get_dir_profile()) / std::string(profile + ".toml"));

        newest_profile = std::max(newest_profile, profile_t);
      }

      if (newest_profile > binding_time) {
        (void)pipeline.generate_libc_wrappers();
      }
    }
  }

  if (!mute) {
    std::println("\n[tolza-compiler] Compilation Started");
    size_t count = 1;
    for (auto& profile_name : compiler::OPTIONS.profiles) {
      auto p = (fs::path(compiler::OPTIONS.dir.get_dir_profile()) / profile_name).string();
      std::println("  Profile {}: \"{}\'", count++, p);
    }
  }

  // filesystem
  fs::path target_dir = compiler::OPTIONS.dir.get_dir_source();
  auto     CUs        = compiler::pipeline.query_CUs_at_dir(cu::ID::main(), target_dir.string());

  if (CUs.empty()) {
    if (!mute) {
      std::println(
          R"([build] No files found at the source folder path:
  "{}"
  Check if the source folder path is correct.
  Or start your project by creating your first script in the source folder path.)",
          target_dir.string());
    }
    return false;
  }

  while (!compiler::pipeline.unprepared_compilation_units.empty()) {
    for (auto cuid : compiler::pipeline.unprepared_compilation_units) (void)compiler::pipeline.engage_preparer(cuid);

    for (auto cuid : compiler::pipeline.prepared_compilation_units)
      compiler::pipeline.unprepared_compilation_units.erase(cuid);

    for (const auto& [cuid, errs] : errors) {
      compiler::pipeline.unprepared_compilation_units.erase(cuid);
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

  bool success = true;
  for (auto cuid : compiler::pipeline.analyzed_compilation_units) {
    if (!compiler::pipeline.engage_generator(cuid)) success = false;
  }

  if (!success) {
    auto end   = std::chrono::high_resolution_clock::now();
    auto milli = std::chrono::duration<double, std::milli>(end - start).count();

    std::println("[tolza-compiler] Compilation failed {:.3f} ms", milli);
    notification::notify("Tolza-Compiler", std::format("Compilation failed - {:.3f} ms", milli), false);
    return false;
  }

  // no building or code emission in check mode
  if (compiler::OPTIONS.is_check_mode) return true;

  if (!compiler::pipeline.engage_module_linker()) {
    return false;
  }

  fs::create_directories(compiler::OPTIONS.dir.get_dir_build());
  fs::create_directories(compiler::OPTIONS.get_dir_debug_graph());
  fs::create_directories(compiler::OPTIONS.get_dir_llvmir());
  fs::create_directories(compiler::OPTIONS.get_dir_preprocess());
  fs::create_directories(compiler::OPTIONS.get_dir_binding_profile());

  if (!pipeline::Pipeline::engage_general_emitter()) {
    return false;
  }

  if (!pipeline::Pipeline::engage_linker()) {
    return false;
  }

  auto end   = std::chrono::high_resolution_clock::now();
  auto milli = std::chrono::duration<double, std::milli>(end - start).count();

  std::println("[tolza-compiler] Compilation successfully ended {:.3f} ms", milli);
  notification::notify("Tolza-Compiler", std::format("Compilation successfully ended - {:.3f} ms", milli), true);

  return success;
}

void compiler::Compiler::print_errors() const
{
  if (errors.empty()) return;

  if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
    std::println(stderr, "@@TOLZA_EXORDIUM_DIAGNOSTICORUM@@\n[");

  const auto last_cuid = errors.back().first;
  for (const auto& [cuid, errs] : errors) {

    for (size_t i = 0; i < errs.size(); i++) {
      const auto err = errs[i];
      std::print(stderr, "{}", err.print_error());
      if (i != errs.size() - 1 && compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
        std::println(stderr, ",");
    }

    if (last_cuid != cuid && compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
      std::println(stderr, ",");
  }

  if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
    std::print(stderr, "]\n@@TOLZA_CLAUSULA_DIAGNOSTICORUM@@");
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
