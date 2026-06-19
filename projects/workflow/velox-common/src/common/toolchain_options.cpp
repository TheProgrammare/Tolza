#include "toolchain_options.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string_view>

#include <CLIUtils/CLI11.hpp>
#include <marzer/toml++.hpp>

#include "environment.hpp"
#include "utils.hpp"


namespace fs = std::filesystem;


#define OUT_LOG std::cout << "[velox] "
#define OUT_ERR std::cerr << "[velox:ERROR] "


common::toolchain::Options common::toolchain::Options::read_config(std::string_view path) noexcept
{
  toolchain::Options t;
  if (!fs::exists(path)) {
    OUT_ERR << "File at \"" << path << "\" dosen't exists.";
    return {};
  }
  toml::table tbl = toml::parse_file(path.data());

  t.compiler_used       = tbl.at_path("compiler.compiler_used").value_or("");
  t.custom_compiler_dir = tbl.at_path("compiler.custom_compiler_dir").value_or("");

  bool file_valid = true;
  if (t.compiler_used.empty() || !fs::exists(t.compiler_used)) {
    OUT_ERR << "No valid compiler used path \"" + t.compiler_used + "\"\n";
    file_valid = false;
  }
  if (!toolchain::OPTIONS.custom_compiler_dir.empty() && !fs::exists(t.custom_compiler_dir)) {
    OUT_ERR << "No valid custom compiler dir \"" << t.custom_compiler_dir << "\", it dosen't exist\n";
    file_valid = false;
  }

  if (!file_valid) {
    std::cout << "[velox] Please, check the toolchain.toml file at \"" << path << "\"\n";
    return {};
  }

  t.is_valid = true;
  return t;
}

void common::toolchain::Options::write_config(std::string_view path) noexcept
{
  std::string str(TOOLCHAIN_CONFIG);

  common::utils::fmt_template(str, {
                                       {"custom_compiler_dir", custom_compiler_dir},
                                       {"compiler_used",       compiler_used      }
  });

  // make sure the toolchain.toml exists
  init_toolchain_context();
  std::ofstream f(path.data());
  f.clear();
  f << str << std::flush;
}

void common::toolchain::init_toolchain_context() noexcept
{
  // write config
  static const fs::path dir  = common::env::get_config_dir();
  static const fs::path file = dir / "toolchain.toml";

  fs::create_directories(dir);

  if (!fs::exists(file)) {
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
    t.write_config(file.string());
  }

  if (!fs::exists(file)) {
    OUT_ERR << "File at \"" + file.string() + "\" dosen't exists.\n";
    return;
  }

  toolchain::OPTIONS = common::toolchain::Options::read_config(file.string());
}


void common::toolchain::Options::apply_compiler(std::string_view file) noexcept
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

void common::toolchain::Options::cogito_compiler(std::string_view file) noexcept
{
  const fs::path f(file);

  if (!fs::exists(f)) {
    OUT_ERR "The file at " << f << " dosen't exists.\n";
    OUT_LOG "Please, set a valid path in config at " << fs::path(common::env::get_config_dir()) << "\n";
    return;
  }

  std::string cmd = std::string(file) + " velox-toolchain cogito";
  std::system(cmd.c_str());
}