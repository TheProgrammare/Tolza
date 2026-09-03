#include "parser_command.hpp"

#include "binder/ffi_c_reader.hpp"
#include "binder/ffi_json_reader.hpp"
#include "compiler/compiler.hpp"
#include "compiler/io.hpp"
#include "pipeline/configurator.hpp"

#include <CLIUtils/CLI11.hpp>
#include <Neargye/magic_enum.hpp>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <filesystem>
#include <print>
#include <string>

namespace fs = std::filesystem;


compiler::Commander::Commander(CLI::App& _app, int argc, const char* argv[])
  : common::Commander(_app, argc, argv)
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

void compiler::Commander::init_command_build() noexcept
{
  auto* build = app.add_subcommand("build", "Compile Tolza project");
  build->alias("b");

  compilation_args(build);

  build->callback([&]() {
    auto f = common::fileutils::find_tolza_toml(opt.project_path);

    if (f.empty()) {
      IO::println(stderr, IO_PASS::NONE, "The project path at \"{}\" is invalid", opt.project_path);
      return;
    }

    configurator::init_compiler_OPTIONS(f, opt);

    IO::println("Command executed:");
    for (const auto& arg : args) IO::print_raw("{} ", arg);
    IO::print_raw("\n");

    COMPILER.run_requested = true;
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