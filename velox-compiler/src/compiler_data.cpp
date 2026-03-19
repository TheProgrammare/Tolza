#include "compiler_data.hpp"

#include <filesystem>
#include <iostream>

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

#include "compiler.hpp"

compiler::CompCtx compiler::COMP_CTX;


namespace fs = std::filesystem;

bool compiler::in_binding_compilation       = false;
bool compiler::command_from_velox_toolchain = false;

void compiler::parse_args_for_compilation_context(CompCtx& ctx, int argc, const char* argv[])
{
  for (int i = 2; i < argc; ++i) {
    const std::string& arg = argv[i];

    // e.g. --debug
    auto bool_arg = [&](bool& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        input = false;
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name, 0) == 0) {
        valid_arg = true;
        ctx.COMPILATION_ARGS.emplace(arg_name, "true");
        input = true;
      } else if (!arg_name.empty() && arg.rfind("--!" + arg_name, 0) == 0) {
        valid_arg = true;
        ctx.COMPILATION_ARGS.emplace(arg_name, "false");
        input = false;
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg = true;
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, "true");
        input = true;
      } else if (!alt_arg_name.empty() && arg.rfind("-!" + alt_arg_name, 0) == 0) {
        valid_arg = true;
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, "false");
        input = false;
      }

      return valid_arg;
    };

    // e.g. --os="linux"
    auto str_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          valid_arg         = true;
          std::string value = arg.substr(pos + 1);
          ctx.COMPILATION_ARGS.emplace(arg_name, value);
          input = value;
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg         = true;
        std::string value = arg.substr(1 + alt_arg_name.size()); // tout après "-o"
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, value);
        input = value;
      }

      return valid_arg;
    };

    // e.g. --dest="/mnt/data/my_project"
    auto path_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          valid_arg         = true;
          std::string value = arg.substr(pos + 1);
          ctx.COMPILATION_ARGS.emplace(arg_name, value);
          input = fs::weakly_canonical(value);
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg         = true;
        std::string value = arg.substr(1 + alt_arg_name.size());
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, value);
        input = fs::weakly_canonical(value);
      }

      return valid_arg;
    };

    // e.g. --opt-level=0
    auto size_arg = [&](size_t& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          std::string value = arg.substr(pos + 1);
          ctx.COMPILATION_ARGS.emplace(arg_name, value);
          valid_arg = true;
          input     = std::stoul(value);
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        std::string value = arg.substr(1 + alt_arg_name.size());
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, value);
        valid_arg = true;
        input     = std::stoul(value);
      }

      return valid_arg;
    };

    // target
    if (str_arg(ctx.target_abi, "abi")) continue;
    if (str_arg(ctx.target_arch, "arch")) continue;
    if (size_arg(ctx.target_bits, "bits")) continue;
    if (str_arg(ctx.target_os, "os")) continue;
    if (str_arg(ctx.target_libc, "libc")) continue;
    if (str_arg(ctx.target_config, "config")) continue;

    // profile
    if (bool_arg(ctx.profile_debug, "debug", "d")) continue;
    if (bool_arg(ctx.profile_debug, "release", "r")) {
      ctx.profile_debug = false;
      continue;
    }
    if (size_arg(ctx.profile_opt_level, "opt-level")) continue;
    if (bool_arg(ctx.profile_size_opt, "size-opt")) continue;

    // logs
    if (bool_arg(ctx.log_all, "log-all", "lall")) continue;
    if (bool_arg(ctx.log_filesystem, "log-filesystem", "lfs")) continue;
    if (bool_arg(ctx.log_lexer, "log-lexer", "llex")) continue;
    if (bool_arg(ctx.log_preprocessor, "log-pre", "lpre")) continue;
    if (bool_arg(ctx.log_parser, "log-parser", "lpar")) continue;
    if (bool_arg(ctx.log_binder, "log-binder", "lemb")) continue;
    if (bool_arg(ctx.log_exporter, "log-exporter", "lexp")) continue;
    if (bool_arg(ctx.log_resolver, "log-resolver", "lres")) continue;
    if (bool_arg(ctx.log_LLVM_IR, "log-llvm", "lllvm")) continue;
    if (bool_arg(ctx.log_linker, "log-linker", "llink")) continue;

    // warnings
    if (bool_arg(ctx.warn_all, "warn-all", "wall")) continue;
    if (bool_arg(ctx.warn_extra, "warn-extra", "wextra")) continue;
    if (bool_arg(ctx.warn_pedantic, "warn-pedantic", "wpedan")) continue;
    if (size_arg(ctx.warn_level, "warn-level")) continue;
    if (bool_arg(ctx.warn_unused, "warn-unused", "wun")) continue;
    if (bool_arg(ctx.warn_dead_code, "warn-dead-code", "wdc")) continue;
    if (bool_arg(ctx.warn_as_error, "warn-as-error", "wae")) continue;

    // printer
    if (bool_arg(ctx.print_ast, "print-ast")) continue;

    // define macro
    if (arg.rfind("-D", 0) == 0) {
      auto def    = arg.substr(2);
      auto eq_pos = def.find('=');

      std::string name, value;

      if (eq_pos != std::string::npos) {
        name  = def.substr(0, eq_pos);
        value = def.substr(eq_pos + 1);
      } else {
        name  = def;
        value = "1"; // implicit value
      }
      ctx.COMPILATION_ARGS.emplace(name, value);
      ctx.defines.emplace(name, value);

      continue;
    }

    // undefine macro
    if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);

      ctx.COMPILATION_ARGS.emplace(name, ""); // no value for -UName
      ctx.undefines.push_back(name);
      continue;
    }

    // codegen
    if (bool_arg(ctx.emit_bin, "emit-bin")) continue;
    if (bool_arg(ctx.emit_llvm, "emit-llvm")) continue;
    if (bool_arg(ctx.emit_obj, "emit-obj")) continue;
    if (bool_arg(ctx.emit_asm, "emit-asm")) continue;
    if (bool_arg(ctx.emit_bc, "emit-bc")) continue;
    if (bool_arg(ctx.emit_static_lib, "emit-static-lib")) continue;
    if (bool_arg(ctx.emit_dynamic_lib, "emit-dynamic-lib")) continue;

    if (path_arg(ctx.codegen_build_dir, "build")) continue;

    // project
    if (path_arg(ctx.project_dir, "project")) continue;
    if (path_arg(ctx.source_dir, "src")) continue;
    if (path_arg(ctx.vendor_dir, "vendor")) continue;
    if (path_arg(ctx.ffi_json_dir, "ffi-json")) continue;
    if (path_arg(ctx.binding_dir, "binding")) continue;

    if (arg == "--from-velox-toolchain" || arg == "-fvt") {
      command_from_velox_toolchain = true;
      continue;
    }

    std::cerr << "[velox-compiler:warning] Unknown argument '" << arg << "'" << std::endl;
  }
}


