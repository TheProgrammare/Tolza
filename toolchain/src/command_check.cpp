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
#include <print>
#include <filesystem>

#include <marzer/toml++.hpp>

namespace fs = std::filesystem;

#define HLOG "[tolza] "
#define HERR "[tolza:ERROR] "


bool command::check::check_tolza_config(std::string_view file, bool full_config, bool verbose) noexcept
{
  const fs::path f(file);

  if (!fs::exists(f)) {
    std::println(stderr, HERR "The config file at \"{}\" dosen't exist.", f.string());
    return false;
  }

  fs::path cache(common::env::get_cache_dir());
  try {
    fs::create_directories(cache);
  } catch (const std::runtime_error& e) {
    std::println(stderr, HERR "{}", e.what());
    return false;
  }

  std::string               cache_config = fs::path(cache) / "tolza.toml.template";
  common::compiler::Options c            = common::compiler::Options::read_config(cache_config);

  (void)workspace::write_file(cache.string(), "tolza.toml.template");
  (void)c.write_config(cache.string());
  if (cache_config.empty()) return false;

  toml::table eg_tbl;
  try {
    eg_tbl = toml::parse_file(cache_config);
  } catch (const toml::parse_error& e) {
    std::println(stderr, HERR "{}, at: {}:{}", e.what(), e.source().begin.line, e.source().begin.column);
    return false;
  }
  toml::table tbl;
  try {
    tbl = toml::parse_file(f.string());
  } catch (const toml::parse_error& e) {
    std::println(stderr, HERR "{}", e.what());
    return false;
  }
  for (auto& [section, fields] : tbl) {
    if (!eg_tbl.contains(section)) {
      std::println(stderr, HERR "Unexpected section [{}]", section.str());
      return false;
    }
  }
  return true;
}


bool command::check::check_workspace(std::string_view ws_path, bool verbose) noexcept
{
  const fs::path p(ws_path);

  std::println("Checking workspace ...");

  if (!fs::exists(p)) {
    std::println(stderr, HERR "The directory at \"{}\" dosen't exist.", p.string());
    return false;
  }

  bool src_found = true;
  if (!fs::exists(fs::path(p) / "src")) {
    std::println(stderr, HERR R"(The mandatory file "src/" at "{}" /src dosen't exist.)", p.string());
    src_found = false;
  }

  bool config_found   = true;
  bool config_healthy = true;
  if (!fs::exists(fs::path(p) / "tolza.toml")) {
    std::println(stderr, HERR R"(The mandatory "tolza.toml" at "{}" /tolza.toml dosen't exist.)", p.string());
    config_found = false;
  } else {
    if (!check_tolza_config(std::string(p / "tolza.toml"), true, verbose)) {
      std::println(stderr, HERR R"(The config at "{}" /tolza.toml is invalid.)", p.string());
      config_healthy = false;
    }
  }

  if (src_found && config_found && config_healthy) {
    std::println(HLOG "The project is healthy and ready for compilation and development.");
    return true;
  }

  return false;
}