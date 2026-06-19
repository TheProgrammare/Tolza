#include "command_compiler.hpp"

#include <filesystem>
#include <iostream>

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/toolchain_options.hpp>

namespace fs = std::filesystem;

#define OUT_LOG std::cout << "[compiler] "
#define OUT_ERR std::cerr << "[compiler:ERROR] "

void command::compiler::apply_compiler(std::string_view file) noexcept
{
  const fs::path f(file);
  if (!fs::exists(f)) {
    OUT_ERR "The file at " << f << " dosen't exists.";
    return;
  }

  auto conf = common::env::get_config_dir();

  fs::path path_t(conf);
  path_t /= "toolchain";
  path_t.replace_extension("toml");

  common::toolchain::OPTIONS.compiler_used = file;
  common::toolchain::OPTIONS.write_config(path_t.string());
}

void command::compiler::cogito_compiler(std::string_view file) noexcept
{
  const fs::path f(file);

  if (!fs::exists(f)) {
    OUT_ERR "The file at " << f << " dosen't exists.";
    OUT_LOG "Please, set a valid path in config at " << fs::path(common::env::get_config_dir());
    return;
  }

  std::string cmd = std::string(file) + " velox-toolchain cogito";
  std::system(cmd.c_str());
}
