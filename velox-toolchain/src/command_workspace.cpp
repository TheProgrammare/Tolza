#include "command_workspace.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>

#include "cli_wrapper.hpp"
#include "toolchain.hpp"

void command::workspace::generate_velox_workspace(const std::string& project_name, const fs::path& path)
{
  fs::path project_path = path / project_name;

  if (!cli::yes_no_question("Do you want to create a new velox projet named \"" + project_name + "\" at\n  \""
                            + project_path.string() + "\"?\n ")) {
    std::cout << "[velox-toolchain] Velox workspace generation aborted..." << std::endl;
    return;
  }

  std::cout << "[velox-toolchain] generate workspace at " << project_path << std::endl;

  bool success    = true;
  auto dir_create = [&](const fs::path& _path) {
    try {
      fs::create_directory(_path);
    } catch (const fs::filesystem_error e) {
      std::cerr << e.what() << std::endl;
      return success = false;
    }
    std::cerr << "[velox-toolchain] Directory created at " << _path << std::endl;
    return true;
  };

  if (!dir_create(project_path)) success = false;
  if (!dir_create(project_path / "src")) success = false;
  if (!dir_create(project_path / "vendor")) success = false;
  if (!dir_create(project_path / "build")) success = false;
  if (!dir_create(project_path / "build" / "debug")) success = false;
  if (!dir_create(project_path / "build" / "release")) success = false;
  if (!dir_create(project_path / "config")) success = false;

  if (!write_config_file(project_path / "velox.config", project_name, false)) success = false;
  if (!write_config_file(project_path / "config" / "debug.config", project_name + "-debug", true)) success = false;

  if (!write_file(project_path / "src" / "main.velox", VELOX_MAIN_TEMPLATE)) success = false;

  if (!success)
    std::cerr << "[velox-toolchain] [error] An error has occured, workspace generation aborted..." << std::endl;
  else
    std::cout << "[velox-toolchain] Workspace successfully generated!" << std::endl;
}

bool command::workspace::write_file(const fs::path& path, const std::string& text)
{
  std::ofstream f;
  try {
    f = std::ofstream(path);
  } catch (const fs::filesystem_error e) {
    std::cout << e.what() << std::endl;
    return false;
  }

  f << text;
  std::cout << "[velox-toolchain] File created at " << path << std::endl;
  return true;
}

bool command::workspace::write_config_file(const fs::path& path, const std::string& name, bool file_debug_mode)
{
  auto fmt_template = [](std::string& templateStr, const std::initializer_list<std::string>& args) {
    size_t count = 0;
    for (auto& arg : args) { // parcours en sens inverse
      std::string placeholder = "%" + std::to_string(count++);
      size_t      pos         = 0;
      while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
        templateStr.replace(pos, placeholder.length(), arg);
        pos += arg.length();
      }
    }
  };

  std::string fmt_config = VELOX_CONFIG_TEMPLATE;
  fmt_template(fmt_config, {name, std::string(toolchain::DETECTED_ABI), std::string(toolchain::DETECTED_ARCH),
                            std::string(toolchain::DETECTED_BITS), std::string(toolchain::DETECTED_OS_NAME),
                            file_debug_mode ? "true" : "false"});

  return write_file(path, fmt_config);
}


void command::workspace::ask_new_workspace(const fs::path& ws_path)
{
  if (cli::yes_no_question("Do you want to generate a Velox project in a new folder?")) {
  retry_project_name:
    auto filename = cli::get_input("Write down your project name (file name only valid)");

    if (!cli::is_valid_filename(filename)) {
      std::cout << "Invalid project name \"" << filename << "\"." << std::endl;
      auto sanitize = cli::sanitize_filename(filename);

      if (!cli::yes_no_question("Do you want to use \"" + sanitize + "\" instead?")) {
        if (cli::yes_no_question("Do you want to retry?")) goto retry_project_name;

        std::cout << std::endl << "[velox-toolchain] Velox workspace generation aborted..." << std::endl;
        return;
      } else {
        generate_velox_workspace(sanitize, ws_path);
        return;
      }
    }

    generate_velox_workspace(filename, ws_path);
  } else {
    std::cout << std::endl << "[velox-toolchain] Velox workspace generation aborted..." << std::endl;
  }
}

bool command::workspace::new_velox_workspace()
{
  std::cout << std::endl << "[velox-toolchain] Generation of Velox workspace... at " << fs::current_path() << std::endl;

retry_project_name:
  auto filename = cli::get_input("write down your project name (file name only valid)");

  if (!cli::is_valid_filename(filename)) {
    std::cout << "Invalid project name \"" << filename << "\"." << std::endl;
    auto sanitize = cli::sanitize_filename(filename);

    if (!cli::yes_no_question("Do you want to use \"" + sanitize + "\" instead? [Y/n]")) {
      if (cli::yes_no_question("Do you want to retry? [Y/n]")) goto retry_project_name;

      std::cout << std::endl << "[velox-toolchain] Velox workspace generation aborted..." << std::endl;
      return false;
    } else {
      generate_velox_workspace(sanitize, fs::current_path());
      return true;
    }
  }

  generate_velox_workspace(filename, fs::current_path());
  return true;
}