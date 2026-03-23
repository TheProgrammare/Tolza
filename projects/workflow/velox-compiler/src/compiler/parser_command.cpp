#include "compiler/parser_command.hpp"

#include <iostream>
#include <string>
#include <filesystem>

#include "binder/ffi-json_reader.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "pipeline/pipeline.hpp"


namespace fs = std::filesystem;

bool parse_command(int argc, const char* argv[])
{
  // dir
  // velox-compiler
  if (argc < 3) {
    invalid_command();
    return false;
  }

  const std::string command = argv[2];

  if (command == "--version" || "-v") {
    version_command();
    return true;
  } else if (command == "--help" || "-h") {
    help_command();
    return true;
  } else if (command == "build" || command == "b") {
    start_compilation(argc, argv);
    return true;
  } else if (command == "generate-ffi-json" || command == "gen-ffi") {
    generate_ffi_json_command(argc, argv);
    return true;
  }

  invalid_command();
  return false;
}


bool generate_ffi_json_command(int argc, const char* argv[])
{
  if (argc < 4) return false;
  if (argc < 5) return false;

  fs::path target = argv[3];
  fs::path dest   = argv[4];

  if (!fs::exists(target))
    std::cerr << "[gen-ffi:ERROR] The file target at " << target << " dosen't exists." << std::endl;
  if (!fs::exists(dest) || fs::is_directory(dest)) {
    std::cerr << "[gen-ffi:ERROR] The destination at " << dest << " is invalid or dosen't exists." << std::endl;
    return true;
  }

  if (fs::is_regular_file(target)) {
    ffi::JSON::read_ffi_json_file(target);
  } else if (fs::is_directory(target)) {
    for (const auto& entry : fs::recursive_directory_iterator(target)) {
      const fs::path& p = entry.path();

      if (fs::is_regular_file(p) && p.extension() == ".json") {
        ffi::JSON::read_ffi_json_file(p);
      }
    }
  } else {
    std::cerr << "[gen-ffi:ERROR] Unsopported special case at " << target << "." << std::endl;
  }

  return true;
}

bool help_command()
{
  std::cout << "Velox Compiler version " << compiler::VELOX_COMPILER_VERSION << std::endl;
  std::cout << common::HELP_LIST_BUILD << std::endl;
  return true;
}

bool version_command()
{
  std::cout << "Velox Compiler version " << compiler::VELOX_COMPILER_VERSION << std::endl;
  return true;
}

void invalid_command()
{
  std::cerr << "[velox-compiler] Invalid command!" << std::endl;
  std::cout << "[velox-compiler] Type --help or -h to see available commands." << std::endl;
}