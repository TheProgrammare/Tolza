#include "Compilation.hpp"

#include <cctype>
#include <iostream>
#include <filesystem>
#include <map>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <fstream>

#include "CLI_IO_Wrapper.hpp"
#include "Globals.hpp"

#include "benhoyt/cpp/INIReader.h"


void check_velox_config_sanity(const fs::path& file, bool full_config)
{
  if (!fs::exists(file)) {
    throw std::runtime_error("[velox] [config] [error] The file at \"" + file.string() + "\" dosen't exists.");
  }

  INIReader reader(file);
  if (reader.ParseError() < 0) {
    throw std::runtime_error("[velox] [config] [error] Cannot open the file at \"" + file.string() + "\".");
  }

  for (auto& section : reader.Sections()) {
    if (!k_config_map.contains(section))
      throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                               + "\", the section [" + section + "] is invalid.");
  }

  if (full_config) {
    for (auto& [section, key] : k_config_map) {
      if (key != "") {
        if (!reader.HasValue(section, key))
          throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                                   + "\", at section [" + section + "]\n  the key [" + key + "] is invalid.");
      }
    }
  }

  for (auto& defines_key : reader.Keys("defines")) {
    if (!reader.HasValue("defines", defines_key))
      throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                               + "\", at section [defines]\n  the key " + defines_key
                               + " must have a value.\n  Otherwise, move the key under the [undefines] section.");
  }

  for (auto& undefines_key : reader.Keys("undefines")) {
    if (reader.HasValue("undefines", undefines_key))
      throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                               + "\", at section [undefines]\n  the key " + undefines_key
                               + " mustn't have a value.\n  Otherwise, move the key under the [defines] section.");
  }

  for (auto& sub_config : reader.Keys("sub_configs")) {
    if (!reader.HasValue("sub_configs", sub_config))
      throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                               + "\", at section [defines]\n  the key " + sub_config + " must have a string value.");
    if (auto val = reader.GetString("sub_configs", sub_config, ""); val.empty()) {
      throw std::runtime_error("[velox] [config] [error] Invalid velox config at \"" + file.string()
                               + "\", at section [defines]\n  the key " + sub_config + " must have a string value.");
    }
  }
}

void generate_velox_workspace(const std::string& project_name, const fs::path& path)
{
  fs::path project_path = path / project_name;

  if (!CLI::yes_no_question("Do you want to generate a new velox projet at \"" + project_path.string()
                            + "\"? [Y/n] ")) {
    std::cout << "[velox] Velox workspace generation aborted..." << std::endl;
    return;
  }

  std::cout << "[velox] generate workspace at \"" << project_path << "\"" << std::endl;


  auto dir_create = [&](const fs::path& _path) {
    try {
      fs::create_directory(_path);
    } catch (const fs::filesystem_error e) {
      std::cerr << e.what() << std::endl;
      return;
    }
    std::cerr << "[velox] Directory created at " << _path << std::endl;
  };

  dir_create(project_path);
  dir_create(project_path / "src");
  dir_create(project_path / "thirdparty");
  dir_create(project_path / "build");
  dir_create(project_path / "build" / "debug");
  dir_create(project_path / "build" / "release");
  dir_create(project_path / "build" / "app");
  dir_create(project_path / "config");

  auto f_create = [&](const fs::path& _path) {
    std::ofstream f;
    try {
      f = std::ofstream(_path);
    } catch (const fs::filesystem_error e) {
      std::cout << e.what() << std::endl;
      return std::ofstream();
    }
    std::cout << "[velox] File created at " << _path << std::endl;
    return f;
  };

  std::string fmt_config = VELOX_CONFIG_TEMPLATE;
  fmt_template(fmt_config, {project_name, std::string(DETECTED_ABI), std::string(DETECTED_ARCH),
                            std::string(DETECTED_BITS), std::string(DETECTED_OS_NAME), "false"});
  std::string fmt_debug = VELOX_CONFIG_TEMPLATE;
  fmt_template(fmt_debug, {project_name + "-debug", std::string(DETECTED_ABI), std::string(DETECTED_ARCH),
                           std::string(DETECTED_BITS), std::string(DETECTED_OS_NAME), "true"});

  if (auto f = f_create(project_path / "velox.config")) f << fmt_config << std::endl;
  if (auto f = f_create(project_path / "config" / "debug.config")) f << fmt_debug << std::endl;
  if (auto f = f_create(project_path / "src" / "main.velox")) f << VELOX_MAIN_TEMPLATE << std::endl;
}

bool check_workspace_sanity(const fs::path& ws_path)
{
  if (!fs::exists(ws_path)) {
    std::cerr << "[velox] [error] The file velox.config not found at \"" << ws_path << "\"" << std::endl;

    if (!CLI::yes_no_question("  Do you want to generate a Velox workspace in a new folder? [Y/n] ")) {

      std::cout << std::endl << "[velox] Generation of Velox workspace..." << std::endl;

    retry_project_name:
      auto filename = CLI::get_input("  Write down your project name (file name only valid): ");

      if (!CLI::is_valid_filename(filename)) {
        std::cout << "Invalid project name \"" << filename << "\"." << std::endl;
        auto sanitize = CLI::sanitize_filename(filename);

        if (!CLI::yes_no_question("Do you want to use \"" + sanitize + "\"? [Y/n] ")) {
          if (CLI::yes_no_question("Do you want to retry? [Y/n]")) goto retry_project_name;

          return false;
        } else {
          generate_velox_workspace(sanitize, ws_path);
          return true;
        }
      }

      generate_velox_workspace(filename, ws_path);
      return true;
    } else {
      std::cout << std::endl << "[velox] Velox workspace generation aborted..." << std::endl;
      return false;
    }
  }
  return true;
}

