#include "globals.hpp"
#include "toolchain/compilation.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#elif __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#endif


fs::path get_home_dir()
{
  static fs::path cached;
  if (!cached.empty()) return cached;

#ifdef _WIN32
  // 1 - USERPROFILE
  if (const char* userprofile = std::getenv("USERPROFILE")) return cached = fs::path(userprofile);

  // 2 - HOME (MSYS / Git Bash / hybrid)
  if (const char* home = std::getenv("HOME")) return cached = fs::path(home);

  // 3 - official API Windows
  PWSTR path = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path))) {
    fs::path result(path);
    CoTaskMemFree(path);
    return cached = result;
  }

  // 4 - fallback legacy
  const char* homeDrive = std::getenv("HOMEDRIVE");
  const char* homePath  = std::getenv("HOMEPATH");
  if (homeDrive && homePath) return cached = fs::path(std::string(homeDrive) + homePath);

  throw std::runtime_error("Cannot determine home directory (Windows)");

#else
  // 1) HOME env var
  if (const char* home = std::getenv("HOME")) return cached = fs::path(home);

  // 2) POSIX fallback
  if (struct passwd* pwd = getpwuid(getuid())) return cached = fs::path(pwd->pw_dir);

  throw std::runtime_error("Cannot determine home directory (POSIX)");
#endif
}

fs::path get_exe_path()
{
  static fs::path dir;

  if (!dir.empty()) return dir;

#ifdef _WIN32
  wchar_t buffer[MAX_PATH];
  DWORD   len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (len == 0) throw std::runtime_error("GetModuleFileNameW failed");
  return dir = fs::path(buffer);

#elif __APPLE__
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size); // get required size
  std::string buf(size, '\0');
  if (_NSGetExecutablePath(buf.data(), &size) != 0) throw std::runtime_error("_NSGetExecutablePath failed");
  return dir = fs::canonical(buf);

#elif __linux__
  std::vector<char> buf(1024);
  ssize_t           len = readlink("/proc/self/exe", buf.data(), buf.size());
  if (len <= 0) throw std::runtime_error("readlink /proc/self/exe failed");
  return dir = fs::canonical(std::string(buf.data(), len));
#else
#error Unsupported platform
#endif
}

fs::path get_exe_dir()
{
  static fs::path dir = get_exe_path().parent_path();
  return dir;
}

fs::path get_stdlib_dir()
{
  static fs::path dir;

  if (!dir.empty()) return dir;

  if (const char* env = std::getenv("VELOX_LIB_STANDARD")) {
    return dir = fs::path(env);
  } else {
    dir        = get_exe_dir();
    return dir = fs::weakly_canonical(dir / ".." / "lib" / "velox" / "standard");
  }
}

fs::path Config::get_userlib_dir()
{
  static fs::path dir;
  if (!dir.empty()) return dir;

  if (const char* env = std::getenv("VELOX_LIB_USER")) {
    return dir = fs::path(env);
  }

#ifdef _WIN32
  if (const char* appdata = std::getenv("APPDATA")) {
    dir = fs::path(appdata) / "velox" / "libs";
    return dir;
  }
  return dir = get_home_dir() / ".velox" / "libs";
#else
  return dir = get_home_dir() / ".velox" / "libs";
#endif
}

fs::path Config::get_project_dir()
{
  return fs::path(COMP_CTX.codegen_dest_file);
}
fs::path Config::get_src_dir()
{
  return get_project_dir() / "src";
}
fs::path Config::get_build_dir()
{
  return get_project_dir() / "build";
}
fs::path Config::get_postpreprocess_dir()
{
  return get_build_dir() / "post-preprocess";
}
fs::path Config::get_binding_dir()
{
  return get_build_dir() / "EMBinds";
}
fs::path Config::get_dot_dir()
{
  return get_build_dir() / "dot";
}
fs::path Config::get_LLVM_IR_dir()
{
  return get_build_dir() / "LLVM-IR";
}
fs::path Config::get_FFI_JSON_dir()
{
  return get_build_dir() / "FFI-JSON";
}


std::string Config::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case Config::EPhase::filesystem:        return "FSYS";
  case Config::EPhase::lexer:             return "LEXE";
  case Config::EPhase::preprosessor:      return "PREP";
  case Config::EPhase::parser:            return "PARS";
  case Config::EPhase::embinder:          return "EMBI";
  case Config::EPhase::resolver_symbol:   return "SYMB";
  case Config::EPhase::resolver_type:     return "TYPE";
  case Config::EPhase::resolver_semantic: return "SEMA";
  case Config::EPhase::llvmir:            return "LLVM";
  case Config::EPhase::linker:            return "LINK";
  }
}

std::string Config::Phase_to_str(EPhase phase)
{
  switch (phase) {
  case Config::EPhase::filesystem:        return "file system";
  case Config::EPhase::lexer:             return "lexer";
  case Config::EPhase::preprosessor:      return "preprocessor";
  case Config::EPhase::parser:            return "parser";
  case Config::EPhase::embinder:          return "external module binder";
  case Config::EPhase::resolver_symbol:   return "resolver symbol";
  case Config::EPhase::resolver_type:     return "resolver type";
  case Config::EPhase::resolver_semantic: return "resolver semantic";
  case Config::EPhase::llvmir:            return "LLVM IR";
  case Config::EPhase::linker:            return "linker";
  }
}