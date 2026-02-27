#include "CommandParser.hpp"
#include "Compilation.hpp"
#include "Globals.hpp"
#include "Pipeline/Pipeline.hpp"

#include <filesystem>
#include <iostream>
#include <optional>

bool parse_commands(int argc, const char* argv[])
{
  if (argc < 2) {
    std::cerr << "[velox] Invalid command!" << std::endl;
    std::cout << "[velox] Type --help or -h to see available commands." << std::endl;
    return false;
  }

  std::string command = argv[1];

  if (command == "create") {
    if (argc < 3) {
      new_velox_workspace();
      return true;
    } else {
      std::string name         = argv[2];
      fs::path    path         = (argc >= 4) ? fs::absolute(argv[3]) : fs::current_path();
      fs::path    project_path = path / name;
      generate_velox_workspace(name, path);
      return true;
    }
  } else if (command == "gui") {
    std::cout << "opening the velox compiler graphical user interface" << std::endl;
  } else if (command == "build") {
    std::string path = (argc >= 3) ? fs::absolute(argv[2]) : fs::current_path();
    if (auto ctx = init_compilation_context(path)) {
      parse_args_for_compilation_context(ctx.value(), argc, argv);

      COMP_CTX = ctx.value();

      return start_compilation(ctx->source_dir);
    }
  } else if (command == "--help" || command == "-h") {
    std::string help = HELP_LIST_COMMANDS;
    fmt_template(help, {VELOX_COMPILER_VERSION});
    std::cout << help << std::endl;
    return true;
  } else if (command == "--version" || command == "-v") {
    std::cout << "Velox version " << VELOX_COMPILER_VERSION << std::endl;
    return true;
  }

  std::cerr << "[velox] Invalid command!" << std::endl;
  std::cout << "[velox] Type --help or -h to see available commands." << std::endl;
  return false;
}
