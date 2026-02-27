/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */


#include "Command_Sanity.hpp"

#include <iostream>

#include <benhoyt/cpp/INIReader.h>

std::vector<std::string> command::sanity::check_velox_config_sanity(const fs::path& file, bool full_config)
{
  std::vector<std::string> out;
  if (!fs::exists(file)) {
    return {"The file at \"" + file.string() + "\" dosen't exists."};
  }

  INIReader reader(file);
  if (reader.ParseError() < 0) {
    return {"Cannot open the config file."};
  }

  for (auto& section : reader.Sections()) {
    if (!k_config_map.contains(section)) {
      out.push_back("Unexpected section [" + section + "]");
    }
  }

  if (full_config) {
    for (auto& [section, key] : k_config_map) {
      if (key != "") {
        if (!reader.HasValue(section, key)) {
          out.push_back("Unexpected key " + key + " at section [" + section + "]");
        }
      }
    }
  }

  for (auto& defines_key : reader.Keys("defines")) {
    if (!reader.HasValue("defines", defines_key)) {
      out.push_back("Unexpected key " + defines_key + " without value at section [defines]");
    }
  }

  for (auto& undefines_key : reader.Keys("undefines")) {
    if (reader.HasValue("undefines", undefines_key)) {
      out.push_back("Unexpected key " + undefines_key + " with value at section [undefines]");
    }
  }

  for (auto& sub_config : reader.Keys("sub_configs")) {
    if (!reader.HasValue("sub_configs", sub_config)) {
      out.push_back("Unexpected key " + sub_config + " without value at section [sub_configs]");
    }
    if (auto val = reader.GetString("sub_configs", sub_config, ""); val.empty()) {
      out.push_back("Unexpected key " + sub_config + " with a non string value at section [sub_configs]");
    }
  }

  return out;
}


bool command::sanity::check_workspace_sanity(const fs::path& ws_path)
{
  if (!fs::exists(ws_path)) {
    std::cerr << "[sanity] [error] The directory not found at " << ws_path << std::endl;
    std::cerr << "[velox] The sanity check is aborted." << ws_path << std::endl;
    return false;
  }

  bool src_found = true;
  if (!fs::exists(ws_path / "src")) {
    std::cerr << "[sanity] [error] The mandatory directory \"src/\" not found at " << ws_path / "src" << std::endl;
    src_found = false;
  }

  bool                     config_found = true;
  std::vector<std::string> config_errors;
  if (!fs::exists(ws_path / "velox.config")) {
    std::cerr << "[sanity] [error] The mandatory file \"velox.config\" dosen't exists at " << ws_path / "velox.config"
              << std::endl;
    config_found = false;
  } else {
    config_errors = check_velox_config_sanity(ws_path / "velox.config", true);
  }

  if (src_found && config_found && config_errors.empty()) {
    std::cout << "[sanity] Sanity check passed\n"
                 "Workspace: "
              << ws_path
              << "\nResult:"
                 "\n  - All required directories and configuration files are present."
                 "\n  - No inconsistencies detected."
                 "[velox] The project is healthy and ready for compilation and development."
              << std::endl;
    return true;
  } else {
    std::cerr << "[sanity] Sanity check failed" << std::endl;
    std::cout << "Workspace: " << ws_path << "\nResult:" << std::endl;
    if (!src_found) std::cout << "  - Missing the directory: \"src/\"" << std::endl;
    if (!config_found) std::cout << "  - Missing the configuration file: \"velox.config\"" << std::endl;
    if (!config_errors.empty()) {
      std::cout << "  - Invalid configuration file: \"velox.config\"\n";
      for (auto& err : config_errors) {
        std::cout << "    - " << err << "\n";
      }
      std::cout << std::endl;
    }

    return false;
  }
}