#include "environment.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#elif __linux__
#include <unistd.h>
#include <linux/limits.h>
#endif

#include "common.hpp"
#include "toolchain_options.hpp"


namespace fs = std::filesystem;


std::vector<std::string> common::env::find_all_compilers(std::string_view dir_search) noexcept
{
  std::vector<std::string> compilers;

  if (!fs::exists(dir_search)) {
    std::cerr << "[velox:ERROR] File at \"" + std::string(dir_search) + "\" dosen't exists.";
    return {};
  }

  for (const auto& entry : fs::directory_iterator(dir_search)) {
    if (!entry.is_directory() && entry.path().filename().string().starts_with("velox-compiler"))
      compilers.emplace_back(entry.path().string());
  }

  if (compilers.empty()) {
    std::cerr << "[velox:ERROR] No velox-compiler found.";
    return compilers;
  }

  for (auto& compiler : compilers) std::cout << compiler << "\n";
  return compilers;
}


std::string common::env::find_latest_compiler(std::string_view dir_search) noexcept
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return "";


  auto latest = std::ranges::max_element(compilers, [](const auto& a, const auto& b) { return a < b; });

  if (latest != compilers.end()) {
    std::cout << *latest;
    return *latest;
  }

  std::cerr << "[velox:ERROR] No compiler found.";
  return "";
}


std::string common::env::find_compiler_version(std::string_view dir_search, std::string_view version) noexcept
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return "";


  for (auto& compiler : compilers) {
    if (fs::path(compiler).stem().string().starts_with("velox-compiler-" + std::string(version))) {
      std::cout << compiler << "\n";
      return compiler;
    }
  }

  std::cerr << "No velox-compiler found for the version " << version;
  return "";
}


std::string_view common::env::get_local_data_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) FATAL_ERROR("The appdata local environment is not defined");
  return path = std::string(appdatalocal) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Application Support/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_DATA_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/.local/share/velox";
#endif
}

std::string_view common::env::get_cache_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) FATAL_ERROR("The appdata local environment is not defined");
  return path = std::string(appdatalocal) + "\\velox\\cache";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Caches/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CACHE_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/.cache/velox";
#endif
}

// ... /velox
// - toolchain.config
std::string_view common::env::get_config_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdata = std::getenv("APPDATA"); // C:\Users\Alice\AppData\Romaning
  if (!appdata) FATAL_ERROR("The appdata environment is not defined");
  return path = std::string(appdata) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME");
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Preferences/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME");

  if (!home) FATAL_ERROR("The home environment is not defined");

  return path = std::string(home) + "/.config/velox";
#endif
}

std::string_view common::env::get_exe_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  char _path[MAX_PATH];
  GetModuleFileNameA(NULL, _path, MAX_PATH);
  return path = _path;
#else
  char    result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return path   = std::string(result, (count != -1) ? count : 0);
#endif
}

// ... /velox/lib/packages
// - package name
// -- versions
// --- package
std::string_view common::env::get_packages_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  return path = std::string(get_local_data_dir()) + "\\lib\\packages";
#else // Linux / UNIX / Apple
  return path = std::string(get_local_data_dir()) + "/lib/packages";
#endif
}

// ... /velox/lib/compiler
// - versions
// -- executable
std::string_view common::env::get_compilers_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  return path = std::string(get_local_data_dir()) + "\\compiler";
#else // Linux / UNIX / Apple
  return path = std::string(get_local_data_dir()) + "/compiler";
#endif
}

// ... /velox/lib/std
// - versions
// -- lib
std::string_view common::env::get_stdlib_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  return path = std::string(get_local_data_dir()) + "\\lib\\std";
#else // Linux / UNIX / Apple
  return path = std::string(get_local_data_dir()) + "/lib/std";
#endif
}

// ... /velox/templates
// - some template files
std::string_view common::env::get_templates_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  return path = std::string(get_local_data_dir()) + "\\templates";
#else // Linux / UNIX / Apple
  return path = std::string(get_local_data_dir()) + "/templates";
#endif
}


std::vector<std::string> common::env::get_compiler_dirs() noexcept
{
  std::vector<std::string> search_dir;
  if (!common::toolchain::OPTIONS.custom_compiler_dir.empty())
    search_dir.emplace_back(common::toolchain::OPTIONS.custom_compiler_dir);

  fs::path bin = get_exe_dir();
  search_dir.emplace_back(bin.parent_path());
#ifdef _WIN32
  search_dir.emplace_back(fs::path("C:/Program Files/Velox"));
  search_dir.emplace_back(fs::path("C:/Program Files (x86)/Velox"));
#elif __APPLE__
  search_dir.emplace_back({fs::path("/Applications/Velox")});
#else
  search_dir.emplace_back(fs::path("/usr/local/velox"));
  search_dir.emplace_back(fs::path("/opt/velox"));
#endif
  return search_dir;
}
