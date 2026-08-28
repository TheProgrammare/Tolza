#include "compiler.hpp"

#include "compiler/io.hpp"
#include "misc/error_output.hpp"
#include "misc/notification/notification.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/module.hpp"
#include "nexus/resolved.hpp"
#include "nexus/type/type.hpp"
#include "nexus/unresolved.hpp"
#include "pipeline/codegen.hpp"
#include "pipeline/linker.hpp"
#include "pipeline/module_resolver.hpp"
#include "pipeline/pipeline.hpp"
#include "pipeline/preparer.hpp"
#include "pipeline/resolver.hpp"

#include <algorithm>
#include <chrono>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>
#include <print>
#include <string>


pipeline::Pipeline compiler::pipeline = pipeline::Pipeline();

unresolved::Arena compiler::unresolved = unresolved::Arena();
resolved::Arena   compiler::resolved   = resolved::Arena();
inference::Arena  compiler::inference  = inference::Arena();


compiler::Compiler         compiler::COMPILER = compiler::Compiler();
common::compiler::Manifest compiler::OPTIONS  = common::compiler::Manifest();

namespace fs = std::filesystem;

void compiler::Compiler::add_error(const Error_Diagnostic& error)
{
  auto it = std::ranges::find_if(errors, [&](const auto& elem) -> bool { return elem.first == error.elem_first.cuid; });

  if (it != errors.end()) {
    auto& vec = it->second;
    vec.emplace_back(error);
  } else {
    errors.emplace_back(error.elem_first.cuid, std::vector({error}));
  }


  if (compiler::OPTIONS.diagnostic.error == common::compiler::EErrorMode::fail_fatal) {
    print_errors();
    if (!compiler::OPTIONS.is_check_mode) {
      auto end      = std::chrono::high_resolution_clock::now();
      auto tp       = std::chrono::system_clock::time_point(std::chrono::milliseconds(start_compilation_time));
      auto total_ms = static_cast<long long>(std::chrono::duration<double, std::milli>(end - tp).count());

      const auto m  = total_ms / 60'000;
      const auto s  = (total_ms / 1'000) % 60;
      const auto ms = total_ms % 1'000;


      IO::println(stderr, IO_PASS::NONE, "Compilation failed - {}m {}s {}ms", m, s, ms);
      notification::notify("Tolza-Compiler", std::format("Compilation failed - {}m {}s {}ms", m, s, ms), false);
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
  auto start             = std::chrono::system_clock::now();
  start_compilation_time = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();


  // if (compiler::OPTIONS.dir.current_profile.empty() && !mute) {
  //   IO::println(
  //       "\n[build:warning] Raw compilation command detected, "
  //       "please use 'tolza-toolchain' to develop proprely with the Tolza programming language.\n");
  // }

  {
    auto p = fs::path(compiler::OPTIONS.get_dir_binding_profile()) / "C.tlz";
    if (!fs::exists(p)) {
      (void)pipeline.generate_libc_wrappers();
    } else {
      auto binding_time = fs::last_write_time(p);

      auto newest_profile = fs::last_write_time(compiler::OPTIONS.get_project_manifest());
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

  if (compiler::OPTIONS.log.level != common::compiler::ELogLevel::quiet) {
    IO::println("Compilation Started");
    size_t count = 1;
    for (auto& profile_name : compiler::OPTIONS.profiles) {
      auto p = (fs::path(compiler::OPTIONS.dir.get_dir_profile()) / profile_name).string();
      IO::println("  Profile {}: \"{}\"", count++, p);
    }
  }

  // filesystem
  fs::path target_dir = compiler::OPTIONS.dir.get_dir_source();
  auto     CUs        = compiler::pipeline.query_CUs_at_dir(cu::ID::main(), target_dir.string());

  if (CUs.empty()) {
    if (compiler::OPTIONS.log.level != common::compiler::ELogLevel::quiet) {
      IO::println(
          R"([build] No files found at the source folder path:
  "{}"
  Check if the source folder path is correct.
  Or start your project by creating your first script in the source folder path.)",
          target_dir.string());
    }
    return false;
  }

  while (!compiler::pipeline.unprepared_compilation_units.empty()) {
    for (auto cuid : compiler::pipeline.unprepared_compilation_units) (void)preparer::prepare_cu(cuid);

    for (auto cuid : compiler::pipeline.prepared_compilation_units)
      compiler::pipeline.unprepared_compilation_units.erase(cuid);

    for (const auto& [cuid, errs] : errors) {
      compiler::pipeline.unprepared_compilation_units.erase(cuid);
    }

    if (compiler::pipeline.unprepared_compilation_units.empty()) {
      (void)module_resolver::resolve_modules(compiler::pipeline.prepared_compilation_units);

      if (!errors.empty()) goto compiler_failed;

      (void)compiler::pipeline.engage_bindings();
    }
  }


  {
    bool analyzer_success = true;
    for (auto cuid : compiler::pipeline.prepared_compilation_units) {
      if (!resolver::resolve_cu(cuid)) analyzer_success = false;
    }

    if (!analyzer_success) goto compiler_failed;
  }

  // no building or code emission in check mode
  if (compiler::OPTIONS.is_check_mode) return true;

  codegen::generate_target_machine();

  {
    bool codegen_success = true;
    for (auto cuid : compiler::pipeline.analyzed_compilation_units) {
      if (!codegen::codegen_cu(cuid)) codegen_success = false;
    }

    if (!codegen_success) goto compiler_failed;
  }

  if (!linker::link_modules()) return false;


  fs::create_directories(compiler::OPTIONS.get_dir_build_profile());
  fs::create_directories(compiler::OPTIONS.get_dir_debug_graph());
  fs::create_directories(compiler::OPTIONS.get_dir_llvmir());
  fs::create_directories(compiler::OPTIONS.get_dir_preprocess());
  fs::create_directories(compiler::OPTIONS.get_dir_binding_profile());

  if (!linker::emit()) return false;

  if (!linker::link_executable()) return false;


  {
  compiler_successful:
    auto end      = std::chrono::high_resolution_clock::now();
    auto tp       = std::chrono::system_clock::time_point(std::chrono::milliseconds(start_compilation_time));
    auto total_ms = static_cast<long long>(std::chrono::duration<double, std::milli>(end - tp).count());

    const auto m  = total_ms / 60'000;
    const auto s  = (total_ms / 1'000) % 60;
    const auto ms = total_ms % 1'000;

    const std::string msg = std::format("Compilation successfully ended - {}m {}s {}ms", m, s, ms);

    IO::println("{}", msg);
    notification::notify("Tolza-Compiler", msg, true);

    return true;
  }

  {
  compiler_failed:
    compiler::COMPILER.print_errors();
    if (!compiler::OPTIONS.is_check_mode) {
      auto end      = std::chrono::high_resolution_clock::now();
      auto tp       = std::chrono::system_clock::time_point(std::chrono::milliseconds(start_compilation_time));
      auto total_ms = static_cast<long long>(std::chrono::duration<double, std::milli>(end - tp).count());

      const auto m  = total_ms / 60'000;
      const auto s  = (total_ms / 1'000) % 60;
      const auto ms = total_ms % 1'000;

      const std::string msg = std::format("Compilation failed - {}m {}s {}ms", m, s, ms);

      IO::println(stderr, IO_PASS::NONE, "{}", msg);
      notification::notify("Tolza-Compiler", msg, false);
    }
    return false;
  }
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
