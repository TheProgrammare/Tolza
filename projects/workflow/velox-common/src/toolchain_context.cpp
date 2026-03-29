#include "toolchain_context.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <CLIUtils/CLI11.hpp>
#include <marzer/toml++.hpp>

#include "common.hpp"


namespace fs = std::filesystem;


std::vector<std::string> command::compiler::find_all_compilers(const std::string& dir_search)
{
  std::vector<std::string> compilers;

  if (!fs::exists(dir_search)) {
    std::cerr << "[velox:ERROR] File at \"" + dir_search + "\" dosen't exists.";
    return {};
  }

  for (const auto& entry : fs::directory_iterator(dir_search)) {
    if (!entry.is_directory() && entry.path().filename().string().starts_with("velox-compiler"))
      compilers.emplace_back(entry.path().string());
  }

  if (compilers.empty()) {
    std::cerr << "[velox:ERROR] No velox-compiler found.";
    return compilers;
  }

  for (auto& compiler : compilers) std::cout << compiler << std::endl;
  return compilers;
}


std::string command::compiler::find_latest_compiler(const std::string& dir_search)
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return "";


  auto latest =
      std::max_element(compilers.begin(), compilers.end(), [](const auto& a, const auto& b) { return a < b; });

  if (latest != compilers.end()) {
    std::cout << *latest;
    return *latest;
  }

  std::cerr << "[velox:ERROR] No compiler found.";
  return "";
}

void common::ToolchainCtx::apply_context()
{
  std::string str = toolchain_config;

  common::fmt_template(str, {
                                {"custom_compiler_dir", custom_compiler_dir},
                                {"compiler_used",       compiler_used      }
  });

  // make sure the toolchain.toml exists
  init_toolchain_context();
  std::ofstream f(fs::path(common::get_config_dir()) / "toolchain.toml");
  f.clear();
  f << str << std::flush;
}

void common::init_toolchain_context()
{
  // write config
  static const fs::path dir  = common::get_config_dir();
  static const fs::path file = dir / "velox-toolchain.toml";
  fs::create_directories(dir);

  if (!fs::exists(file)) {
    std::ofstream f(file);

    std::string str = toolchain_config;
    std::string compiler;
    for (auto& dir : get_compiler_dirs()) {
      if (auto result = command::compiler::find_latest_compiler(dir); !result.empty()) {
        compiler = result;
        break;
      }
    }

    fmt_template(str, {
                          {"custom_compiler_dir", dir     },
                          {"compiler_used",       compiler},
    });

    f << str << std::flush;
  }

  if (!fs::exists(file)) {
    std::cerr << "[velox:ERROR] File at \"" + file.string() + "\" dosen't exists.";
    return;
  }
  toml::table tbl = toml::parse_file(file.string());

  TOOL_CTX.compiler_used       = tbl.at_path("compiler.compiler_used").value_or("");
  TOOL_CTX.custom_compiler_dir = tbl.at_path("compiler.custom_compiler_dir").value_or("");

  bool file_valid = true;
  if (TOOL_CTX.compiler_used.empty() || !fs::exists(TOOL_CTX.compiler_used)) {
    std::cerr << "[velox:ERROR] No valid compiler used path \"" + TOOL_CTX.compiler_used + "\"";
    file_valid = false;
  }
  if (!TOOL_CTX.custom_compiler_dir.empty() && !fs::exists(TOOL_CTX.custom_compiler_dir)) {
    std::cerr << "[velox:ERROR] No valid custom compiler dir \"" + TOOL_CTX.custom_compiler_dir
                     + "\", it dosen't exist";
    file_valid = false;
  }

  if (!file_valid) std::cout << "[velox] Please, check the toolchain.toml file at \"" + file.string() + "\"";
}