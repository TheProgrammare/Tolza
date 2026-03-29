/*
 * This program include and use the marzer/tomlplusplus project
 * You can find this project at
 *
 *     https://marzer.github.io/tomlplusplus/
 *
 * Used for the ini format file reading.
 */


#include "command_check.hpp"
#include "command_workspace.hpp"
#include "common.hpp"
#include "toolchain/toolchain.hpp"

#include <expected>
#include <iostream>
#include <filesystem>

#include <marzer/toml++.hpp>

namespace fs = std::filesystem;


void command::check::err(const std::string& msg)
{
  std::cerr << "[check:ERROR] " << msg << std::endl;
}

void command::check::log(const std::string& msg, bool sub_log)
{
  if (sub_log)
    std::cerr << "  " << msg << std::endl;
  else
    std::cerr << "[check] " << msg << std::endl;
}

bool command::check::check_velox_config(const std::string& file, bool full_config, bool verbose)
{
  if (!fs::exists(file)) {
    err("The config file at " + file + " dosen't exists.");
    return false;
  }

  std::string cache = common::get_cache_dir();
  try {
    fs::create_directories(cache);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  std::string cache_config = fs::path(cache) / "velox.toml.template";
  workspace::write_config_file(cache, "velox.toml.template", false);
  if (cache_config.empty()) return false;

  toml::table eg_tbl;
  try {
    eg_tbl = toml::parse_file(cache_config);
  } catch (const toml::parse_error& e) {
    std::cerr << e.what() << ", at: " << e.source().begin.line << ":" << e.source().begin.column << std::endl;
    return false;
  }
  toml::table tbl;
  try {
    tbl = toml::parse_file(file);
  } catch (const toml::parse_error& e) {
    err(e.what());
    return false;
  }
  for (auto& [section, fields] : tbl) {
    if (!eg_tbl.contains(section)) {
      err("Unexpected section [" + std::string(section.str()) + "]");
      return false;
    }
  }
  return true;
}


bool command::check::check_workspace(const std::string& ws_path, bool verbose)
{
  log("Checking workspace check...");

  if (!fs::exists(ws_path)) {
    err("The directory at " + ws_path + " dosen't exists.");
    return false;
  }

  bool src_found = true;
  if (!fs::exists(fs::path(ws_path) / "src")) {
    err("The mandatory file \"src/\" at " + ws_path + "/src dosen't exists.");
    src_found = false;
  }

  bool config_found   = true;
  bool config_healthy = true;
  if (!fs::exists(fs::path(ws_path) / "velox.toml")) {
    err("The mandatory \"velox.toml\" at " + ws_path + "/velox.toml dosen't exists.");
    config_found = false;
  } else {
    if (!check_velox_config(fs::path(ws_path) / "velox.toml", true, verbose)) {
      err("The config at " + ws_path + "/velox.toml is invalid.");
      config_healthy = false;
    }
  }

  if (src_found && config_found && config_healthy) {
    log("The project is healthy and ready for compilation and development.");
    return true;
  }

  return false;
}