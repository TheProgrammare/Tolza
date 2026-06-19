#include "toolchain/parser_command.hpp"

#include <filesystem>

#include <CLIUtils/CLI11.hpp>

#include <common/common.hpp>
#include <common/fileutils.hpp>
#include <common/compiler_options.hpp>


#include "cli_wrapper.hpp"
#include "command_package.hpp"
#include "command_compiler.hpp"
#include "command_workspace.hpp"
#include "command_check.hpp"
#include "command_audit.hpp"
#include "command_build.hpp"
#include "toolchain/toolchain.hpp"

#include <common/toolchain_options.hpp>
#include <string>

namespace fs = std::filesystem;

void toolchain::Commander::init_command_package() noexcept
{
  auto* pkg = app.add_subcommand("packages", "Package manager commands");
  pkg->alias("pkg");
  {
    auto* install = pkg->add_subcommand("install", "Install package from the velox repository");
    install->add_option("package", name, "Package name to install")->required()->type_name("<package name>");
    install->callback([&]() { (void)command::package::install(name); });
  }
  {
    auto* remove = pkg->add_subcommand("remove", "Remove package");
    remove->add_option("package", name, "Package name to remove")->required()->type_name("<package name>");
    remove->callback([&]() { (void)command::package::remove(name); });
  }
  {
    auto* info = pkg->add_subcommand("info", "Display package informations");
    info->add_option("package", name, "Package name to display")->required()->type_name("<package name>");
    info->callback([&]() { (void)command::package::info(name); });
  }
  {
    auto* purge = pkg->add_subcommand("purge", "Remove package configuration");
    purge->add_option("package", name, "Package name to purge")->required()->type_name("<package name>");
    purge->callback([&]() { (void)command::package::purge(name); });
  }
  {
    auto* check = pkg->add_subcommand("check", "Check package integrity");
    check->add_option("package", name, "Package name to check")->required()->type_name("<package name>");
    check->callback([&]() { (void)command::package::check(name); });
  }
  {
    auto* list = pkg->add_subcommand("list", "Display all packages package");
    list->add_option("package", regex_name, "Filter list from a regex")->type_name("[package name filter]");
    list->add_flag("--installed", installed, "Filter list on installed packages only");
    list->add_flag("--upgradable", upgradable, "Filter list on upgradable packages only");
    list->callback([&]() { (void)command::package::list(regex_name, installed, upgradable); });
  }
  {
    auto* update = pkg->add_subcommand("update", "Update package manager cache");
    update->callback([&]() { (void)command::package::update(); });
  }
  {
    auto* upgrade = pkg->add_subcommand("upgrade", "Upgrade all packages");
    upgrade->callback([&]() { (void)command::package::upgrade(); });
  }
}


void toolchain::Commander::init_command_workspace() noexcept
{
  auto* workspace = app.add_subcommand();
  auto* create    = workspace->add_subcommand("create", "Create a new velox workspace to start your project");
  auto* check =
      workspace->add_subcommand("check", "Check velox workspace if contains all necessary files and directories");
  {
    auto* create_ws = create->add_subcommand("workspace", "Create a new workspace directory with name");
    create_ws->add_option("--name", name, "If no name provided, a input prompt will appear");
    create_ws->add_option("--path", from_path, "If no path provided, the current directory will be used");
    create_ws->add_flag("-f", force, "Force the creation");
    create_ws->callback([&]() {
      std::string dest_path = from_path.empty() ? fs::current_path().string() : from_path;
      std::string dest_name = name;
      if (dest_name.empty()) {
        dest_name = cli::ask_filename();
        if (dest_name.empty()) return;
      }
      (void)command::workspace::generate_velox_workspace(dest_name, dest_path, force);
    });
  }
  {
    auto* create_conf = create->add_subcommand("config", "Create a new .toml file");
    create_conf->add_option("path", from_path, "If no path provided, the current directory will be used");
    create_conf->add_option("name", name, "If no name provided, a input prompt will appear");
    create_conf->callback([&]() {
      if (name.empty()) {
        name = cli::ask_filename();
        if (name.empty()) return;
      }
      if (from_path.empty()) {
        from_path = fs::current_path();
      }

      common::compiler::Options c(name);
      (void)c.write_config(from_path);
    });
  }
  {
    auto* gui = create->add_subcommand("gui", "Open the toolchain interface");
    gui->alias("ui");
    gui->add_option("path", from_path, "If no path provided, the current directory will be used");
    gui->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      // command::workspace::gui(path);
    });
  }
  {
    auto* check_ws = check->add_subcommand("workspace", "Check workspace directory");
    check_ws->add_option("path", from_path, "If no path provided, the current directory will be used");
    check_ws->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      (void)command::check::check_workspace(common::fileutils::resolve_path(from_path));
    });
  }
  {
    auto* check_conf = check->add_subcommand("config", "Check .toml file");
    check_conf->add_option("path", from_path, "If no path provided, the current directory will be used");
    check_conf->add_option("--full,-f", full, "Set the checker in full mode");
    check_conf->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      (void)command::check::check_velox_config(common::fileutils::resolve_path(from_path), full);
    });
  }
  {
    auto* audit = app.add_subcommand("audit", "Produce an audit report of your velox project");
    audit->add_option("path", from_path, "If no path provided, the current directory will be used");
    audit->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      command::audit::audit_workspace(common::fileutils::resolve_path(from_path));
    });
  }
  {
    auto* sync = workspace->add_subcommand("sync", "Synchronize the workspace module tree with the filesystem");
    sync->add_option("path", from_path, "If no path provided, the current directory will be used");
    sync->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      command::workspace::synchronize(common::fileutils::resolve_path(from_path));
    });
  }
}

void toolchain::Commander::init_command_build() noexcept
{
  auto* build = app.add_subcommand("build", "Compile your velox project");
  build->alias("b");
  build->add_option("path", from_path, "Path to the .toml project file to get the compilation context")
      ->type_name("<config path>")
      ->required()
      ->expected(1);

  build->callback([&]() {
    from_path = common::fileutils::resolve_path(from_path);

    if (!fs::exists(from_path)) {
      std::cerr << "[build:Error] The file path dosen't exist\n";
      exit(1);
    }

    if (!fs::is_regular_file(from_path)) {
      std::cerr << "[build:Error] The path is not a file\n";
      exit(1);
    }

    auto        ctx = common::compiler::Options::read_config(from_path);
    std::string cmd = "build " + from_path + " ";
    for (const auto& arg : ctx.to_args()) cmd += arg + " ";
    cmd += "\n";

    (void)toolchain::exec_compiler_cmd(cmd);
  });
}

void toolchain::Commander::init_commands() noexcept
{
  init_command_build();
  init_command_package();
  init_command_workspace();
}

void toolchain::Commander::exec_ffi_command() noexcept
{
  if (ffi_c_flag) {
    (void)command::build::generate_ffi_c(from_path, to_path);
  } else if (ffi_c_flag) {
    (void)command::build::generate_ffi_json(from_path, to_path);
  } else {
    common::FATAL_ERROR("ffi command failed, generation flag mode is required");
  }
}
