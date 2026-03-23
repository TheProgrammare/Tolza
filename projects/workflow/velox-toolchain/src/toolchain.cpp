#include "toolchain.hpp"

#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <string>
#include <expected>
#include <iostream>

#include "parser_command.hpp"
#include "command_compiler.hpp"

#include <common.hpp>

#include <benhoyt/cpp/INIReader.h>

namespace fs = std::filesystem;

static const std::string toolchain_config =
    R"(
# velox-toolchain config file

# DO NOT remove or add any section nor field

[compiler]
compiler_used = "%0"

)";

int toolchain::init_toolchain()
{
  static const fs::path dir  = common::get_config_dir();
  static const fs::path file = dir / "velox-toolchain.config";
  fs::create_directories(dir);
  if (!fs::exists(file)) {
    std::fstream f(file);

    std::string str = toolchain_config;
    fmt_template(str, {command::compiler::find_lastest_compiler()});

    f << str << std::flush;
  }

  INIReader r(file);

  TOOL_CTX.compiler_used = r.Get("compiler", "compiler_used", "");

  if (TOOL_CTX.compiler_used.empty()) {
    toolchain::err("No valid compiler used");
    toolchain::log("Please, check the toolchain.config file at \"" + file.string() + "\"");
    return 1;
  }

  return 0;
}

void toolchain::ToolchainCtx::apply_context()
{
  std::string str = toolchain_config;

  fmt_template(str, {compiler_used});

  // make sure the toolchain.config exists
  init_toolchain();
  std::fstream f(fs::path(common::get_config_dir()) / "toolchain.config");
  f.clear();
  f << str << std::flush;
}


void toolchain::err(const std::string& msg)
{
  std::cerr << "[velox-toolchain:ERROR] " << msg << std::endl;
}

void toolchain::log(const std::string& msg)
{
  std::cerr << "[velox-toolchain] " << msg << std::endl;
}
