#include "common.hpp"

#include "toolchain_options.hpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <filesystem>
#include <string_view>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

common::FastRNG common::RAND = common::FastRNG(std::random_device{}());


std::string_view common::get_local_data_dir()
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) throw std::runtime_error("%APPDATALOCAL% undefined");
  return path = std::string(appdatalocal) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/Library/Application Support/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_DATA_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/.local/share/velox";
#endif
}

std::string_view common::get_cache_dir()
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) throw std::runtime_error("%APPDATALOCAL% undefined");
  return path = std::string(appdatalocal) + "\\velox\\cache";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/Library/Caches/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CACHE_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/.cache/velox";
#endif
}

// ... /velox
// - toolchain.config
std::string_view common::get_config_dir()
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  const char* appdata = std::getenv("APPDATA"); // C:\Users\Alice\AppData\Romaning
  if (!appdata) throw std::runtime_error("%APPDATA% undefined");
  return path = std::string(appdata) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME");
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/Library/Preferences/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg) return path = std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME");
  if (!home) throw std::runtime_error("$HOME undefined");
  return path = std::string(home) + "/.config/velox";
#endif
}

std::string_view common::get_exe_dir()
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
std::string_view common::get_packages_dir()
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
std::string_view common::get_compilers_dir()
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
std::string_view common::get_stdlib_dir()
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
std::string_view common::get_templates_dir()
{
  static std::string path;
  if (!path.empty()) return path;

#ifdef _WIN32
  return path = std::string(get_local_data_dir()) + "\\templates";
#else // Linux / UNIX / Apple
  return path = std::string(get_local_data_dir()) + "/templates";
#endif
}


std::vector<std::string> common::get_compiler_dirs()
{
  std::vector<std::string> search_dir;
  if (!common::TOOLCHAIN_OPTIONS.custom_compiler_dir.empty())
    search_dir.push_back(common::TOOLCHAIN_OPTIONS.custom_compiler_dir);

  fs::path bin = common::get_exe_dir();
  search_dir.push_back(bin.parent_path());
#ifdef _WIN32
  search_dir.push_back(fs::path("C:/Program Files/Velox"));
  search_dir.push_back(fs::path("C:/Program Files (x86)/Velox"));
#elif __APPLE__
  search_dir.push_back({fs::path("/Applications/Velox")});
#else
  search_dir.push_back(fs::path("/usr/local/velox"));
  search_dir.push_back(fs::path("/opt/velox"));
#endif
  return search_dir;
}


std::string common::resolve_path(std::string_view s, std::string_view relative)
{
  if (s.empty()) return std::string(s);

  std::string path_str(s);

  // Expand tilde
  if (s[0] == '~') {
    const char* home =
#if __unix__
        std::getenv("HOME");
    if (!home) throw std::runtime_error("HOME not set");
#elif _WIN32
        std::getenv("USERPROFILE");
    if (!home) throw std::runtime_error("USERPROFILE not set");
#endif
    path_str = std::string(home) + std::string(s.substr(1));
  }

  fs::path p(path_str);

  // Résoudre par rapport à relative si nécessaire
  fs::path abs_path = p.is_absolute() ? p : fs::path(relative) / p;

  // Canonicalize si possible, sinon normalisation lexicale
  try {
    return fs::canonical(abs_path).string();
  } catch (...) {
    return abs_path.lexically_normal().string();
  }
}

bool common::filesystem::is_velox_extension(std::string_view extension)
{
  if (extension.empty()) return false;
  if (extension[0] == '.') return common::filesystem::velox_extensions.contains(extension.substr(1));

  return common::filesystem::velox_extensions.contains(extension);
}

bool common::filesystem::is_velox_file(std::string_view file_path)
{
  fs::path f(file_path);
  return is_velox_extension(f.extension().string());
}

std::set<std::string> common::filesystem::find_velox_files(std::string_view target_dir, bool is_recursive)
{
  std::set<std::string> out;
  fs::path              dir(target_dir);

  try {
    if (is_recursive) {
      for (auto& entry : fs::recursive_directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && is_velox_extension(entry.path().extension().string()))
          out.insert(entry.path().string());
      }
    } else {
      for (auto& entry : fs::directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && is_velox_extension(entry.path().extension().string()))
          out.insert(entry.path().string());
      }
    }
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return {};
  }

  return out;
}

void common::fmt_template(std::string& template_str, const std::initializer_list<std::string_view>& args)
{
  size_t count = 0;
  for (auto& arg : args) {
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}

void common::fmt_template(std::string& template_str, const std::map<std::string_view, std::string_view>& args)
{
  for (auto& [key, val] : args) {
    std::string placeholder = "%" + std::string(key);
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), val);
      pos += key.length();
    }
  }
}
