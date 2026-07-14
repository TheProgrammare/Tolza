/*
 * This program include and use the marzer/tomlplusplus project
 * You can find this project at
 *
 *     https://marzer.github.io/tomlplusplus/
 *
 * Used for the ini format file reading.
 */

#include "command_build.hpp"
#include "toolchain/toolchain.hpp"

#include <cstdlib>
#include <expected>
#include <string>
#include <iostream>
#include <filesystem>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>

#include <marzer/toml++.hpp>

#include <common/common.hpp>
#include <common/utils.hpp>
#include <common/compiler_options.hpp>
#include <common/toolchain_options.hpp>

namespace fs = std::filesystem;

#define OUT_LOG std::cout << "[velox] "
#define OUT_ERR std::cerr << "[velox:ERROR] "


std::string remove_quotes(std::string_view s)
{
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
    return std::string(s.substr(1, s.size() - 2));
  }
  return "";
}


bool command::build::generate_ffi_json(std::string_view from, std::string_view to)
{
  std::string cmd = "ffi --json \"" + std::string(from) + "\" \"" + std::string(to) + "\"";

  return toolchain::exec_compiler_cmd(cmd) > 0;
}

bool command::build::generate_ffi_c(std::string_view from, std::string_view to)
{
  std::string cmd = "ffi --c \"" + std::string(from) + "\" \"" + std::string(to) + "\"";

  return toolchain::exec_compiler_cmd(cmd) > 0;
}

bool command::build::start_compilation(int argc, const char* argv[])
{
  std::string cmd = "build ";
  for (size_t i = 0; i < argc; i++) {
    cmd += argv[i];
    cmd += " ";
  }

  OUT_LOG "Compiler command launched: \n  " << common::toolchain::OPTIONS.compiler_used << " " << cmd;

  return toolchain::exec_compiler_cmd(cmd) > 0;
}