bool new_velox_workspace()
{
  std::cout << std::endl << "[velox] Generation of Velox workspace... at " << fs::current_path() << std::endl;

retry_project_name:
  auto filename = CLI::get_input("  write down your project name (file name only valid): ");

  if (!CLI::is_valid_filename(filename)) {
    std::cout << "Invalid project name \"" << filename << "\"." << std::endl;
    auto sanitize = CLI::sanitize_filename(filename);

    if (!CLI::yes_no_question("Do you want to use \"" + sanitize + "\"? [Y/n]")) {
      if (CLI::yes_no_question("Do you want to retry? [Y/n]")) goto retry_project_name;

      std::cout << std::endl << "[velox] Velox workspace generation aborted..." << std::endl;
      return false;
    } else {
      generate_velox_workspace(sanitize, get_exe_dir());
      return true;
    }
  }

  generate_velox_workspace(filename, fs::current_path());
  return true;
}

std::optional<CompCtx> init_compilation_context(const fs::path& path)
{
  // check the workspace sanity
  std::cout << "[velox] Welcome to the Velox compiler !" << "\n  Version: " << VELOX_COMPILER_VERSION
            << "\n  Compiler launched at: \"" << path << "\"\n"
            << "\n [velox] Checking workspace sanity..." << std::endl;
  fs::path config_path = path / "velox.config";
  if (!check_workspace_sanity(path)) {
    return std::nullopt;
  }

  // check config file sanity
  std::cout << "[velox] [config] checking velox.config sanity..." << std::endl;
  try {
    check_velox_config_sanity(config_path, true);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }

  INIReader reader(config_path);
  auto      target_config = reader.GetString("target", "config", "self");
  if (target_config == "self") {
    return parse_compilation_context(config_path);
  } else {
    // velox.config set all defaults
    auto ctx = parse_compilation_context(config_path);

    auto sub_config_path = reader.GetString("sub_configs", target_config, "");
    if (sub_config_path.empty() && target_config != "self") {
      std::cerr << "[velox] [config] [error] The specified config " + target_config
                       + " is not defined in [sub_configs] section."
                << std::endl;
      return std::nullopt;
    }

    if (!fs::exists(sub_config_path)) {
      std::cerr << "[velox] [config] [error] The specified config " + target_config
                       + " defined in [sub_configs] section, the file located at \n"
                << "  " << sub_config_path << " doesn't exists." << std::endl;
      return std::nullopt;
    }

    try {
      check_velox_config_sanity(sub_config_path, false);
    } catch (const std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      return std::nullopt;
    }

    return parse_compilation_context(sub_config_path);
  }
}


CompCtx parse_compilation_context(const fs::path& config_path)
{
  CompCtx result;

  INIReader reader(config_path);

  result.target_abi    = reader.GetString("target", "abi", "LP64");
  result.target_arch   = reader.GetString("target", "arch", "");
  result.target_bits   = reader.GetInteger("target", "bits", 64);
  result.target_os     = reader.GetString("target", "os", "");
  result.target_libc   = reader.GetString("target", "libc", "");
  result.target_config = reader.GetString("target", "config", "self");

  result.profile_debug     = reader.GetBoolean("profile", "debug", false);
  result.profile_opt_level = reader.GetInteger("profile", "opt_level", 0);
  result.profile_size_opt  = reader.GetBoolean("profile", "size_opt", false);

  result.log_all          = reader.GetBoolean("logs", "all", false);
  result.log_filesystem   = reader.GetBoolean("logs", "filesystem", false);
  result.log_lexer        = reader.GetBoolean("logs", "lexer", false);
  result.log_preprocessor = reader.GetBoolean("logs", "preprocessor", false);
  result.log_parser       = reader.GetBoolean("logs", "parser", false);
  result.log_EMBinder     = reader.GetBoolean("logs", "EMBinder", false);
  result.log_exporter     = reader.GetBoolean("logs", "exporter", false);
  result.log_resolver     = reader.GetBoolean("logs", "resolver", false);
  result.log_LLVM_IR      = reader.GetBoolean("logs", "LLVM_IR", false);
  result.log_Linker       = reader.GetBoolean("logs", "Linker", false);

  for (auto& define : reader.Keys("defines")) {
    auto val               = reader.GetString("defines", define, "");
    result.defines[define] = val;
  }

  result.undefines = reader.Keys("undefines");

  auto emit_mode = reader.GetString("codegen", "emit_mode", "BIN");
  if (emit_mode == "LLVM") result.codegen_emit_mode = CompCtx::EEmitMode::LLVM;
  if (emit_mode == "OBJ") result.codegen_emit_mode = CompCtx::EEmitMode::OBJ;
  if (emit_mode == "ASM") result.codegen_emit_mode = CompCtx::EEmitMode::ASM;
  if (emit_mode == "BC") result.codegen_emit_mode = CompCtx::EEmitMode::BC;
  if (emit_mode == "BIN") result.codegen_emit_mode = CompCtx::EEmitMode::BIN;
  result.codegen_output_dir = reader.GetString("codegen", "output_dir", "./build");
  result.codegen_dest_file  = reader.GetString("codegen", "dest_dir", "./build/app");

  result.project_dir     = reader.GetString("project", "project_dir", "./");
  result.source_dir      = reader.GetString("project", "source_dir", "./src");
  result.thrid_party_dir = reader.GetString("project", "third_party_dir", "./thirdparty");

  for (auto& sub_config : reader.Keys("sub_configs")) {
    auto val                       = reader.GetString("sub_configs", sub_config, "");
    result.sub_configs[sub_config] = val;
  }

  return result;
}
