#include "environment.hpp"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <print>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <vector>

#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>
#elif __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#elif __linux__
#include <linux/limits.h>
#include <unistd.h>
#endif

#include "common.hpp"
#include "toolchain_options.hpp"

namespace fs = std::filesystem;


std::vector<std::string> common::env::find_all_compilers(std::string_view dir_search) noexcept
{
  std::vector<std::string> compilers;

  if (!fs::exists(dir_search)) {
    std::println(stderr, "[tolza:ERROR] File at \"{}\" dosen't exist.", dir_search);
    return {};
  }

  for (const auto& entry : fs::directory_iterator(dir_search)) {
    if (!entry.is_directory() && entry.path().filename().string().starts_with("tolza-compiler"))
      compilers.emplace_back(entry.path().string());
  }

  if (compilers.empty()) {
    std::println(stderr, "[tolza:ERROR] No tolza-compiler found.");
    return compilers;
  }

  for (auto& compiler : compilers) std::println("{}", compiler);
  return compilers;
}


std::string common::env::find_latest_compiler(std::string_view dir_search) noexcept
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return {};


  auto latest = std::ranges::max_element(compilers, [](const auto& a, const auto& b) { return a < b; });

  if (latest != compilers.end()) {
    std::print("{}", *latest);
    return *latest;
  }

  std::println(stderr, "[tolza:ERROR] No compiler found.");
  return {};
}


std::string common::env::find_compiler_version(std::string_view dir_search, std::string_view version) noexcept
{
  std::vector<std::string> compilers = find_all_compilers(dir_search);

  if (compilers.empty()) return {};


  for (auto& compiler : compilers) {
    if (fs::path(compiler).stem().string().starts_with("tolza-compiler-" + std::string(version))) {
      std::println("{}", compiler);
      return compiler;
    }
  }

  std::println(stderr, "No tolza-compiler found for the version {}", version);
  return {};
}


std::string_view common::env::get_local_data_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) FATAL_ERROR("The appdata local environment is not defined");
  return path = std::string(appdatalocal) + "\\tolza";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Application Support/tolza";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_DATA_HOME");
  if (xdg) return path = std::string(xdg) + "/tolza";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/.local/share/tolza";
#endif
}

std::string_view common::env::get_cache_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) FATAL_ERROR("The appdata local environment is not defined");
  return path = std::string(appdatalocal) + "\\tolza\\cache";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Caches/tolza";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CACHE_HOME");
  if (xdg) return path = std::string(xdg) + "/tolza";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/.cache/tolza";
#endif
}

// ... /tolza
// - toolchain.config
std::string_view common::env::get_config_dir() noexcept
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdata = std::getenv("APPDATA"); // C:\Users\Alice\AppData\Romaning
  if (!appdata) FATAL_ERROR("The appdata environment is not defined");
  return path = std::string(appdata) + "\\tolza";
#elif __APPLE__
  const char* home = std::getenv("HOME");
  if (!home) FATAL_ERROR("The home environment is not defined");
  return path = std::string(home) + "/Library/Preferences/tolza";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg) return path = std::string(xdg) + "/tolza";
  const char* home = std::getenv("HOME");

  if (!home) FATAL_ERROR("The home environment is not defined");

  return path = std::string(home) + "/.config/tolza";
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

// ... /tolza/lib/packages
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

// ... /tolza/lib/compiler
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

// ... /tolza/lib/std
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

// ... /tolza/templates
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
  search_dir.emplace_back(fs::path("C:/Program Files/Tolza"));
  search_dir.emplace_back(fs::path("C:/Program Files (x86)/Tolza"));
#elif __APPLE__
  search_dir.emplace_back({fs::path("/Applications/Tolza")});
#else
  search_dir.emplace_back(fs::path(std::format("{}/{}", getenv("HOME"), "/.local/bin")));
  search_dir.emplace_back(fs::path(std::format("{}/{}", getenv("HOME"), "/bin")));
  search_dir.emplace_back(fs::path("/usr/local/bin"));
  search_dir.emplace_back(fs::path("/usr/bin"));
  search_dir.emplace_back(fs::path("/opt/bin"));
  search_dir.emplace_back(fs::path("/opt/tolza/"));
#endif
  return search_dir;
}
