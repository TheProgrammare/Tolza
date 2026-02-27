#include "Command_Workspace.hpp"

#include <iostream>
#include <fstream>

#include "Globals.hpp"
#include "CLI_IO_Wrapper.hpp"
#include "Toolchain/Compilation.hpp"

void command::workspace::generate_velox_workspace(const std::string& project_name, const fs::path& path)
{
  fs::path project_path = path / project_name;

  if (!CLI::yes_no_question("Do you want to generate a new velox projet at \"" + project_path.string() + "\"?")) {
    std::cout << "[velox] Velox workspace generation aborted..." << std::endl;
    return;
  }

  std::cout << "[velox] generate workspace at " << project_path << std::endl;


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

  write_config_file(project_path / "velox.config", project_name, false);
  write_config_file(project_path / "config" / "debug.config", project_name + "-debug", true);

  write_file(project_path / "src" / "main.velox", VELOX_MAIN_TEMPLATE);
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
  std::cout << "[velox] File created at " << path << std::endl;
  return true;
}

void command::workspace::write_config_file(const fs::path& path, const std::string& name, bool file_debug_mode)
{
  std::string fmt_config = VELOX_CONFIG_TEMPLATE;
  fmt_template(fmt_config, {name, std::string(DETECTED_ABI), std::string(DETECTED_ARCH), std::string(DETECTED_BITS),
                            std::string(DETECTED_OS_NAME), file_debug_mode ? "true" : "false"});

  write_file(path, fmt_config);
}


void command::workspace::ask_new_workspace(const fs::path& ws_path)
{
  if (!CLI::yes_no_question("Do you want to generate a new Velox project in a new folder?")) {
  retry_project_name:
    auto filename = CLI::get_input("Write down your project name (file name only valid)");

    if (!CLI::is_valid_filename(filename)) {
      std::cout << "Invalid project name \"" << filename << "\"." << std::endl;
      auto sanitize = CLI::sanitize_filename(filename);

      if (!CLI::yes_no_question("Do you want to use \"" + sanitize + "\" instead?")) {
        if (CLI::yes_no_question("Do you want to retry?")) goto retry_project_name;

        return;
      } else {
        generate_velox_workspace(sanitize, ws_path);
        return;
      }
    }

    generate_velox_workspace(filename, ws_path);
  } else {
    std::cout << std::endl << "[velox] Velox workspace generation aborted..." << std::endl;
  }
}

bool command::workspace::new_velox_workspace()
{
  std::cout << std::endl << "[velox] Generation of Velox workspace... at " << fs::current_path() << std::endl;

retry_project_name:
  auto filename = CLI::get_input("write down your project name (file name only valid)");

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