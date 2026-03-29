#include "command_compiler.hpp"

#include <filesystem>
#include <vector>
#include <iostream>

#include "common.hpp"
#include "toolchain_context.hpp"

namespace fs = std::filesystem;

void command::compiler::err(const std::string& msg)
{
  std::cerr << "[compiler:ERROR] " << msg << std::endl;
}

void command::compiler::log(const std::string& msg, bool sub_log)
{
  if (sub_log)
    std::cerr << "  " << msg << std::endl;
  else
    std::cerr << "[compiler] " << msg << std::endl;
}


std::string command::compiler::find_compiler_version(const std::string& dir_search, const std::string& version)
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return "";


  for (auto& compiler : compilers) {
    if (fs::path(compiler).stem().string().rfind("velox-compiler-" + version, 0) == 0) {
      std::cout << compiler << std::endl;
      return compiler;
    }
  }

  err("No velox-compiler found for the version " + version);
  return "";
}

void command::compiler::apply_compiler(const std::string& file)
{
  if (!fs::exists(file)) {
    err("The file at \"" + file + "\" dosen't exists.");
    return;
  }

  common::TOOL_CTX.compiler_used = file;
  common::TOOL_CTX.apply_context();
}

void command::compiler::cogito_compiler(const std::string& file)
{
  if (!fs::exists(file)) {
    err("The file at \"" + file + "\" dosen't exists.");
    log("Please, set a valid path in config at \"" + common::get_config_dir() + "\"");
    return;
  }

  std::string cmd = file + " velox-toolchain cogito";
  std::system(cmd.c_str());
}
