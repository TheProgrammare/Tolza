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

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/toolchain_options.hpp>
#include <common/utils.hpp>
#include <cstdlib>
#include <expected>
#include <marzer/toml++.hpp>
#include <print>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unistd.h>

#define HLOG "[tolza] "
#define HERR "[tolza:ERROR] "


std::string remove_quotes(std::string_view s)
{
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
    return std::string(s.substr(1, s.size() - 2));
  }
  return "";
}


bool command::build::generate_ffi_json(std::string_view from, std::string_view to)
{
  const auto cmd = std::format(R"(ffi --json "{}" "{}")", from, to);

  return toolchain::exec_compiler_cmd(cmd) > 0;
}

bool command::build::generate_ffi_c(std::string_view from, std::string_view to)
{
  const auto cmd = std::format(R"(ffi --c "{}" "{}")", from, to);

  return toolchain::exec_compiler_cmd(cmd) > 0;
}

bool command::build::start_compilation(int argc, const char* argv[])
{
  std::string cmd = "build ";
  for (size_t i = 0; i < argc; i++) std::format_to(std::back_inserter(cmd), "{} ", argv[i]);

  std::println("Compiler command launched: \n  {} {}", common::toolchain::OPTIONS.compiler_used, cmd);

  return toolchain::exec_compiler_cmd(cmd) > 0;
}