/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */


#include "command_check.hpp"

#include <expected>
#include <iostream>
#include <filesystem>

#include <benhoyt/cpp/INIReader.h>

namespace fs = std::filesystem;


void command::check::err(const std::string& msg)
{
  std::cerr << "[check] [ERROR] " << msg << std::endl;
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

  INIReader reader(file);
  if (reader.ParseError() < 0) {
    err("Cannot open the config file at " + file + ".");
    return false;
  }

  bool is_healthy = true;
  for (auto& section : reader.Sections()) {
    if (!k_config_map.contains(section)) {
      err("Unexpected section [" + section + "].");
      is_healthy = false;
    }
  }

  if (full_config) {
    for (auto& [section, key] : k_config_map) {
      if (key != "") {
        if (!reader.HasValue(section, key)) {
          err("Unexpected key [" + key + "] at section [" + section + "].");
          is_healthy = false;
        }
      }
    }
  }

  for (auto& defines_key : reader.Keys("defines")) {
    if (!reader.HasValue("defines", defines_key)) {
      err("Unexpected key [" + defines_key + "] at section [defines]");
      is_healthy = false;
    }
  }

  for (auto& undefines_key : reader.Keys("undefines")) {
    if (reader.HasValue("undefines", undefines_key)) {
      err("Unexpected key [" + undefines_key + "] at section [undefines] with value.");
      is_healthy = false;
    }
  }

  for (auto& sub_config : reader.Keys("sub_configs")) {
    if (!reader.HasValue("sub_configs", sub_config)) {
      err("Unexpected key [" + sub_config + "] at section [sub_configs] without value.");
      is_healthy = false;
    }
    if (auto val = reader.GetString("sub_configs", sub_config, ""); val.empty()) {
      err("Unexpected key [" + sub_config + "] at section [sub_configs] with a non string value.");
      is_healthy = false;
    }
  }

  return is_healthy;
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
  if (!fs::exists(fs::path(ws_path) / "velox.config")) {
    err("The mandatory \"velox.config\" at " + ws_path + "/velox.config dosen't exists.");
    config_found = false;
  } else {
    if (auto result = check_velox_config(fs::path(ws_path) / "velox.config", true, verbose); result) {
      config_healthy = true;
    } else {
      err("The .config at " + ws_path + "/velox.config is invalid.");
    }
  }

  if (src_found && config_found && config_healthy) {
    log("The project is healthy and ready for compilation and development.");
    return true;
  }

  return false;
}