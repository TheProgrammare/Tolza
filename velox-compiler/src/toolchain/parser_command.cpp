#include "parser_command.hpp"

#include <cstring>
#include <iostream>

#include "command_audit.hpp"
#include "command_build.hpp"
#include "command_sanity.hpp"
#include "command_workspace.hpp"

#include "toolchain/compilation.hpp"
#include "compiler/pipeline/pipeline.hpp"
#include "globals.hpp"

bool parse_commands(int argc, const char* argv[])
{
  if (argc < 2) {
    invalid_command();
    return false;
  }

  std::string command = argv[1];

  if (command == "create") {
    if (command_create(false, argc, argv)) return true;
  } else if (command == "cw" | command == "cc") {
    if (command_create(true, argc, argv)) return true;
  } else if (command == "gui" || command == "ui") {
    if (command_gui(argc, argv)) return true;
  } else if (command == "build" | command == "b") {
    if (command_build(argc, argv)) return true;
  } else if (command == "sanity") {
    if (command_sanity(false, argc, argv)) return true;
  } else if (command == "sw" || command == "sc") {
    if (command_sanity(true, argc, argv)) return true;
  } else if (command == "--help" || command == "-h") {
    if (command_help(argc, argv)) return true;
  } else if (command == "--version" || command == "-v") {
    if (command_version(argc, argv)) return true;
  } else if (command == "audit" || command == "a") {
    if (command_audit(argc, argv)) return true;
  }

  invalid_command();
  return false;
}

bool command_create(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    std::string command = argv[1];
    if (command == "cw") {
      std::string name         = argv[2];
      fs::path    path         = (argc >= 4) ? fs::absolute(argv[3]) : fs::current_path();
      fs::path    project_path = path / name;
      command::workspace::generate_velox_workspace(name, path);

      return true;
    } else if (command == "cc") {
      fs::path path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
      command::workspace::write_config_file(path / "new.config", "my_project_name", false);
      return true;
    }
    return false;
  }

  std::string mode = argv[2];
  if (mode == "workspace") {
    if (argc >= 4) {
      std::string name         = argv[3];
      fs::path    path         = (argc >= 5) ? fs::absolute(argv[4]) : fs::current_path();
      fs::path    project_path = path / name;
      command::workspace::generate_velox_workspace(name, path);

      return true;
    }

    command::workspace::ask_new_workspace(fs::current_path());

    return true;
  } else if (mode == "config") {
    fs::path path = (argc >= 4) ? fs::absolute(argv[3]) : fs::current_path();
    command::workspace::write_config_file(path / "new.config", "my_project_name", false);

    return true;
  }

  return false;
}

bool command_gui(int argc, const char* argv[])
{
  std::cout << "[velox] Opening the velox compiler graphical user interface" << std::endl;
  return true;
}

bool command_build(int argc, const char* argv[])
{
  std::string path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
  if (auto ctx = command::build::init_compilation_context(path)) {
    if (argc > 3) command::build::parse_args_for_compilation_context(ctx.value(), 3, argc, argv);

    COMP_CTX = ctx.value();

    start_compilation(ctx->source_dir);
    return true;
  }
  return false;
}

bool command_sanity(bool short_command, int argc, const char* argv[])
{
  if (short_command) {
    std::string command = argv[1];
    if (command == "sw") {
      fs::path path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
      command::sanity::check_workspace_sanity(path);
    } else if (command == "sc") {
      fs::path path         = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
      bool     in_full_mode = (argc >= 4) ? std::string(argv[3]) == "--full" : false;
      command::sanity::check_velox_config_sanity(path, in_full_mode);
    }
  }

  std::string command = argv[2];

  if (command == "workspace") {
    fs::path path = (argc >= 4) ? fs::absolute(argv[3]) : fs::current_path();
    command::sanity::check_workspace_sanity(path);
  } else if (command == "config") {
    fs::path path         = (argc >= 4) ? fs::absolute(argv[3]) : fs::current_path();
    bool     in_full_mode = (argc >= 5) ? std::string(argv[4]) == "--full" : false;
    command::sanity::check_velox_config_sanity(path, in_full_mode);
  }

  fs::path path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
  command::sanity::check_workspace_sanity(path);
  return true;
}

bool command_help(int argc, const char* argv[])
{
  std::string help = HELP_LIST_COMMANDS;
  fmt_template(help, {VELOX_COMPILER_VERSION});
  std::cout << help << std::endl;
  return true;
}

bool command_version(int argc, const char* argv[])
{
  std::cout << "[velox] version " << VELOX_COMPILER_VERSION << std::endl;
  return true;
}

void invalid_command()
{
  std::cerr << "[velox] Invalid command!" << std::endl;
  std::cout << "[velox] Type --help or -h to see available commands." << std::endl;
}

bool command_audit(int argc, const char* argv[])
{
  fs::path path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
  command::audit::audit_workspace(path);
  return true;
}