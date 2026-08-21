#include "parser_command.hpp"

#include <print>
#include <string>
#include <filesystem>


#include <common/common.hpp>
#include <common/fileutils.hpp>
#include <common/compiler_options.hpp>

#include <CLIUtils/CLI11.hpp>
#include <Neargye/magic_enum.hpp>


#include "binder/ffi_c_reader.hpp"
#include "binder/ffi_json_reader.hpp"

// unused false positive
// mandatory
#include "CLI11_type_seralization.hpp"


#include "common/environment.hpp"
#include "compiler/compiler.hpp"


namespace fs = std::filesystem;


compiler::Commander::Commander(CLI::App& _app, int argc, const char* argv[])
  : common::Commander(_app)
  , args(argv, argv + argc)
  , opt(*new common::compiler::Sub_Compiler_Options())
{
  init_commands();
}


void compiler::Commander::init_command_cogito() noexcept
{
  auto* cogito   = app.add_subcommand("tolza-toolchain", "Revceive the toolchain cogito ask");
  auto* ergo_sum = cogito->add_subcommand("cogito", "Respond to René Descartes");
  ergo_sum->callback([&]() {
    std::println("[tolza-compiler] ergo sum\n {}", common::env::get_exe_dir());
    exit(0);
  });
}

void compiler::Commander::compilation_args(CLI::App* build) noexcept
{
  auto add_opt_path = [&](std::string_view option_name, std::basic_string<char>& variable,
                          std::string_view option_description) {
    return build
        ->add_option_function<fs::path>(
            std::string(option_name),
            [&](const fs::path& path) { variable = common::fileutils::resolve_path(path.string()); },
            std::string(option_description))
        ->type_name("<path>");
  };

#define new_flag(flag_name, flag, desc) build->add_flag(flag_name, opt.flag, desc)->type_name("<flag>")
#define new_opt(opt_name, var, desc)    build->add_option(opt_name, opt.var, desc)->type_name("<type>")

  add_opt_path("path", opt.current_config_file, "Path to the .toml project file to get the compilation context");

  new_flag("--mute", mute, "Mute any output log");

  // base
  new_opt("--project-name", project_name, "Set the target project name");
  new_opt("--sub-config", sub_configs, "Set the sub config to apply after main config");

  // preset
  build
      ->add_option_function<std::string>(
          "--preset",
          [&](const std::string& s) {
            auto triple       = common::compiler::TargetTriple::parse(s);
            compiler::OPTIONS = common::compiler::Options::get_preset(triple);
          },
          "define a target triple"
          "baremetal, custom")
      ->type_name("<arch>-<vendor>-<platform>");

  // error stop mode
  new_opt("--error-mode", diagnostic.error_mode, "e.g. " + common::compiler::EErrorMode_names());
  new_opt("--diagnostic-format", diagnostic.out_format, "e.g. " + common::compiler::EDiagnosticFormat_names());

  // target
  new_opt("--arch", target.triple.arch, "e.g. " + common::env::EArch_names());
  new_opt("--platform", target.triple.platform, "e.g. " + common::env::EPlatform_names());
  new_opt("--vendor", target.triple.vendor, "e.g. " + common::env::EVendor_names());
  new_opt("--abi", target.triple.abi, "e.g. " + common::env::EABI_names());
  new_opt("--cpu", target.cpu, "e.g. generic, core-avx2, znver4");
  new_opt("--features", target.features, "e.g. " + common::compiler::FCPUFeature_names());
  new_opt("--call-conv", target.call_convention, "e.g. " + common::env::ECallConvention_names());
  new_opt("--code-model", target.code_model, "e.g. " + common::compiler::ECodeModel_names());
  new_opt("--reloc-model", target.reloc_model, "e.g. " + common::compiler::ERelocModel_names());

  // profile
  auto* opt_debug = new_opt("--debug,-d", profile.debug, "Compile in debug mode");
  auto* opt_release =
      build->add_flag("--release,-r", [&](std::size_t count) { opt.profile.debug = false; }, "Compile in release mode");
  opt_debug->excludes(opt_release);
  new_opt("--optimization,-O", profile.optimization, "Optimization level 0, 1, 2, 3, s, z");


  // clang
  new_opt("--libc", c_ffi.libc, "e.g. " + common::env::ELibC_names());
  new_opt("--libc-version", c_ffi.libc_version, "major.minor, e.g. 1.20")->type_name("<version>");


  new_opt("--logs", log.logs, "e.g. " + common::compiler::FPass_names());

  new_opt("--warns", warn.warns, "e.g. " + common::compiler::FWarnMode_names());
  new_opt("--warning,-W", warn.level, "Warn level 0, 1, 2, 3");

  new_opt("--debugs", debug.debugs, "e.g. " + common::compiler::FDebugPrinter_names());

  build
      ->add_option_function<std::string>(

          "-D",
          [&](std::string_view text) {
            auto pos = text.find('=');
            if (pos == std::string::npos) {
              opt.preprocessor.defines[std::string(text)] = "true";
            } else {
              opt.preprocessor.defines[std::string(text.substr(0, pos))] = text.substr(pos + 1);
            }
          }

          )
      ->type_name("<key>");

  build
      ->add_option_function<std::string>(
          "-U", [&](std::string_view text) { opt.preprocessor.undefines.emplace_back(text.data()); })
      ->type_name("<key>");

  new_opt("--emits", target.emits, "Code emission");

  add_opt_path("--dir-project", opt.dir.project, "Set the project directory");
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


void compiler::Commander::init_command_build() noexcept
{
  auto* build = app.add_subcommand("build", "Compile Tolza project");
  build->alias("b");

  compilation_args(build);

  build->callback([&]() {
    compiler::OPTIONS = common::compiler::Options::read_config(opt.current_config_file);

    if (!compiler::OPTIONS.mute) {
      std::println("[tolza] Command executed:");
      for (const auto& arg : args) std::print("{} ", arg);
      std::println();
    }

    compiler::COMPILER.run_requested = true;
  });
}

void compiler::Commander::init_command_check() noexcept
{
  auto* check = app.add_subcommand("check", "Check Tolza code");

  compilation_args(check);

  check->callback([&]() {
    compiler::OPTIONS.mute = true;
    compiler::OPTIONS      = common::compiler::Options::read_config(opt.current_config_file);

    compiler::OPTIONS.is_check_mode         = true;
    compiler::OPTIONS.diagnostic.out_format = common::compiler::EDiagnosticFormat::json;

    if (!compiler::OPTIONS.mute) {
      std::println("[tolza] Command executed:");
      for (const auto& arg : args) std::print("{} ", arg);
      std::println();
    }

    compiler::COMPILER.run_requested = true;
  });
}

void compiler::Commander::init_commands() noexcept
{
  app.set_version_flag("--version,-v", "Version: " SOFTWARE_VERSION);

  app.add_flag_function(
      "--about,-a",
      [&](int count) {
        std::println(compiler::SOFTWARE_ABOUT);
        exit(0);
      },
      "Show detailed software info");

  init_command_build();
  init_command_check();
  init_command_cogito();
}


void compiler::Commander::exec_ffi_command() noexcept
{
  to_path   = common::fileutils::resolve_path(to_path);
  from_path = common::fileutils::resolve_path(from_path);

  if (!fs::exists(from_path)) {
    std::println(stderr, "The source path doesn't exists.");
    exit(1);
  }
  if (!fs::exists(to_path)) {
    std::println(stderr, "The destination path doesn't exists.");
    exit(1);
  }

  if (ffi_json_flag) {
    auto     ast       = ffi::JSON_Reader::parse_json_compilation_unit(from_path);
    fs::path dest_file = fs::path(to_path) / ast->bind.lang / ast->bind.lib;
    dest_file.replace_extension(common::fileutils::TOLZA_FILE_EXTENSION);

    ast->tolza_codegen(dest_file.string());

    exit(0);
  } else if (ffi_c_flag) {
    auto     ast       = ffi::C_Reader::parse_c_compilation_unit(from_path);
    fs::path dest_file = fs::path(to_path) / ast->bind.lang / ast->bind.lib;
    dest_file.replace_extension(common::fileutils::TOLZA_FILE_EXTENSION);

    ast->tolza_codegen(dest_file.string());

    exit(0);
  } else {
    std::println(stderr, "Unspecified ffi mode.");
    exit(1);
  }
}