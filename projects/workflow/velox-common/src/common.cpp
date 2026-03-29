#include "common.hpp"

#include "toolchain_context.hpp"
#include "compiler_context.hpp"

#include <stdexcept>
#include <filesystem>

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

std::string common::get_local_data_dir()
{
#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) throw std::runtime_error("%APPDATALOCAL% undefined");
  return std::string(appdatalocal) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/Library/Application Support/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_DATA_HOME");
  if (xdg) return std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/.local/share/velox";
#endif
}

std::string common::get_cache_dir()
{
#ifdef _WIN32
  const char* appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (!appdatalocal) throw std::runtime_error("%APPDATALOCAL% undefined");
  return std::string(appdatalocal) + "\\velox\\cache";
#elif __APPLE__
  const char* home = std::getenv("HOME"); // /Users/Alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/Library/Caches/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CACHE_HOME");
  if (xdg) return std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME"); // /home/alice
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/.cache/velox";
#endif
}

// ... /velox
// - toolchain.config
std::string common::get_config_dir()
{
#ifdef _WIN32
  const char* appdata = std::getenv("APPDATA"); // C:\Users\Alice\AppData\Romaning
  if (!appdata) throw std::runtime_error("%APPDATA% undefined");
  return std::string(appdata) + "\\velox";
#elif __APPLE__
  const char* home = std::getenv("HOME");
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/Library/Preferences/velox";
#else // Linux / UNIX
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  if (xdg) return std::string(xdg) + "/velox";
  const char* home = std::getenv("HOME");
  if (!home) throw std::runtime_error("$HOME undefined");
  return std::string(home) + "/.config/velox";
#endif
}

std::string common::get_exe_dir()
{
#ifdef _WIN32
  char path[MAX_PATH];
  GetModuleFileNameA(NULL, path, MAX_PATH);
  return path;
#else
  char    result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  return std::string(result, (count != -1) ? count : 0);
#endif
}

// ... /velox/lib/packages
// - package name
// -- versions
// --- package
std::string common::get_packages_dir()
{
#ifdef _WIN32
  return get_local_data_dir() + "\\lib\\packages";
#else // Linux / UNIX / Apple
  return get_local_data_dir() + "/lib/packages";
#endif
}

// ... /velox/lib/compiler
// - versions
// -- executable
std::string common::get_compilers_dir()
{
#ifdef _WIN32
  return get_local_data_dir() + "\\compiler";
#else // Linux / UNIX / Apple
  return get_local_data_dir() + "/compiler";
#endif
}

// ... /velox/lib/std
// - versions
// -- lib
std::string common::get_stdlib_dir()
{
#ifdef _WIN32
  return get_local_data_dir() + "\\lib\\std";
#else // Linux / UNIX / Apple
  return get_local_data_dir() + "/lib/std";
#endif
}

// ... /velox/templates
// - some template files
std::string common::get_templates_dir()
{
#ifdef _WIN32
  return get_local_data_dir() + "\\templates";
#else // Linux / UNIX / Apple
  return get_local_data_dir() + "/templates";
#endif
}


std::vector<std::string> common::get_compiler_dirs()
{
  std::vector<std::string> search_dir;
  if (!common::TOOL_CTX.custom_compiler_dir.empty()) search_dir.push_back(common::TOOL_CTX.custom_compiler_dir);

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


std::string common::resolve_path(const std::string& s, const std::string& relative)
{
  if (s.empty()) return s;

  std::string path_str = s;

  // Expand tilde
  if (path_str[0] == '~') {
    const char* home =
#if __unix__
        std::getenv("HOME");
    if (!home) throw std::runtime_error("HOME not set");
#elif _WIN32
        std::getenv("USERPROFILE");
    if (!home) throw std::runtime_error("USERPROFILE not set");
#endif
    path_str = std::string(home) + path_str.substr(1);
  }

  fs::path p(path_str);

  // Résoudre par rapport à relative si nécessaire
  fs::path abs_path = p.is_absolute() ? p : fs::path(relative) / p;

  // Canonicalize si possible, sinon normalisation lexicale
  try {
    return fs::canonical(abs_path).string();
  } catch (const fs::filesystem_error&) {
    return abs_path.lexically_normal().string();
  }
}

void common::fmt_template(std::string& template_str, const std::initializer_list<std::string>& args)
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

void common::fmt_template(std::string& template_str, const std::map<std::string, std::string>& args)
{
  for (auto& [key, val] : args) {
    std::string placeholder = "%" + key;
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), val);
      pos += key.length();
    }
  }
}
