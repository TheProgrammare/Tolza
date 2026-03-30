#include "toolchain/parser_command.hpp"

#include <filesystem>

#include <CLIUtils/CLI11.hpp>


#include "cli_wrapper.hpp"
#include "command_package.hpp"
#include "command_compiler.hpp"
#include "command_workspace.hpp"
#include "command_check.hpp"
#include "command_audit.hpp"
#include "command_build.hpp"
#include "common.hpp"
#include "toolchain_context.hpp"

namespace fs = std::filesystem;

void Command::init_command_package()
{
  auto pkg = app.add_subcommand("packages", "Package manager commands");
  pkg->alias("pkg");
  {
    auto install = pkg->add_subcommand("install", "Install package from the velox repository");
    install->add_option("package", name, "Package name to install")->required()->type_name("<package name>");
    install->callback([&]() { command::package::install(name); });
  }
  {
    auto remove = pkg->add_subcommand("remove", "Remove package");
    remove->add_option("package", name, "Package name to remove")->required()->type_name("<package name>");
    remove->callback([&]() { command::package::remove(name); });
  }
  {
    auto info = pkg->add_subcommand("info", "Display package informations");
    info->add_option("package", name, "Package name to display")->required()->type_name("<package name>");
    info->callback([&]() { command::package::info(name); });
  }
  {
    auto purge = pkg->add_subcommand("purge", "Remove package configuration");
    purge->add_option("package", name, "Package name to purge")->required()->type_name("<package name>");
    purge->callback([&]() { command::package::purge(name); });
  }
  {
    auto check = pkg->add_subcommand("check", "Check package integrity");
    check->add_option("package", name, "Package name to check")->required()->type_name("<package name>");
    check->callback([&]() { command::package::check(name); });
  }
  {
    auto list = pkg->add_subcommand("list", "Display all packages package");
    list->add_option("package", regex_name, "Filter list from a regex")->type_name("[package name filter]");
    list->add_flag("--installed", installed, "Filter list on installed packages only");
    list->add_flag("--upgradable", upgradable, "Filter list on upgradable packages only");
    list->callback([&]() { command::package::list(regex_name, installed, upgradable); });
  }
  {
    auto update = pkg->add_subcommand("update", "Update package manager cache");
    update->callback([&]() { command::package::update(); });
  }
  {
    auto upgrade = pkg->add_subcommand("upgrade", "Upgrade all packages");
    upgrade->callback([&]() { command::package::upgrade(); });
  }
}


