#include "toolchain_options.hpp"

#include "environment.hpp"

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <marzer/toml++.hpp>
#include <ostream>
#include <print>
#include <string>
#include <string_view>


namespace fs = std::filesystem;


#define HLOG "[tolza] "
#define HERR "[tolza:ERROR] "


common::toolchain::Options common::toolchain::Options::read_config(std::string_view path) noexcept
{
  toolchain::Options t;
  if (!fs::exists(path)) {
    std::println(stderr, HERR "File at \"{}\" dosen't exist.", path);
    return {};
  }
  toml::table tbl = toml::parse_file(path.data());

  t.compiler_used       = tbl.at_path("compiler.compiler_used").value_or("");
  t.custom_compiler_dir = tbl.at_path("compiler.custom_compiler_dir").value_or("");

  bool file_valid = true;
  if (t.compiler_used.empty() || !fs::exists(t.compiler_used)) {
    std::println(stderr, HERR "No valid compiler used path \"{}\"", t.compiler_used);
    file_valid = false;
  }
  if (!toolchain::OPTIONS.custom_compiler_dir.empty() && !fs::exists(t.custom_compiler_dir)) {
    std::println(stderr, HERR "No valid custom compiler dir \"{}\", it dosen't exist\n", t.custom_compiler_dir);
    file_valid = false;
  }

  if (!file_valid) {
    std::println(HLOG "Please, check the toolchain.toml file at \"{}\"", path);
    return {};
  }

  t.is_valid = true;
  return t;
}

void common::toolchain::Options::write_config(std::string_view path) noexcept
{
  std::string str = std::format(TOOLCHAIN_CONFIG, custom_compiler_dir, compiler_used);

  std::ofstream f(path.data());
  f.clear();
  f << str << std::flush;
}

void common::toolchain::init_toolchain_context() noexcept
{
  // write config
  static const fs::path dir = common::env::get_config_dir();
  static const fs::path f   = dir / "toolchain.toml";

  fs::create_directories(dir);

  if (!fs::exists(f)) {
    toolchain::Options t;

    std::string compiler;
    for (auto& dir : common::env::get_compiler_dirs()) {
      if (auto result = common::env::find_latest_compiler(dir); !result.empty()) {
        compiler = result;
        break;
      }
    }

    t.custom_compiler_dir = dir;
    t.compiler_used       = compiler;
    t.write_config(f.string());
  }

  if (!fs::exists(f)) {
    std::println(stderr, HERR "File at \"{}\" dosen't exist.", f.string());
    return;
  }

  toolchain::OPTIONS = common::toolchain::Options::read_config(f.string());
}


void common::toolchain::Options::apply_compiler(std::string_view file) noexcept
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

void common::toolchain::Options::cogito_compiler(std::string_view file) noexcept
{
  const fs::path f(file);

  if (!fs::exists(f)) {
    std::println(stderr, HERR "The file at \"{}\" dosen't exist.", f.string());
    std::println(HLOG, "Please, set a valid path in config at \"{}\"", common::env::get_config_dir());
    return;
  }

  std::string cmd = std::format("{} tolza-toolchain cogito", file);
  std::system(cmd.c_str());
}