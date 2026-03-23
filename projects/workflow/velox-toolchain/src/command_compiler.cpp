#include "command_compiler.hpp"

#include <filesystem>
#include <regex>
#include <vector>
#include <iostream>

#include "toolchain.hpp"

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

std::vector<std::string> command::compiler::find_all_compilers()
{
  std::vector<fs::path> baseDirs;
#ifdef _WIN32
  baseDirs = {"C:/Program Files/Velox", "C:/Program Files (x86)/Velox"};
#elif __APPLE__
  baseDirs = {"/Applications/Velox"};
#else
  baseDirs = {"/usr/local/velox", "/opt/velox"};
#endif

  std::vector<std::string> compilers;

  for (const auto& base : baseDirs) {
    if (!fs::exists(base)) continue;

    for (const auto& entry : fs::directory_iterator(base)) {
      if (entry.is_directory()) compilers.emplace_back(entry.path().string());
    }
  }

  if (compilers.empty()) {
    err("No velox-compiler found.");
    return compilers;
  }

  for (auto& compiler : compilers) std::cout << compiler << std::endl;
  return compilers;
}

std::string command::compiler::find_compiler_version(const std::string& version)
{
  std::vector<std::string> compilers = find_all_compilers();

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

std::string command::compiler::find_lastest_compiler()
{
  std::vector<std::string> compilers = find_all_compilers();

  if (compilers.empty()) return "";


  auto latest =
      std::max_element(compilers.begin(), compilers.end(), [](const auto& a, const auto& b) { return a < b; });

  if (latest != compilers.end()) {
    std::cout << *latest;
    return *latest;
  }

  err("No compiler found.");
  return "";
}

void command::compiler::apply_compiler(const std::string& file)
{
  if (!fs::exists(file)) {
    err("The file at \"" + file + "\" dosen't exists.");
    return;
  }

  toolchain::TOOL_CTX.compiler_used = file;
  toolchain::TOOL_CTX.apply_context();
}