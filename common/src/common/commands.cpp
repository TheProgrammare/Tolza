#include "commands.hpp"

#include <CLIUtils/CLI11.hpp>
#include <common/compiler_options.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/toolchain_options.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>


namespace fs = std::filesystem;


inline common::compiler::EOptimization parse_opti(std::string_view input)
{
  if (input.size() > 1 || input.empty()) return common::compiler::EOptimization::NONE;

  switch (input[0]) {
  case '0': return common::compiler::EOptimization::O0;
  case '1': return common::compiler::EOptimization::O1;
  case '2': return common::compiler::EOptimization::O2;
  case '3': return common::compiler::EOptimization::O3;
  case 's':
  case 'S': return common::compiler::EOptimization::Os;
  case 'z':
  case 'Z': return common::compiler::EOptimization::Oz;
  default:  return common::compiler::EOptimization::NONE;
  }
}
inline common::compiler::EWarnLevel parse_warnlevel(std::string_view input)
{
  int level{};

  try {
    level = std::stoi(std::string(input));
  } catch (...) {
    return common::compiler::EWarnLevel::NONE;
  }

  switch (level) {
  case 0:  return common::compiler::EWarnLevel::W0;
  case 1:  return common::compiler::EWarnLevel::W1;
  case 2:  return common::compiler::EWarnLevel::W2;
  case 3:  return common::compiler::EWarnLevel::W3;
  default: return common::compiler::EWarnLevel::NONE;
  }
}

common::Commander::Commander(CLI::App& _app, int argc, const char* argv[])
  : app(_app)
  , opt(*new common::compiler::Profile{})
  , args(argv, argv + argc)

{
  init_common_commands();
}