void compiler::fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args)
{
  size_t count = 0;
  for (auto& arg : args) { // parcours en sens inverse
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
      templateStr.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}

const std::string& compiler::get_home_dir()
{
  static std::string cached;
  if (!cached.empty()) return cached;

#ifdef _WIN32
  // 1 - USERPROFILE
  if (const char* userprofile = std::getenv("USERPROFILE")) return cached = fs::path(userprofile).string();

  // 2 - HOME (MSYS / Git Bash / hybrid)
  if (const char* home = std::getenv("HOME")) return cached = fs::path(home).string();

  // 3 - official API Windows
  PWSTR path = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path))) {
    fs::path result(path);
    CoTaskMemFree(path);
    return cached = result.string();
  }

  // 4 - fallback legacy
  const char* homeDrive = std::getenv("HOMEDRIVE");
  const char* homePath  = std::getenv("HOMEPATH");
  if (homeDrive && homePath) return cached = fs::path(std::string(homeDrive) + homePath).string();

  throw std::runtime_error("Cannot determine home directory (Windows)");

#else
  // 1) HOME env var
  if (const char* home = std::getenv("HOME")) return cached = fs::path(home).string();

  // 2) POSIX fallback
  if (struct passwd* pwd = getpwuid(getuid())) return cached = fs::path(pwd->pw_dir).string();

  throw std::runtime_error("Cannot determine home directory (POSIX)");
