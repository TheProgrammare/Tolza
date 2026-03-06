#include "parser_command.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <unistd.h>

#include "command_audit.hpp"
#include "command_build.hpp"
#include "command_check.hpp"
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

void command::err(const std::string& msg)
{
  std::cerr << "[command] [ERROR] " << msg << std::endl;
}

void command::log(const std::string& msg, bool sub_log)
{
  if (sub_log)
    std::cout << "  " << msg << std::endl;
  else
    std::cout << "[command] " << msg << std::endl;
}


bool command::parse_commands(int argc, const char* argv[])
{
  if (argc < 2) {
    invalid_command();
    return false;
  }

  auto exec = [](bool valid_command) {
    if (!valid_command) {
      invalid_command();
      return false;
    }
    return true;
  };

  const std::string command = argv[1];

  if (command == "create") {
    return exec(parse_create(false, argc, argv));
  } else if (command == "find" | command == "f") {
    return exec(parse_find_compiler());
  } else if (command == "crw" | command == "crc") {
    return exec(parse_create(true, argc, argv));
  } else if (command == "gui" || command == "ui") {
    return exec(parse_gui(argc, argv));
  } else if (command == "build" | command == "b") {
    return exec(parse_build(argc, argv));
  } else if (command == "generate-ffi-json" | command == "gen-ffi") {
    return exec(parse_generate_ffi_json(argc, argv));
  } else if (command == "check") {
    return exec(parse_check(false, argc, argv));
  } else if (command == "chw" || command == "chc") {
    return exec(parse_check(true, argc, argv));
  } else if (command == "--help" || command == "-h") {
    return exec(help_command());
  } else if (command == "--version" || command == "-v") {
    return exec(version_command());
  } else if (command == "audit" || command == "a") {
    return exec(parse_audit(argc, argv));
  } else if (command == "package" || command == "pkg") {
    return exec(parse_package(argc, argv));
  }

  invalid_command();
  return false;
}

bool command::parse_find_compiler()
{
  log("[velox-compiler] Searching a compiler on your machine...");

  if (auto result = toolchain::find_lastest_compiler(); result) {
    log("Compiler found at : " + result.value().string() + ".", true);
    return true;
  } else {
    log("Please install a velox-compiler to use properly velox-toolchain.");
    return false;
  }
}

bool command::parse_package(int argc, const char* argv[])
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

bool command::parse_create(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    const std::string command = argv[1];
    if (command == "cw") {
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

bool command::parse_gui(int argc, const char* argv[])
{
  command::err("Opening the velox toolchain graphical user interface");
  return true;
}

bool command::parse_build(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  if (auto ctx = command::build::init_compilation_context(path); ctx) {
    if (argc > 3) command::build::parse_args_for_compilation_context(ctx.value(), 3, argc, argv);

    if (!fs::exists(ctx->compiler_file)) {
      command::err("The velox-compiler path at " + ctx->compiler_file.string() + " dosen't exists.");
      return true;
    }

    if (!command::build::start_compilation(ctx.value())) command::err("The compiler failed.");

    return true;
  }
  return false;
}

bool command::parse_generate_ffi_json(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  if (auto ctx = command::build::init_compilation_context(path); ctx) {
    if (!fs::exists(ctx->compiler_file)) {
      command::err("The velox-compiler path at " + ctx->compiler_file.string() + " dosen't exists.");
      return false;
    }

    command::build::generate_ffi_json(ctx.value().get_build_dir(), ctx->get_ffi_json_dir(), ctx->get_binding_dir());

    return true;
  }
  return false;
}

bool command::parse_check(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    const std::string command = argv[1];
    if (command == "sw") {
      const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();

      command::check::check_workspace(path);

      return true;
    } else if (command == "sc") {
      const fs::path path         = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
      const bool     in_full_mode = (argc >= 4) ? strcmp(argv[3], "--full") : false;

      command::check::check_velox_config(path, in_full_mode);

      return true;
    }
  }

  const std::string command = argv[2];

  if (command == "workspace") {
    const fs::path path = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();

    command::check::check_workspace(path);

    return true;
  } else if (command == "config") {
    const fs::path path         = (argc >= 4) ? fs::weakly_canonical(argv[3]) : fs::current_path();
    const bool     in_full_mode = (argc >= 5) ? strcmp(argv[4], "--full") : false;
    command::check::check_velox_config(path, in_full_mode);

    return true;
  }

  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  command::check::check_workspace(path);

  return true;
}

bool command::help_command()
{
  std::cout << "The Velox Toolchain version " << toolchain::VELOX_TOOLCHAIN_VERSION << std::endl;
  std::cout << HELP_LIST_COMMANDS << std::endl;
  return true;
}

bool command::version_command()
{
  std::cout << "Velox Toolchain version " + toolchain::VELOX_TOOLCHAIN_VERSION << std::endl;
  return true;
}

void command::invalid_command()
{
  err("Invalid command!");
  log("Type --help or -h to see available commands.");
}

bool command::parse_audit(int argc, const char* argv[])
{
  const fs::path path = (argc >= 3) ? fs::weakly_canonical(argv[2]) : fs::current_path();
  command::audit::audit_workspace(path);
  return true;
}