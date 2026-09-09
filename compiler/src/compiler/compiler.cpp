#include "compiler.hpp"

#include "compiler/io.hpp"
#include "id/cuid.hpp"
#include "misc/error_output.hpp"
#include "misc/notification/notification.hpp"
#include "module/pool.hpp"
#include "nexus/forward.hpp"
#include "pipeline/codegen.hpp"
#include "pipeline/linker.hpp"
#include "pipeline/module_resolver.hpp"
#include "pipeline/pipeline.hpp"
#include "pipeline/preparer.hpp"
#include "pipeline/resolver.hpp"
#include "pool/node_resolved.hpp"
#include "pool/node_to_eval.hpp"
#include "pool/node_to_inf.hpp"
#include "pool/node_to_metadata.hpp"
#include "pool/node_unresolved.hpp"
#include "type/pool.hpp"

#include <algorithm>
#include <chrono>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <format>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Target/TargetMachine.h>
#include <print>
#include <ratio>
#include <string>
#include <vector>


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


  if (OPTIONS.diagnostic.error == common::compiler::EErrorMode::fail_fatal) {
    print_errors();
    if (!OPTIONS.is_check_mode) {
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
  : unresolved(*new unresolved::Arena())
  , resolved(*new resolved::Arena())
  , inference(*new inference::Arena())
  , semantic_metadata(*new semantic::Arena())
  , evaluated(*new evaluated::Arena())
{
  module::initialization();
  type::initialization();
}

bool compiler::Compiler::start_compilation()
{
  auto start             = std::chrono::system_clock::now();
  start_compilation_time = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();

  IO::println("Compilation Started");

  if (auto p = fs::path(OPTIONS.get_dir_binding_profile()) / "C.tlz"; !fs::exists(p)) {
    (void)PIPELINE.generate_libc_wrappers();
  } else {
    auto binding_time = fs::last_write_time(p);

    auto newest_profile = fs::last_write_time(OPTIONS.get_project_manifest());
    for (const auto& profile : OPTIONS.profiles) {
      auto profile_t = fs::last_write_time(fs::path(OPTIONS.dir.get_dir_profile()) / std::string(profile + ".toml"));

      newest_profile = std::max(newest_profile, profile_t);
    }

    if (newest_profile > binding_time) {
      (void)PIPELINE.generate_libc_wrappers();
    }
  }

  // filesystem
  fs::path target_dir = OPTIONS.dir.get_dir_source();
  auto     CUs        = PIPELINE.query_CUs_at_dir(cu::ID::main(), target_dir.string());

  if (CUs.empty()) {
    if (OPTIONS.log.level != common::compiler::ELogLevel::quiet) {
      IO::println(
          R"([build] No files found at the source folder path:
  "{}"
  Check if the source folder path is correct.
  Or start your project by creating your first script in the source folder path.)",
          target_dir.string());
    }
    return false;
  }

  while (!PIPELINE.unprepared_compilation_units.empty()) {
    for (auto cuid : PIPELINE.unprepared_compilation_units) (void)preparer::prepare_cu(cuid);

    for (auto cuid : PIPELINE.prepared_compilation_units) PIPELINE.unprepared_compilation_units.erase(cuid);

    for (const auto& [cuid, errs] : errors) {
      PIPELINE.unprepared_compilation_units.erase(cuid);
    }

    if (PIPELINE.unprepared_compilation_units.empty()) {
      (void)module_resolver::resolve_modules(PIPELINE.prepared_compilation_units);

      // avoid analyser on partial ast
      // if (!errors.empty()) goto compiler_failed;

      (void)PIPELINE.engage_bindings();
    }
  }


  {
    bool analyzer_success = true;
    for (auto cuid : PIPELINE.prepared_compilation_units) {
      if (!resolver::resolve_cu(cuid)) analyzer_success = false;
    }

    if (!analyzer_success) goto compiler_failed;
  }

  if (!errors.empty()) goto compiler_failed;

  // no building or code emission in check mode
  if (OPTIONS.is_check_mode) return true;

  codegen::generate_target_machine();

  {
    bool codegen_success = true;
    for (auto cuid : PIPELINE.analyzed_compilation_units) {
      if (!codegen::codegen_cu(cuid)) codegen_success = false;
    }

    if (!codegen_success) goto compiler_failed;
  }

  if (!linker::link_modules()) return false;


  fs::create_directories(OPTIONS.get_dir_build_profile());
  fs::create_directories(OPTIONS.get_dir_debug_graph());
  fs::create_directories(OPTIONS.get_dir_llvmir());
  fs::create_directories(OPTIONS.get_dir_preprocess());
  fs::create_directories(OPTIONS.get_dir_binding_profile());

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
    COMPILER.print_errors();
    if (!OPTIONS.is_check_mode) {
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

  if (OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
    std::println(stderr, "@@TOLZA_EXORDIUM_DIAGNOSTICORUM@@\n[");

  const auto last_cuid = errors.back().first;
  for (const auto& [cuid, errs] : errors) {

    for (size_t i = 0; i < errs.size(); i++) {
      const auto err = errs[i];
      std::print(stderr, "{}", err.print_error());
      if (i != errs.size() - 1 && OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
        std::println(stderr, ",");
    }

    if (last_cuid != cuid && OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
      std::println(stderr, ",");
  }

  if (OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json)
    std::println(stderr, "\n]\n@@TOLZA_CLAUSULA_DIAGNOSTICORUM@@");
}


std::string compiler::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case EPhase::filesystem:          return "FSYS";
  case EPhase::lexer:               return "LEXE";
  case EPhase::preprosessor:        return "PREP";
  case EPhase::parser:              return "PARS";
  case EPhase::binder:              return "EMBI";
  case EPhase::shipowner:           return "SHIP";
  case EPhase::resolver_symbol:     return "SYMB";
  case EPhase::resolver_type:       return "TYPE";
  case EPhase::resolver_semantic:   return "SEMA";
  case EPhase::resolver_evaluation: return "EVAL";
  case EPhase::llvmir:              return "LLVM";
  case EPhase::linker:              return "LINK";
  }
}
