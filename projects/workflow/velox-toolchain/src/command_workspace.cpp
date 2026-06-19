#include "command_workspace.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string_view>

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/toolchain_options.hpp>
#include <common/fileutils.hpp>

#include "cli_wrapper.hpp"
#include "toolchain/toolchain.hpp"

#define OUT_LOG std::cout << "[workspace] "
#define OUT_ERR std::cerr << "[workspace:ERROR] "

namespace fs = std::filesystem;

std::string command::workspace::generate_velox_workspace(std::string_view project_name, std::string_view path,
                                                         bool force) noexcept
{
  fs::path project_path = fs::path(path) / project_name;

  if (!force
      && !cli::yes_no_question("Do you want to create a new velox projet named \"" + std::string(project_name)
                               + "\" at\n  \"" + project_path.string() + "\"?\n ")) {
    OUT_LOG "Velox workspace generation aborted...";
    return "";
  }

  if (fs::exists(fs::path(project_path / project_name))) {
    OUT_ERR "The file already exists.";
    OUT_LOG "Velox workspace generation aborted...";
    return "";
  }

  OUT_LOG << project_path;

  bool success    = true;
  auto dir_create = [&](const fs::path& _path) {
    try {
      fs::create_directory(_path);
    } catch (const fs::filesystem_error e) {
      OUT_ERR << e.what();
      return success = false;
    }
    OUT_LOG << _path;
    return true;
  };

  if (!dir_create(project_path)) success = false;
  if (!dir_create(project_path / "src")) success = false;
  if (!dir_create(project_path / "vendor")) success = false;
  if (!dir_create(project_path / "binding")) success = false;
  if (!dir_create(project_path / "binding" / "ffi_json")) success = false;
  if (!dir_create(project_path / "build")) success = false;
  if (!dir_create(project_path / "build" / "debug")) success = false;
  if (!dir_create(project_path / "build" / "release")) success = false;
  if (!dir_create(project_path / "config")) success = false;

  if (common::compiler::Options("velox").write_config((project_path / "velox").string())) success = false;
  if (common::compiler::Options("debug").write_config((project_path / "config").string())) success = false;

  if (write_file((project_path / "src" / "main.vlx").string(), toolchain::VELOX_MAIN_TEMPLATE).empty()) success = false;

  if (!success)
    OUT_ERR "An error has occured, workspace generation aborted...";
  else
    OUT_LOG "Workspace successfully generated!";

  return project_path;
}

std::string command::workspace::write_file(std::string_view path, std::string_view text, bool verbose) noexcept
{
  std::ofstream f;
  fs::path      p(path);
  try {
    f = std::ofstream(p);
  } catch (const fs::filesystem_error e) {
    OUT_ERR << e.what();
    return "";
  }

  f << text;
  if (verbose) OUT_LOG << p;
  return p.string();
}


void command::workspace::ask_new_workspace(std::string_view ws_path, std::string_view name) noexcept
{
  std::string filename(name);
  if (cli::yes_no_question("Do you want to generate a Velox project in a new folder?")) {
  retry_project_name:
    if (filename.empty()) filename = cli::get_input("Write down your project name (file name only valid)");

    if (!cli::is_valid_filename(filename)) {
      OUT_LOG "Invalid project name \"" + filename + "\".";
      cli::sanitize_filename(filename);

      if (!cli::yes_no_question("Do you want to use \"" + filename + "\" instead?")) {
        if (cli::yes_no_question("Do you want to retry?")) {
          filename.clear();
          goto retry_project_name;
        }

        OUT_LOG "Velox workspace generation aborted...";
        return;
      }

      (void)generate_velox_workspace(filename, ws_path);
      return;
    }

    (void)generate_velox_workspace(filename, ws_path);
    return;
  }

  OUT_LOG "Velox workspace generation aborted...";
}

void command::workspace::synchronize(std::string_view path) noexcept
{
  // generate sub barrels before the parent barrel
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_directory()) common::fileutils::write_barrel(entry.path().string(), "");
  }
}


std::string command::workspace::new_velox_workspace() noexcept
{
  OUT_LOG "Generation of Velox workspace... at " << fs::current_path();

retry_project_name:
  auto filename = cli::get_input("write down your project name (file name only valid)");

  if (!cli::is_valid_filename(filename)) {
    OUT_LOG "Invalid project name \"" + filename + "\".";
    cli::sanitize_filename(filename);

    if (!cli::yes_no_question("Do you want to use \"" + filename + "\" instead? [Y/n]")) {
      if (cli::yes_no_question("Do you want to retry? [Y/n]")) goto retry_project_name;

      OUT_LOG "Velox workspace generation aborted...";
      return "";
    }

    return generate_velox_workspace(filename, fs::current_path().string());
  }

  return generate_velox_workspace(filename, fs::current_path().string());
}
