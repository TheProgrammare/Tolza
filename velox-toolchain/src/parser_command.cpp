#include "parser_command.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <unistd.h>

#include "command_audit.hpp"
#include "command_build.hpp"
#include "command_sanity.hpp"
#include "command_workspace.hpp"
#include "command_package.hpp"

#include "toolchain.hpp"

void fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args)
{
  size_t count = 0;
  for (auto& arg : args) { // parcours en sens inverse
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
      templateStr.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}

bool parse_commands(int argc, const char* argv[])
{
  if (argc < 2) {
    invalid_command();
    return false;
  }

  const std::string command = argv[1];

  if (command == "create") {
    parse_create(false, argc, argv);
    return true;
  } else if (command == "crw" | command == "crc") {
    parse_create(true, argc, argv);
    return true;
  } else if (command == "gui" || command == "ui") {
    parse_gui(argc, argv);
    return true;
  } else if (command == "build" | command == "b") {
    parse_build(argc, argv);
    return true;
  } else if (command == "generate-ffi-json" | command == "gen-ffi") {
    parse_generate_ffi_json(argc, argv);
    return true;
  } else if (command == "check") {
    parse_check(false, argc, argv);
    return true;
  } else if (command == "chw" || command == "chc") {
    parse_check(true, argc, argv);
    return true;
  } else if (command == "--help" || command == "-h") {
    help_command();
    return true;
  } else if (command == "--version" || command == "-v") {
    version_command();
    return true;
  } else if (command == "audit" || command == "a") {
    parse_audit(argc, argv);
    return true;
  } else if (command == "package" || command == "pkg") {
    parse_package(argc, argv);
    return true;
  }

  invalid_command();
  return false;
}

bool parse_package(int argc, const char* argv[])
{
  if (argc < 3) return false;

  const std::string& command = argv[2];

  if (command == "--help" || command == "-h") {
    std::cout << "The Velox Toolchain " << toolchain::VELOX_TOOLCHAIN_VERSION << std::endl;
    std::cout << HELP_LIST_PKG_COMMANDS << std::endl;
    return true;
  } else if (command == "update") {
    command::package::update();
    return true;
  } else if (command == "upgrade") {
    command::package::upgrade();
    return true;
  } else if (command == "clean") {
    command::package::clean();
    return true;
  } else if (command == "list") {
    if (argc < 4) {
      command::package::list(false, false);

      return true;
    }

    bool opt_installed  = false;
    bool opt_upgradable = false;

    if (strcmp(argv[3], "--installed") == 0)
      opt_installed = true;
    else if (strcmp(argv[3], "--upgradable") == 0)
      opt_upgradable = true;

    if (argc >= 5) {
      if (strcmp(argv[4], "--installed") == 0)
        opt_installed = true;
      else if (strcmp(argv[4], "--upgradable") == 0)
        opt_upgradable = true;
    }

    command::package::list(opt_installed, opt_upgradable);

    return true;
  }

  if (argc < 4) return false;

  const std::string pkg_name = argv[3];

  if (command == "install") {
    command::package::install(pkg_name);
    return true;
  } else if (command == "remove") {
    command::package::remove(pkg_name);
    return true;
  } else if (command == "info") {
    command::package::info(pkg_name);
    return true;
  } else if (command == "purge") {
    command::package::purge(pkg_name);
    return true;
  } else if (command == "check") {
    command::package::check(pkg_name);
    return true;
  }

  return false;
}

bool parse_create(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    const std::string command = argv[1];
    if (command == "cw") {
      std::cout << "test" << std::endl;
      if (argc >= 3) {
        const std::string name         = argv[2];
        const fs::path    path         = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();
        const fs::path    project_path = path / name;
        command::workspace::generate_velox_workspace(name, path);

        return true;
      }

      command::workspace::ask_new_workspace(fs::current_path());

      return true;
    } else if (command == "cc") {
      const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
      command::workspace::write_config_file(path / "new.config", "my_project_name", false);

      return true;
    }
    return false;
  }

  const std::string mode = argv[2];
  if (mode == "workspace") {
    if (argc >= 4) {
      const std::string name         = argv[3];
      const fs::path    path         = (argc >= 5) ? fs::weakly_canonical(argv[4]) : fs::current_path();
      const fs::path    project_path = path / name;
      command::workspace::generate_velox_workspace(name, path);

      return true;
    }

    command::workspace::ask_new_workspace(fs::current_path());

    return true;
  } else if (mode == "config") {
    const fs::path path = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();
    command::workspace::write_config_file(path / "new.config", "my_project_name", false);

    return true;
  }

  return false;
}

bool parse_gui(int argc, const char* argv[])
{
  std::cout << "[velox-toolchain] Opening the velox toolchain graphical user interface" << std::endl;
  return true;
}

bool parse_build(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  if (auto ctx = command::build::init_compilation_context(path)) {
    if (argc > 3) command::build::parse_args_for_compilation_context(ctx.value(), 3, argc, argv);

    if (!fs::exists(ctx->compiler_file)) {
      std::cerr << "[velox-toolchain] the compiler path at " << ctx->compiler_file << " dosen't exists." << std::endl;
      return true;
    }

    command::build::start_compilation(ctx.value());

    return true;
  }
  return false;
}

bool parse_generate_ffi_json(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  if (auto ctx = command::build::init_compilation_context(path)) {
    if (!fs::exists(ctx->compiler_file)) {
      std::cerr << "[velox-toolchain] the compiler path at " << ctx->compiler_file << " dosen't exists." << std::endl;
      return true;
    }

    command::build::generate_ffi_json(ctx.value().get_build_dir(), ctx->get_ffi_json_dir(), ctx->get_binding_dir());

    return true;
  }
  return false;
}

bool parse_check(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    const std::string command = argv[1];
    if (command == "sw") {
      const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
      command::sanity::check_workspace_sanity(path);

      return true;
    } else if (command == "sc") {
      const fs::path path         = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
      const bool     in_full_mode = (argc >= 4) ? strcmp(argv[3], "--full") : false;
      command::sanity::check_velox_config_sanity(path, in_full_mode);

      return true;
    }
  }

  const std::string command = argv[2];

  if (command == "workspace") {
    const fs::path path = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();
    command::sanity::check_workspace_sanity(path);

    return true;
  } else if (command == "config") {
    const fs::path path         = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();
    const bool     in_full_mode = (argc >= 5) ? strcmp(argv[4], "--full") : false;
    command::sanity::check_velox_config_sanity(path, in_full_mode);

    return true;
  }

  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  command::sanity::check_workspace_sanity(path);

  return true;
}

bool help_command()
{
  std::cout << "The Velox Toolchain " << toolchain::VELOX_TOOLCHAIN_VERSION << std::endl;
  std::cout << HELP_LIST_COMMANDS << std::endl;
  return true;
}

bool version_command()
{
  std::cout << "[velox-toolchain] version " << toolchain::VELOX_TOOLCHAIN_VERSION << std::endl;
  return true;
}

void invalid_command()
{
  std::cerr << "[velox-toolchain] Invalid command!" << std::endl;
  std::cout << "[velox-toolchain] Type --help or -h to see available commands." << std::endl;
}

bool parse_audit(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  command::audit::audit_workspace(path);
  return true;
}