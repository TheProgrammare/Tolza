#include "command_compiler.hpp"

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/toolchain_options.hpp>
#include <filesystem>
#include <print>

namespace fs = std::filesystem;

#define HLOG "[compiler] "
#define HERR "[compiler:ERROR] "

void command::compiler::apply_compiler(std::string_view file) noexcept
{
  const fs::path f(file);
  if (!fs::exists(f)) {
    std::println(stderr, HERR "The file at \"{}\" dosen't exist.", f.string());
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
    std::println(stderr, HERR "The file at \"{}\" dosen't exist.", f.string());
    std::println(HLOG "Please, set a valid path in toolchain config at \"{}\"", common::env::get_config_dir());
    return;
  }

  std::string cmd = std::format("{} tolza-toolchain cogito", file);
  std::system(cmd.c_str());
}
