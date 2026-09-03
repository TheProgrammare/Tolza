#include "command_workspace.hpp"

#include "cli_wrapper.hpp"
#include "toolchain/toolchain.hpp"

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <common/toolchain_options.hpp>
#include <filesystem>
#include <fstream>
#include <print>
#include <string_view>

#define HLOG "[workspace] "
#define HERR "[workspace:ERROR] "

namespace fs = std::filesystem;

std::string command::workspace::generate_tolza_workspace(std::string_view project_name, std::string_view path,
                                                         bool force) noexcept
{
  fs::path project_path = fs::path(path) / project_name;

  if (!force
      && !cli::yes_no_question(std::format("Do you want to create a new tolza projet named \"{}\" at\n \"{}\"?\n ",
                                           project_name, project_path.string()))) {
    std::println(stderr, HERR "Tolza workspace generation aborted...");
    return "";
  }

  if (fs::exists(fs::path(project_path / project_name))) {
    std::println(stderr, HERR "The file already exists.");
    std::println(HLOG "Tolza workspace generation aborted...");
    return "";
  }

  std::println(HLOG "{}", project_path.string());

  bool success    = true;
  auto dir_create = [&](const fs::path& _path) {
    try {
      fs::create_directory(_path);
    } catch (const fs::filesystem_error e) {
      std::println(stderr, HERR "{}", e.what());
      return success = false;
    }
    std::println(HLOG "{}", _path.string());
    return true;
  };

  if (!dir_create(project_path)) success = false;
  if (!dir_create(project_path / "vendor")) success = false;
  if (!dir_create(project_path / "binding")) success = false;
  if (!dir_create(project_path / "binding" / "ffi_json")) success = false;
  if (!dir_create(project_path / "build")) success = false;
  if (!dir_create(project_path / "build" / "debug")) success = false;
  if (!dir_create(project_path / "build" / "release")) success = false;

  auto manifest = common::compiler::Manifest::get_current(project_name);

  if (manifest.write_manifest((project_path / "tolza.toml").string()))
    std::println(HLOG "{}", (project_path / "tolza.toml").string());
  else
    success = false;

  if (!dir_create(project_path / "profile")) success = false;

  if (common::compiler::Profile(manifest).write_profile((project_path / "profile" / "debug.toml").string(), true, true))
    std::println(HLOG "{}", (project_path / "profile" / "debug.toml").string());
  else
    success = false;

  if (common::compiler::Profile(manifest).write_profile((project_path / "profile" / "release.toml").string(), true))
    std::println(HLOG "{}", (project_path / "profile" / "release.toml").string());
  else
    success = false;

  if (!dir_create(project_path / "src")) success = false;
  if (write_file((project_path / "src" / "main.tlz").string(), toolchain::TOLZA_MAIN_TEMPLATE).empty()) success = false;
  if (!dir_create(project_path / ".vscode")) success = false;
  if (write_file((project_path / ".vscode" / "launch.json").string(), toolchain::TOLZA_LAUNCH_TEMPLATE).empty())
    success = false;
  if (write_file((project_path / ".vscode" / "tasks.json").string(), toolchain::TOLZA_TASKS_TEMPLATE).empty())
    success = false;

  if (!success)
    std::println(stderr, HERR "An error has occured, workspace generation aborted...");
  else
    std::println(HLOG "Workspace successfully generated!");

  return project_path;
}

std::string command::workspace::write_file(std::string_view path, std::string_view text, bool verbose) noexcept
{
  std::ofstream f;
  fs::path      p(path);
  try {
    f = std::ofstream(p);
  } catch (const fs::filesystem_error e) {
    std::println(stderr, HERR "{}", e.what());
    return "";
  }

  f << text;
  if (verbose) std::println(HLOG "{}", p.string());
  return p.string();
}


void command::workspace::ask_new_workspace(std::string_view ws_path, std::string_view name) noexcept
{
  std::string filename(name);
  if (cli::yes_no_question("Do you want to generate a Tolza project in a new folder?")) {
  retry_project_name:
    if (filename.empty()) filename = cli::get_input("Write down your project name (file name only valid)");

    if (!cli::is_valid_filename(filename)) {
      std::println(stderr, HERR "Invalid project name \"{}\".", filename);
      cli::sanitize_filename(filename);

      if (!cli::yes_no_question(std::format("Do you want to use \"{}\" instead?", filename))) {
        if (cli::yes_no_question("Do you want to retry?")) {
          filename.clear();
          goto retry_project_name;
        }

        std::println(HLOG "Tolza workspace generation aborted...");
        return;
      }

      (void)generate_tolza_workspace(filename, ws_path);
      return;
    }

    (void)generate_tolza_workspace(filename, ws_path);
    return;
  }

  std::println(HLOG "Tolza workspace generation aborted...");
}

void command::workspace::synchronize(std::string_view path) noexcept
{
  // generate sub barrels before the parent barrel
  for (const auto& entry : fs::directory_iterator(path)) {
    if (entry.is_directory()) common::fileutils::write_barrel(entry.path().string(), "");
  }
}


std::string command::workspace::new_tolza_workspace() noexcept
{
  std::println(HLOG "Generation of Tolza workspace... at {}", fs::current_path().string());

retry_project_name:
  auto filename = cli::get_input("write down your project name (file name only valid)");

  if (!cli::is_valid_filename(filename)) {
    std::println(HLOG "Invalid project name \"{}\".", filename);
    cli::sanitize_filename(filename);

    if (!cli::yes_no_question(std::format("Do you want to use \"{}\" instead ?", filename))) {
      if (cli::yes_no_question("Do you want to retry ?")) goto retry_project_name;

      std::println(HLOG "Tolza workspace generation aborted...");
      return "";
    }

    return generate_tolza_workspace(filename, fs::current_path().string());
  }

  return generate_tolza_workspace(filename, fs::current_path().string());
}
