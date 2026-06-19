/*
 * This program include and use the marzer/tomlplusplus project
 * You can find this project at
 *
 *     https://marzer.github.io/tomlplusplus/
 *
 * Used for the ini format file reading.
 */


#include "command_check.hpp"


#include <common/common.hpp>
#include <common/fileutils.hpp>
#include <common/compiler_options.hpp>
#include <common/utils.hpp>

#include "command_workspace.hpp"


#include <expected>
#include <iostream>
#include <filesystem>

#include <marzer/toml++.hpp>

namespace fs = std::filesystem;

#define OUT_LOG std::cout << "[check] "
#define OUT_ERR std::cerr << "[check:ERROR] "


bool command::check::check_velox_config(std::string_view file, bool full_config, bool verbose) noexcept
{
  const fs::path f(file);

  if (!fs::exists(f)) {
    OUT_ERR "The config file at " << f << " dosen't exists.";
    return false;
  }

  fs::path cache(common::env::get_cache_dir());
  try {
    fs::create_directories(cache);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  std::string               cache_config = fs::path(cache) / "velox.toml.template";
  common::compiler::Options c            = common::compiler::Options::read_config(cache_config);

  (void)workspace::write_file(cache.string(), "velox.toml.template");
  (void)c.write_config(cache.string());
  if (cache_config.empty()) return false;

  toml::table eg_tbl;
  try {
    eg_tbl = toml::parse_file(cache_config);
  } catch (const toml::parse_error& e) {
    std::cerr << e.what() << ", at: " << e.source().begin.line << ":" << e.source().begin.column << "\n";
    return false;
  }
  toml::table tbl;
  try {
    tbl = toml::parse_file(f.string());
  } catch (const toml::parse_error& e) {
    OUT_ERR << e.what();
    return false;
  }
  for (auto& [section, fields] : tbl) {
    if (!eg_tbl.contains(section)) {
      OUT_ERR "Unexpected section [" << section.str() << "]";
      return false;
    }
  }
  return true;
}


bool command::check::check_workspace(std::string_view ws_path, bool verbose) noexcept
{
  const fs::path p(ws_path);

  OUT_LOG "Checking workspace check...";

  if (!fs::exists(p)) {
    OUT_ERR "The directory at " << p << " dosen't exists.";
    return false;
  }

  bool src_found = true;
  if (!fs::exists(fs::path(p) / "src")) {
    OUT_ERR "The mandatory file \"src/\" at " << p << " /src dosen't exists.";
    src_found = false;
  }

  bool config_found   = true;
  bool config_healthy = true;
  if (!fs::exists(fs::path(p) / "velox.toml")) {
    OUT_ERR "The mandatory \"velox.toml\" at " << p << " /velox.toml dosen't exists.";
    config_found = false;
  } else {
    if (!check_velox_config(std::string(p / "velox.toml"), true, verbose)) {
      OUT_ERR "The config at " << p << " /velox.toml is invalid.";
      config_healthy = false;
    }
  }

  if (src_found && config_found && config_healthy) {
    OUT_LOG "The project is healthy and ready for compilation and development.";
    return true;
  }

  return false;
}