void common::Commander::compilation_args(CLI::App* build) noexcept
{
  auto add_opt_path = [&](std::string_view option_name, std::string& variable, std::string_view option_description) {
    return build
        ->add_option_function<fs::path>(
            std::string(option_name),
            [&](const fs::path& path) { variable = common::fileutils::resolve_path(path.string()); },
            std::string(option_description))
        ->type_name("<path>");
  };

  decltype(10) a;

#define new_flag(flag_name, flag, desc)                                                                                \
  build->add_flag_function(flag_name, [&](int64_t) { opt.flag = true; }, desc)->type_name("<flag>")

#define new_opt_flags(opt_name, type, var, desc)                                                                       \
  build                                                                                                                \
      ->add_option_function<std::string>(                                                                              \
          opt_name, [&](const std::string& input) { opt.var = type##_from_str(input); }, desc)                         \
      ->type_name("<type>")

#define new_opt_enum(opt_name, type, var, desc)                                                                        \
  build                                                                                                                \
      ->add_option_function<std::string>(                                                                              \
          opt_name, [&](const std::string& input) { opt.var = type##_from_str(input); }, desc)                         \
      ->type_name("<type>")

#define new_opt_parse(opt_name, var, fn, desc)                                                                         \
  build->add_option_function<std::string>(                                                                             \
           opt_name, [&](const std::string& input) { opt.var = fn(input); }, desc)                                     \
      ->type_name("<type>")

#define new_str(opt_name, var, desc) build->add_option(opt_name, opt.var, desc)->type_name("<text>");

  add_opt_path("path", opt.project_path, "Path to Tolza project");

  new_flag("--check,-c", is_check_mode, "Will compile without any codegen");

  // base
  new_str("--project-name", project_name, "Set the target project name");

  build
      ->add_option_function<std::string>(
          "--profiles,-p",
          [&](const std::string& s) {
            std::stringstream ss(s);
            std::string       item;

            while (std::getline(ss, item, '|')) opt.profiles.push_back(item);
          },
          "Set compilation profiles")
      ->type_name("<p1>|<p2>|...");

  // preset
  build
      ->add_option_function<std::string>(
          "--preset",
          [&](const std::string& s) {
            auto triple = common::compiler::TargetTriple::parse(s);
            opt         = common::compiler::Manifest::get_preset(triple);
          },
          "define a target triple "
          "baremetal, custom")
      ->type_name("<arch>-<vendor>-<platform>");

  // error stop mode
  new_opt_enum("--error-mode", common::compiler::EErrorMode, diagnostic.error,
               std::format("e.g. {}", common::compiler::EErrorMode_values));
  new_opt_enum("--diagnostic-format", common::compiler::EDiagnosticFormat, diagnostic.out_format,
               std::format("e.g. {}", common::compiler::EDiagnosticFormat_values));

  // target
  new_opt_enum("--arch", common::env::EArch, target.triple.arch, std::format("e.g. {}", common::env::EArch_values));
  new_opt_enum("--platform", common::env::EPlatform, target.triple.platform,
               std::format("e.g. {}", common::env::EPlatform_values));
  new_opt_enum("--vendor", common::env::EVendor, target.triple.vendor,
               std::format("e.g. {}", common::env::EVendor_values));
  new_opt_enum("--abi", common::env::EABI, target.triple.abi, std::format("e.g. {}", common::env::EABI_values));
  new_str("--cpu", target.cpu, "e.g. generic, core-avx2, znver4");
  new_opt_flags("--features", common::compiler::FCPUFeature, target.features,
                std::format("e.g. {}", common::compiler::FCPUFeature_values));
  new_opt_enum("--call-conv", common::env::ECallConvention, target.call_convention,
               std::format("e.g. {}", common::env::ECallConvention_values));
  new_opt_enum("--code-model", common::compiler::ECodeModel, target.code_model,
               std::format("e.g. {}", common::compiler::ECodeModel_values));
  new_opt_enum("--reloc-model", common::compiler::ERelocModel, target.reloc_model,
               std::format("e.g. {}", common::compiler::ERelocModel_values));

  // profile
  auto* opt_debug = new_flag("--debug,-d", profile.debug, "Compile in debug mode");
  auto* opt_release =
      build->add_flag("--release,-r", [&](std::size_t) { opt.profile.debug = false; }, "Compile in release mode");
  opt_debug->excludes(opt_release);
  new_opt_parse("--optimization,-O", profile.optimization, parse_opti, "Optimization level 0, 1, 2, 3, s, z");


  // clang
  new_opt_enum("--libc", common::env::ELibC, c_ffi.libc, std::format("e.g. {}", common::env::ELibC_values));
  new_opt_parse("--libc-version", c_ffi.libc_version, compiler::Cffi::LibCVersion::parse, "major.minor, e.g. 1.20");

  // log
  new_opt_flags("--logs", common::compiler::FPass, log.logs, std::format("e.g. {}", common::compiler::FPass_values));
  new_opt_enum("--log-level", common::compiler::ELogLevel, log.level,
               std::format("e.g. {}", common::compiler::ELogLevel_values));
  auto* opt_notify = new_flag("--notify", log.notify, "Notify the compilation result");
  auto* opt_no_notify =
      build->add_flag("--no-notify", [&](std::size_t) { opt.log.notify = false; }, "No notify the compilation result");
  opt_notify->excludes(opt_no_notify);


  // warn
  new_opt_flags("--warns", common::compiler::FWarnMode, warn.warns,
                std::format("e.g. {}", common::compiler::FWarnMode_values));
  new_opt_parse("--warning,-W", warn.level, parse_warnlevel, "Warn level 0, 1, 2, 3");

  new_opt_flags("--debugs", common::compiler::FDebugPrinter, debug.debugs,
                std::format("e.g. {}", common::compiler::FDebugPrinter_values));

  build
      ->add_option_function<std::string>(

          "-D",
          [&](std::string_view text) {
            auto pos = text.find('=');
            if (pos == std::string::npos) {
              opt.preprocessor.defines.emplace_back(std::string(text), "true");
            } else {
              opt.preprocessor.defines.emplace_back(std::string(text.substr(0, pos)), text.substr(pos + 1));
            }
          }

          )
      ->type_name("<key>");

  build
      ->add_option_function<std::string>(
          "-U", [&](std::string_view text) { opt.preprocessor.undefines.emplace_back(text.data()); })
      ->type_name("<key>");

  new_opt_flags("--emits", compiler::FEmit, target.emits, "Code emission");

  add_opt_path("--overlay", opt.dir.overlay, "Set overlay directory");
  add_opt_path("--dir-build", opt.dir.build, "Set the build directory");
  add_opt_path("--dir-src", opt.dir.source, "Set the source code directory");
  add_opt_path("--dir-vendor", opt.dir.vendor, "Set the vendor source code directory");
  add_opt_path("--dir-ffi-json", opt.dir.ffi_json, "Set the vendor source code directory");
  add_opt_path("--dir-binding", opt.dir.binding, "Set the binding directory");
  add_opt_path("--dir-compiler", opt.dir.compiler, "Set the compiler directory");
  add_opt_path("--dir-stdlib", opt.dir.stdlib, "Set the stdlib directory");
  add_opt_path("--dir-packages", opt.dir.packages, "Set the packages directory");

  build->add_flag("--llvm-verify_module", opt.llvm.verify_module, "Enable llvm verification before and after passes");

  build->allow_extras();

#undef new_flag
#undef new_opt
}


void common::Commander::init_command_compiler() noexcept
{
  auto* compiler = app.add_subcommand("compiler", "Compiler manager commands");
  compiler->alias("c");

  {
    auto* find        = compiler->add_subcommand("find", "Search of tolza-compiler");
    auto* opt_all     = find->add_flag("--all", compiler_all, "Find all tolza-compiler installed");
    auto* opt_version = find->add_option("--version", compiler_version, "Find specific tolza-compiler version");
    auto* opt_latest  = find->add_flag("--latest", compiler_latest, "Find latest tolza-compiler version");
    auto* opt_path    = find->add_option("path", from_path,
                                         "If no path provided, will use standard path or custom path in toolchain.toml");

    opt_all->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_all, opt_latest);
    opt_latest->excludes(opt_all, opt_version);

    find->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);

      auto found = [](std::string_view c) { std::println("[compiler] Found at \"{}\"", c); };

      if (!from_path.empty()) {
        if (compiler_all) {
          if (auto result = common::env::find_all_compilers(from_path); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (compiler_latest) {
          if (auto result = common::env::find_latest_compiler(from_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!compiler_version.empty()) {
          if (auto result = common::env::find_compiler_version(from_path, compiler_version); !result.empty()) {
            found(result);
            return;
          }
        }
        return;
      }

      for (auto& dir : common::env::get_compiler_dirs()) {
        if (compiler_all) {
          if (auto result = common::env::find_all_compilers(dir); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (compiler_latest) {
          if (auto result = common::env::find_latest_compiler(from_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!compiler_version.empty()) {
          if (auto result = common::env::find_compiler_version(from_path, compiler_version); !result.empty()) {
            found(result);
            return;
          }
        }
      }
      std::println("[tolza:compiler] No compiler found.");
    });
  }
  {
    auto* set         = compiler->add_subcommand("set", "Set the compiler used in tolza.toml configuration");
    auto* opt_path    = set->add_option("path", from_path, "Set the path of the tolza-compiler used");
    auto* opt_latest  = set->add_option("--latest", compiler_latest, "Set the latest tolza-compiler used");
    auto* opt_version = set->add_option("--version,-v", compiler_version, "Set specific tolza-compiler version used");
    auto* opt_custom_path = set->add_option("--custom-path", to_path, "Set the custom compiler directory to search");
    opt_path->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_path, opt_latest, opt_custom_path);
    opt_latest->excludes(opt_path, opt_version, opt_custom_path);

    set->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);
      to_path   = common::fileutils::resolve_path(to_path);


      if (!from_path.empty()) {
        if (!to_path.empty()) {
          common::toolchain::OPTIONS.custom_compiler_dir = to_path;
          return;
        }
        common::toolchain::Options::apply_compiler(from_path);
        return;
      }

      for (auto& dir : common::env::get_compiler_dirs()) {
        if (compiler_latest) {
          auto result = common::env::find_latest_compiler(dir);
          if (!result.empty()) {
            common::toolchain::OPTIONS.compiler_used = result;
            return;
          }
        } else if (!compiler_version.empty()) {
          auto result = common::env::find_compiler_version(dir, compiler_version);
          if (!result.empty()) {
            common::toolchain::OPTIONS.compiler_used = result;
            return;
          }
        }
      }
    });
  }
#ifdef TOLZA_TOOLCHAIN
  {
    auto* cogito = compiler->add_subcommand("cogito", "Check if the file is indeed the tolza-compiler");
    cogito->add_option("path", from_path, "If no path provided, will check the compiler used in the tolza.toml");
    cogito->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);
      toolchain::Options::cogito_compiler(
          from_path.empty() ? common::fileutils::resolve_path(common::toolchain::OPTIONS.compiler_used) : from_path);
    });
  }
#endif
}


void common::Commander::init_command_ffi() noexcept
{
  auto* ffi = app.add_subcommand("ffi", "Translate ffi representation to .tlzbind wrappers");
  ffi->add_option("--from,-f", from_path, "Source of the ffi representation")
      ->required()
      ->expected(1)
      ->type_name("<source>");
  ffi->add_option("--to,-t", to_path, "Directory destination of the .tlzbind generated")
      ->required()
      ->expected(1)
      ->type_name("<dest>");
  auto* flag_json = ffi->add_flag("--json,-j", ffi_json_flag, "JSON generation mode according to the source format");
  auto* flag_c    = ffi->add_flag("--c,-c", ffi_c_flag, "C language generation mode according to the source format");
  flag_json->excludes(flag_c);

  ffi->callback([&]() { exec_ffi_command(); });
}

void common::Commander::init_common_commands() noexcept
{
  init_command_compiler();
  init_command_ffi();
}