#endif
}

const std::string& compiler::get_exe_path()
{
  static std::string dir;

  if (!dir.empty()) return dir;

#ifdef _WIN32
  wchar_t buffer[MAX_PATH];
  DWORD   len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  if (len == 0) throw std::runtime_error("GetModuleFileNameW failed");
  return dir = fs::path(buffer).string();

#elif __APPLE__
  uint32_t size = 0;
  _NSGetExecutablePath(nullptr, &size); // get required size
  std::string buf(size, '\0');
  if (_NSGetExecutablePath(buf.data(), &size) != 0) throw std::runtime_error("_NSGetExecutablePath failed");
  return dir = fs::canonical(buf).string();

#elif __linux__
  std::vector<char> buf(1024);
  ssize_t           len = readlink("/proc/self/exe", buf.data(), buf.size());
  if (len <= 0) throw std::runtime_error("readlink /proc/self/exe failed");
  return dir = fs::canonical(std::string(buf.data(), len)).string();
#else
#error Unsupported platform
#endif
}

const std::string& compiler::get_exe_dir()
{
  static std::string dir = fs::path(get_exe_path()).parent_path().string();
  return dir;
}

const std::string& compiler::get_stdlib_dir()
{
  static std::string dir;
  if (!dir.empty()) return dir;

  if (const char* env = std::getenv("VELOX_LIB_STANDARD")) {
    return dir = fs::path(env).string();
  } else {
    return dir = fs::weakly_canonical(fs::path(get_exe_dir()) / "stdlib").string();
  }
}

const std::string& compiler::get_packages_dir()
{
  static std::string dir;
  if (!dir.empty()) return dir;

  if (const char* env = std::getenv("VELOX_LIB_USER")) {
    return dir = fs::path(env).string();
  }

#ifdef _WIN32
  if (const char* appdata = std::getenv("APPDATA")) {
    dir = fs::path(appdata) / "velox" / VELOX_COMPILER_VERSION / "packages";
    return dir.string();
  }
  return dir = (fs::path(get_home_dir()) / ".velox" / VELOX_COMPILER_VERSION / "packages").string();
#else
  return dir = (fs::path(get_home_dir()) / ".velox" / VELOX_COMPILER_VERSION / "packages").string();
#endif
}

std::string compiler::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "FSYS";
  case compiler::EPhase::lexer:             return "LEXE";
  case compiler::EPhase::preprosessor:      return "PREP";
  case compiler::EPhase::parser:            return "PARS";
  case compiler::EPhase::binder:            return "EMBI";
  case compiler::EPhase::resolver_symbol:   return "SYMB";
  case compiler::EPhase::resolver_type:     return "TYPE";
  case compiler::EPhase::resolver_semantic: return "SEMA";
  case compiler::EPhase::llvmir:            return "LLVM";
  case compiler::EPhase::linker:            return "LINK";
  }
}

std::string compiler::Phase_to_str(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "file system";
  case compiler::EPhase::lexer:             return "lexer";
  case compiler::EPhase::preprosessor:      return "preprocessor";
  case compiler::EPhase::parser:            return "parser";
  case compiler::EPhase::binder:            return "external module binder";
  case compiler::EPhase::resolver_symbol:   return "resolver symbol";
  case compiler::EPhase::resolver_type:     return "resolver type";
  case compiler::EPhase::resolver_semantic: return "resolver semantic";
  case compiler::EPhase::llvmir:            return "LLVM IR";
  case compiler::EPhase::linker:            return "linker";
  }
}