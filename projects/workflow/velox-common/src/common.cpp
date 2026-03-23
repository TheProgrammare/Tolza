#include "common.hpp"

#include <stdexcept>

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

std::string common::get_local_data_dir()
{
#ifdef _WIN32
  const std::string appdatalocal = std::getenv("APPDATALOCAL"); // C:\Users\Alice\AppData\Local
  if (appdatalocal.empty()) throw std::runtime_error("%APPDATALOCAL% undefined");
  return appdatalocal + "\\velox";
#elif __APPLE__
  const std::string home = std::getenv("HOME"); // /Users/Alice
  if (home.empty()) throw std::runtime_error("$HOME undefined");
  return home + "/Library/Application Support/velox";
#else // Linux / UNIX
  const std::string xdg = std::getenv("XDG_CONFIG_HOME");
  if (!xdg.empty()) return xdg + "/velox";
  const std::string home = std::getenv("HOME"); // /home/alice
  if (home.empty()) throw std::runtime_error("$HOME undefined");
  return home + "/.velox";
#endif
}

// ... /velox
// - toolchain.config
std::string common::get_config_dir()
{
#ifdef _WIN32
  const std::string appdata = std::getenv("APPDATA"); // C:\Users\Alice\AppData\Romaning
  if (appdata.empty()) throw std::runtime_error("%APPDATA% undefined");
  return appdata + "\\velox";
#elif __APPLE__
  const std::string home = std::getenv("HOME");
  if (home.empty()) throw std::runtime_error("$HOME undefined");
  return home + "/Library/Preferences/velox";
#else // Linux / UNIX
  const std::string xdg = std::getenv("XDG_CONFIG_HOME");
  if (!xdg.empty()) return xdg + "/velox";
  std::string home = std::getenv("HOME");
  if (home.empty()) throw std::runtime_error("$HOME undefined");
  return home + "/.config/velox";
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
