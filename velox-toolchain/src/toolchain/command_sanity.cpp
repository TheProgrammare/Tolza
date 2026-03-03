/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */


#include "command_sanity.hpp"

#include <iostream>

#include <benhoyt/cpp/INIReader.h>

bool command::sanity::check_velox_config_sanity(const fs::path& file, bool full_config, bool verbose)
{
  if (!fs::exists(file)) {
    if (verbose) std::cerr << "[sanity] [error] The file at " << file.string() << " dosen't exists." << std::endl;
    return false;
  }

  INIReader reader(file);
  if (reader.ParseError() < 0) {
    if (verbose) std::cerr << "[sanity] [error] Cannot open the config file." << std::endl;
    return false;
  }

  bool is_healthy = true;
  for (auto& section : reader.Sections()) {
    if (!k_config_map.contains(section)) {
      if (verbose) std::cerr << "  - Unexpected section [" << section << "]";
      is_healthy = false;
    }
  }

  if (full_config) {
    for (auto& [section, key] : k_config_map) {
      if (key != "") {
        if (!reader.HasValue(section, key)) {
          if (verbose) std::cerr << "  - Unexpected key " << key << " at section [" << section << "]" << std::endl;
          is_healthy = false;
        }
      }
    }
  }

  for (auto& defines_key : reader.Keys("defines")) {
    if (!reader.HasValue("defines", defines_key)) {
      if (verbose)
        std::cerr << "  - Unexpected key " << defines_key << " without value at section [defines]" << std::endl;
      is_healthy = false;
    }
  }

  for (auto& undefines_key : reader.Keys("undefines")) {
    if (reader.HasValue("undefines", undefines_key)) {
      if (verbose)
        std::cerr << "  - Unexpected key " << undefines_key << " with value at section [undefines]" << std::endl;
      is_healthy = false;
    }
  }

  for (auto& sub_config : reader.Keys("sub_configs")) {
    if (!reader.HasValue("sub_configs", sub_config)) {
      if (verbose)
        std::cerr << "  - Unexpected key " << sub_config << " without value at section [sub_configs]" << std::endl;
      is_healthy = false;
    }
    if (auto val = reader.GetString("sub_configs", sub_config, ""); val.empty()) {
      if (verbose)
        std::cerr << "  - Unexpected key " << sub_config << " with a non string value at section [sub_configs]"
                  << std::endl;
      is_healthy = false;
    }
  }

  if (!is_healthy) {
    if (verbose) std::cerr << "[sanity] Config sanity check failed" << std::endl;
  } else {
    if (verbose) std::cout << "[sanity] Config sanity check passed" << std::endl;
  }

  return is_healthy;
}


bool command::sanity::check_workspace_sanity(const fs::path& ws_path, bool verbose)
{
  if (verbose) std::cout << "[velox] Checking workspace sanity..." << std::endl;

  if (!fs::exists(ws_path)) {
    if (verbose) std::cerr << "[sanity] [error] The directory not found at " << ws_path << std::endl;
    if (verbose) std::cerr << "[velox] The sanity check is aborted." << ws_path << std::endl;
    return false;
  }

  bool src_found = true;
  if (!fs::exists(ws_path / "src")) {
    if (verbose)
      std::cerr << "[sanity] [error] The mandatory directory \"src/\" not found at " << ws_path / "src" << std::endl;
    src_found = false;
  }

  bool config_found   = true;
  bool config_healthy = true;
  if (!fs::exists(ws_path / "velox.config")) {
    if (verbose)
      std::cerr << "[sanity] [error] The mandatory file \"velox.config\" dosen't exists at " << ws_path / "velox.config"
                << std::endl;
    config_found = false;
  } else {
    config_healthy = check_velox_config_sanity(ws_path / "velox.config", true, verbose);
  }

  if (src_found && config_found && config_healthy) {
    if (verbose)
      std::cout << "[sanity] Sanity check passed\n"
                   "Workspace: "
                << ws_path
                << "\nResult:"
                   "\n  - All required directories and configuration files are present."
                   "\n  - No inconsistencies detected."
                   "\n[velox] The project is healthy and ready for compilation and development."
                << std::endl;
    return true;
  } else {
    if (verbose) {
      std::cerr << "[sanity] Sanity check failed" << std::endl;
      std::cout << "Workspace: " << ws_path << "\nResult:" << std::endl;
      if (!src_found) std::cout << "  - Missing the directory: \"src/\"" << std::endl;
      if (!config_found) std::cout << "  - Missing the configuration file: \"velox.config\"" << std::endl;
      if (!config_healthy) std::cout << "  - Invalid configuration file: \"velox.config\"\n" << std::endl;
    }

    return false;
  }
}