void Command::init_command_compiler()
{
  auto compiler = app.add_subcommand("compiler", "Compiler manager commands");
  compiler->alias("c");

  {
    auto find        = compiler->add_subcommand("find", "Search of velox-compiler");
    auto opt_all     = find->add_flag("--all", all, "Find all velox-compiler installed");
    auto opt_version = find->add_option("--version", version, "Find specific velox-compiler version");
    auto opt_latest  = find->add_flag("--latest", latest, "Find latest velox-compiler version");
    auto opt_path    = find->add_option("path", dir_path,
                                        "If no path provided, will use standard path or custom path in toolchain.toml");

    opt_all->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_all, opt_latest);
    opt_latest->excludes(opt_all, opt_version);

    find->callback([&]() {
      dir_path = common::resolve_path(dir_path);

      auto found = [](const std::string& c) { std::cout << "[compuler] Found at \"" << c << "\"" << std::endl; };

      if (!dir_path.empty()) {
        if (all) {
          if (auto result = command::compiler::find_all_compilers(dir_path); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (latest) {
          if (auto result = command::compiler::find_latest_compiler(dir_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!version.empty()) {
          if (auto result = command::compiler::find_compiler_version(dir_path, version); !result.empty()) {
            found(result);
            return;
          }
        }
        return;
      }

      for (auto dir : common::get_compiler_dirs()) {
        if (all) {
          if (auto result = command::compiler::find_all_compilers(dir); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (latest) {
          if (auto result = command::compiler::find_latest_compiler(dir_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!version.empty()) {
          if (auto result = command::compiler::find_compiler_version(dir_path, version); !result.empty()) {
            found(result);
            return;
          }
        }
      }
      std::cout << "[compiler] No compiler found." << std::endl;
    });
  }
  {
    auto set             = compiler->add_subcommand("set", "Set the compiler used in velox.toml configuration");
    auto opt_path        = set->add_option("path", file_path, "Set the path of the velox-compiler used");
    auto opt_latest      = set->add_option("--latest", latest, "Set the latest velox-compiler used");
    auto opt_version     = set->add_option("--version,-v", version, "Set specific velox-compiler version used");
    auto opt_custom_path = set->add_option("--custom-path", dir_path, "Set the custom compiler directory to search");
    opt_path->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_path, opt_latest, opt_custom_path);
    opt_latest->excludes(opt_path, opt_version, opt_custom_path);

    set->callback([&]() {
      file_path = common::resolve_path(file_path);
      dir_path  = common::resolve_path(dir_path);

      if (!file_path.empty()) {
        if (!dir_path.empty()) {
          common::TOOL_CTX.custom_compiler_dir = file_path;
          return;
        }
        common::TOOL_CTX.compiler_used = file_path;
        return;
      }

      for (auto dir : common::get_compiler_dirs()) {
        if (latest) {
          auto result = command::compiler::find_latest_compiler(dir);
          if (!result.empty()) {
            common::TOOL_CTX.compiler_used = result;
            return;
          }
        } else if (!version.empty()) {
          auto result = command::compiler::find_compiler_version(dir, version);
          if (!result.empty()) {
            common::TOOL_CTX.compiler_used = result;
            return;
          }
        }
      }
    });
  }
  {
    auto cogito = compiler->add_subcommand("cogito", "Check if the file is indeed the velox-compiler");
    cogito->add_option("path", file_path, "If no path provided, will check the compiler used in the velox.toml");
    cogito->callback([&]() {
      file_path = common::resolve_path(file_path);
      command::compiler::cogito_compiler(file_path.empty() ? common::resolve_path(common::TOOL_CTX.compiler_used)
                                                           : file_path);
    });
  }
}


void Command::init_command_workspace()
{
  auto workspace = app.add_subcommand();
  auto create    = workspace->add_subcommand("create");
  auto check     = workspace->add_subcommand("check");
  {
    auto create_ws = create->add_subcommand("workspace", "Create a new workspace directory with name");
    create_ws->add_option("--name", name, "If no name provided, a input prompt will appear");
    create_ws->add_option("--path", dir_path, "If no path provided, the current directory will be used");
    create_ws->add_flag("-f", force, "Force the creation");
    create_ws->callback([&]() {
      std::string dest_path = dir_path.empty() ? fs::current_path().string() : dir_path;
      std::string dest_name = name;
      if (dest_name.empty()) {
        dest_name = cli::ask_filename();
        if (dest_name.empty()) return;
      }
      command::workspace::generate_velox_workspace(dest_name, dest_path, force);
    });
  }
  {
    auto create_conf = create->add_subcommand("config", "Create a new .toml file");
    create_conf->add_option("path", dir_path, "If no path provided, the current directory will be used");
    create_conf->add_option("name", name, "If no name provided, a input prompt will appear");
    create_conf->callback([&]() {
      if (name.empty()) {
        name = cli::ask_filename();
        if (name.empty()) return;
      }
      if (dir_path.empty()) {
        dir_path = fs::current_path();
      }

      command::workspace::write_config_file(common::resolve_path(dir_path), name, false);
    });
  }
  {
    auto gui = create->add_subcommand("gui", "Open the toolchain interface");
    gui->alias("ui");
    gui->add_option("path", dir_path, "If no path provided, the current directory will be used");
    gui->callback([&]() {
      if (dir_path.empty()) dir_path = fs::current_path();

      // command::workspace::gui(path);
    });
  }
  {
    auto check_ws = check->add_subcommand("workspace", "Check workspace directory");
    check_ws->add_option("path", dir_path, "If no path provided, the current directory will be used");
    check_ws->callback([&]() {
      if (dir_path.empty()) dir_path = fs::current_path();

      command::check::check_workspace(common::resolve_path(dir_path));
    });
  }
  {
    auto check_conf = check->add_subcommand("config", "Check .toml file");
    check_conf->add_option("path", dir_path, "If no path provided, the current directory will be used");
    check_conf->add_option("--full,-f", full, "Set the checker in full mode");
    check_conf->callback([&]() {
      if (dir_path.empty()) dir_path = fs::current_path();

      command::check::check_velox_config(common::resolve_path(dir_path), full);
    });
  }
  {
    auto audit = app.add_subcommand("audit", "Produce an audit report of your velox project");
    audit->add_option("path", dir_path, "If no path provided, the current directory will be used");
    audit->callback([&]() {
      if (dir_path.empty()) dir_path = fs::current_path();

      command::audit::audit_workspace(common::resolve_path(dir_path));
    });
  }
}

void Command::init_command_build()
{
  auto build = app.add_subcommand("build", "Compile your velox project");
  build->alias("b");
  build->add_option("path", file_path, "Path to the .toml project file to get the compilation context")
      ->type_name("<config path>")
      ->required()
      ->expected(1);

  build->callback([&]() {
    file_path = common::resolve_path(file_path);

    if (!fs::exists(file_path)) {
      std::cerr << "[build:Error] The file path dosen't exist." << std::endl;
      exit(1);
    }

    if (!fs::is_regular_file(file_path)) {
      std::cerr << "[build:Error] The path is not a file." << std::endl;
      exit(1);
    }

    auto        ctx = command::build::config_to_compilation_context(file_path);
    std::string cmd = common::TOOL_CTX.compiler_used + " build " + file_path + " ";
    for (auto& arg : ctx.to_args()) cmd += arg + " ";
    cmd += "\n";
    exit(std::system(cmd.c_str()));
  });
}

void Command::init_command_ffi_json()
{
  auto ffi_json = app.add_subcommand("generate-ffi-json", "Translate .json ast ffi to .vlxbind wrappers");
  ffi_json->alias("gen-ffi");
  ffi_json->add_option("source", file_path, "Source of the .json ast ffi representation")
      ->required()
      ->expected(1)
      ->type_name("<source>");
  ffi_json->add_option("dir_dest", dir_path, "Directory destination of the .vlxbind generated")
      ->required()
      ->expected(1)
      ->type_name("<dest>");

  ffi_json->callback([&]() {
    std::string cmd = common::TOOL_CTX.compiler_used + " generate-ffi-json \"" + common::resolve_path(file_path)
                      + "\" \"" + common::resolve_path(dir_path) + "\"";
    std::system(cmd.c_str());
  });
}

void Command::init_command_toolchain()
{
  app.set_version_flag("--version,-v", "Version: " + common::SOFTWARE_VERSION);

  app.add_flag_function(
      "--about,-a",
      [&](int count) {
        std::cout << common::SOFTWARE_ABOUT << std::endl;
        exit(0);
      },
      "Show detailed software info");

  init_command_package();
  init_command_build();
  init_command_ffi_json();
  init_command_compiler();
  init_command_workspace();
}
