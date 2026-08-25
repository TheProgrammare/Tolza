#include "toolchain/parser_command.hpp"

#include <filesystem>
#include <string>
#include <print>

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

namespace fs = std::filesystem;

void toolchain::Commander::init_command_package() noexcept
{
  auto* pkg = app.add_subcommand("packages", "Package manager commands");
  pkg->alias("pkg");
  {
    auto* install = pkg->add_subcommand("install", "Install package from the tolza repository");
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
    list->add_flag("--installed,-i", installed, "Filter list on installed packages only");
    list->add_flag("--upgradable,-u", upgradable, "Filter list on upgradable packages only");
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

void toolchain::Commander::init_command_toolchain() noexcept
{
  auto* cogito = app.add_subcommand("cogito", "Ask if the compiler is detected");

  cogito->callback([&]() { command::compiler::cogito_compiler(common::toolchain::OPTIONS.compiler_used); });
}


void toolchain::Commander::init_command_check() noexcept
{
  {
    auto* check = app.add_subcommand("check", "Check tolza files");

    {
      auto* check_ws = check->add_subcommand("workspace", "Check workspace directory");
      check_ws->alias("ws");
      check_ws->add_option("path", from_path, "If no path provided, the current directory will be used");
      check_ws->callback([&]() {
        if (from_path.empty()) from_path = fs::current_path();

        (void)command::check::check_workspace(common::fileutils::resolve_path(from_path));
      });
    }
    {
      auto* check_profile = check->add_subcommand("profile", "Check .toml profile file");
      check_profile->alias("p");
      check_profile->add_option("path", from_path, "If no path provided, the current directory will be used");
      check_profile->add_option("--full,-f", full, "Set the checker in full mode");
      check_profile->callback([&]() {
        if (from_path.empty()) from_path = fs::current_path();

        (void)command::check::check_tolza_config(common::fileutils::resolve_path(from_path), full);
      });
    }
  }
  {
    auto* audit = app.add_subcommand("audit", "Produce an audit report of your tolza project");
    audit->add_option("path", from_path, "If no path provided, the current directory will be used");
    audit->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      command::audit::audit_workspace(common::fileutils::resolve_path(from_path));
    });
  }
  {
    auto* sync = app.add_subcommand("sync", "Synchronize the workspace module tree with the filesystem");
    sync->add_option("path", from_path, "If no path provided, the current directory will be used");
    sync->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      command::workspace::synchronize(common::fileutils::resolve_path(from_path));
    });
  }
}

void toolchain::Commander::init_command_new() noexcept
{
  auto* _new = app.add_subcommand("new", "Create a new tolza workspace to start your project");

  {
    auto* _new_ws = _new->add_subcommand("workspace", "Create a new workspace directory with name");
    _new_ws->alias("ws");
    _new_ws->add_option("path", from_path, "If no path provided, the current directory will be used");
    _new_ws->add_option("--name,-n", name, "If no project name provided, a input prompt will appear");
    _new_ws->add_flag("-f", force, "Force the creation");
    _new_ws->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);
      if (fs::is_regular_file(from_path)) {
        std::println("[tolza] The path provided \"{}\" must be a directory.", from_path);
        return;
      }
      if (name.empty()) {
        name = cli::ask_text("Write down the project name");
        if (name.empty()) return;
      }
      (void)command::workspace::generate_tolza_workspace(name, from_path, force);
    });
  }
  {
    auto* _new_profile = _new->add_subcommand("profile", "Create a new .toml profile file");
    _new_profile->alias("p");
    _new_profile->add_option("path", from_path, "If no path provided, the current directory will be used");
    _new_profile->add_option("--name,-n", name, "If no profile name provided, a input prompt will appear");
    _new_profile->add_flag("--release,-r", is_release,
                           "Default profile preset. Will config the profile to a release build");
    _new_profile->add_flag("--debug,-d", is_debug, "Will config the profile to a debug build");
    _new_profile->callback([&]() {
      if (fs::exists(from_path) && fs::is_regular_file(from_path)) {
        std::println("[tolza] The profile file at \"{}\" already exists.", from_path);
        if (!cli::yes_no_question("Do you want to override it ?")) return;
      }

      if (name.empty()) {
        name = cli::ask_text("Write down the profile name");
        if (name.empty()) return;
      }
      if (from_path.empty()) {
        from_path = fs::current_path();
      }

      auto c = common::compiler::Options::get_current(name);
      from_path =
          (fs::is_regular_file(from_path)) ? from_path : (fs::path(from_path) / std::string(name + ".toml")).string();
      from_path = common::fileutils::resolve_path(from_path);

      if (c.write_config(from_path, is_debug)) {
        std::println("[tolza] profile file has been created at \"{}\"", from_path);
      } else {
        std::println("[tolza:ERROR] profile file cannot be created at \"{}\"", from_path);
      }
    });
  }
  {
    auto* gui = _new->add_subcommand("gui", "Open the toolchain interface");
    gui->alias("ui");
    gui->add_option("path", from_path, "If no path provided, the current directory will be used");
    gui->callback([&]() {
      if (from_path.empty()) from_path = fs::current_path();

      // command::workspace::gui(path);
    });
  }
}


void toolchain::Commander::init_command_build() noexcept
{
  auto* build = app.add_subcommand("build", "Compile Tolza project");
  build->alias("b");
  build->add_option("path", from_path, "Path to the .toml project file to get the compilation context")
      ->type_name("<config path>");

  build->callback([&]() {
    from_path = common::fileutils::resolve_path(from_path);

    if (!fs::exists(from_path)) {
      std::println(stderr, "[build:Error] The file path dosen't exist.");
      exit(1);
    }

    if (!fs::is_regular_file(from_path)) {
      std::println("[build:Error] The path is not a tolza.toml file.");
      exit(1);
    }

    auto        ctx = common::compiler::Options::read_config(from_path);
    std::string cmd = std::format("build {} ", from_path);
    for (const auto& arg : ctx.to_args()) std::format_to(std::back_inserter(cmd), "{} ", arg);
    cmd += "\n";

    (void)toolchain::exec_compiler_cmd(cmd);
  });
}


void toolchain::Commander::init_commands() noexcept
{
  app.set_version_flag("--version,-v", "Version: " SOFTWARE_VERSION);

  app.add_flag_function(
      "--about,-a",
      [&](int count) {
        std::println(toolchain::SOFTWARE_ABOUT);
        exit(0);
      },
      "Show detailed software info");

  init_command_toolchain();
  init_command_check();
  init_command_new();
  init_command_build();
  init_command_package();
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
