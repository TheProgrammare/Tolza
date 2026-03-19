/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <map>
#include <vector>
#include <string>
#include <optional>

namespace toolchain
{

void log(const std::string& msg);
void err(const std::string& msg);

struct CompCtx {
  std::string config_path;

  std::map<std::string, std::string> COMPILATION_ARGS;

  // target
  std::string target_abi;
  std::string target_arch;
  size_t      target_bits = 64;
  std::string target_os;
  std::string target_libc;
  std::string target_config;

  // profile
  bool   profile_debug     = false;
  size_t profile_opt_level = 0;
  bool   profile_size_opt  = false;

  // logs
  bool log_all          = false;
  bool log_filesystem   = false;
  bool log_lexer        = false;
  bool log_preprocessor = false;
  bool log_parser       = false;
  bool log_binder       = false;
  bool log_exporter     = false;
  bool log_resolver     = false;
  bool log_LLVM_IR      = false;
  bool log_linker       = false;

  // warnings
  bool   warn_all       = false;
  bool   warn_extra     = false;
  bool   warn_pedantic  = false;
  size_t warn_level     = 0;
  bool   warn_unused    = false;
  bool   warn_dead_code = false;
  bool   warn_as_error  = false;

  // printer
  bool print_ast = false;


  // defines
  std::map<std::string, std::string> defines;

  // undefines
  std::vector<std::string> undefines;

  // codegen
  bool        emit_bin         = false;
  bool        emit_llvm        = false;
  bool        emit_obj         = false;
  bool        emit_asm         = false;
  bool        emit_bc          = false;
  bool        emit_static_lib  = false;
  bool        emit_dynamic_lib = false;
  std::string codegen_build_dir;

  // project
  std::string project_dir;
  std::string source_dir;
  std::string vendor_dir;
  std::string ffi_json_dir;
  std::string binding_dir;
  std::string compiler_file;

  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;

  const std::string& get_config_file() const
  {
    static std::string out;
    if (!out.empty()) return out;

    if (target_config.empty() || target_config == "self") return out = config_path;
    if (auto find = sub_configs.find(target_config); find != sub_configs.end()) return out = find->second;

    return out = config_path;
  }

  const std::string& get_project_dir() const
  {
    return project_dir;
  }

  const std::string& get_source_dir() const
  {
    return source_dir;
  }

  const std::string& get_vendor_dir() const
  {
    return vendor_dir;
  }

  const std::string& get_build_dir() const
  {
    return codegen_build_dir;
  }

  const std::string& get_compiler_file() const
  {
    return compiler_file;
  }

  const std::string& get_preprocess_dir() const;

  const std::string& get_binding_dir() const
  {
    static auto out = binding_dir;
    return out;
  }

  const std::string& get_debug_graph_dir() const;

  const std::string& get_llvmir_dir() const;

  const std::string& get_ffi_json_dir() const
  {
    return ffi_json_dir;
  }
  std::vector<std::string> to_args() const;
};

// for sub configuration
struct CompCtx_Optional {
  enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN };
  // union: A{1,2} + A{1}B{2} = A{1,2}B{2}
  // intersection: A{1,2} + A{1}B{2} = A{1}
  // anti intersection: A{1,2} + A{1}B{2} = A{2}B{2}
  // any copy overrided bu the children
  enum class EMergeMode { _union, _intersection, _anti_intersection };


  // target
  std::string target_abi;
  std::string target_arch;
  size_t      target_bits = 0;
  std::string target_os;
  std::string target_libc;
  std::string target_config;

  // profile
  bool   profile_debug         = false;
  bool   has_profile_debug     = false;
  size_t profile_opt_level     = 0;
  bool   has_profile_opt_level = false;
  bool   profile_size_opt      = false;
  bool   has_profile_size_opt  = false;

  // logs

  bool log_all;
  bool has_log_all = false;
  bool log_filesystem;
  bool has_log_filesystem = false;
  bool log_lexer;
  bool has_log_lexer = false;
  bool log_preprocessor;
  bool has_log_preprocessor = false;
  bool log_parser;
  bool has_log_parser = false;
  bool log_binder;
  bool has_log_binder = false;
  bool log_exporter;
  bool has_log_exporter = false;
  bool log_resolver;
  bool has_log_resolver = false;
  bool log_LLVM_IR;
  bool has_log_LLVM_IR = false;
  bool log_linker;
  bool has_log_linker = false;

  // warnings
  bool   warn_all           = false;
  bool   has_warn_all       = false;
  bool   warn_extra         = false;
  bool   has_warn_extra     = false;
  bool   warn_pedantic      = false;
  bool   has_warn_pedantic  = false;
  size_t warn_level         = 0;
  bool   has_warn_level     = false;
  bool   warn_unused        = false;
  bool   has_warn_unused    = false;
  bool   warn_dead_code     = false;
  bool   has_warn_dead_code = false;
  bool   warn_as_error      = false;
  bool   has_warn_as_error  = false;

  // dot
  bool dot_ast      = false;
  bool has_dot_ast  = false;
  bool dot_link     = false;
  bool has_dot_link = false;

  // defines
  std::map<std::string, std::string> defines;
  EMergeMode                         defines_merge_mode = EMergeMode::_union;

  // undefines
  std::vector<std::string> undefines;
  EMergeMode               undefines_merge_mode = EMergeMode::_union;

  // codegen
  EEmitMode   codegen_emit_mode     = EEmitMode::BIN;
  bool        has_codegen_emit_mode = false;
  std::string codegen_build_dir;
  bool        has_codegen_build_dir = false;

  // project
  std::string project_dir;
  bool        has_project_dir = false;
  std::string source_dir;
  bool        has_source_dir = false;
  std::string vendor_dir;
  bool        has_vendor_dir = false;
  std::string ffi_json_dir;
  bool        has_ffi_json_dir = false;
  std::string compiler_file;
  bool        has_compiler_file = false;

  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;
};

inline constexpr char VELOX_TOOLCHAIN_VERSION[] = "2026.2.0b";

inline constexpr char DETECTED_OS_NAME[] =
#ifdef _WIN32
    "windows";
#elif __APPLE__
    "mac";
#elif __linux__
    "linux";
#else
    "unknown";
#endif

inline constexpr char DETECTED_ARCH[] =
#if defined(__x86_64__) || defined(_M_X64)
    "amd64";
#elif defined(__i386) || defined(_M_IX86)
    "x86";
#elif defined(__aarch64__)
    "arm64";
#elif defined(__arm__)
    "arm";
#else
        "unknown";
#endif

inline constexpr char DETECTED_BITS[] =
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    "64";
#elif defined(__i386) || defined(_M_IX86) || defined(__arm__)
    "32";
#else
    "0";
#endif

inline constexpr char DETECTED_ABI[] =
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__)
    "LP64";
#elif defined(__ILP32__) || defined(__i386)
    "ILP32";
#else
    "unknown";
#endif


struct Version {
  int  year, month, day;
  char suffix; // '\0' = stable, 'b' = beta, 'p' = preview

  Version(const std::string& str, const std::string& separator = "-");

  bool operator<(const Version& other) const
  {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    if (day != other.day) return day < other.day;
    // Stable > preview > beta
    return suffix > other.suffix;
  }

  bool operator==(const Version& other) const
  {
    return year == other.year && month == other.month && day == other.day && suffix == other.suffix;
  }
};

std::vector<std::pair<Version, std::string>> find_all_compilers();
std::string                                  find_compiler_version(const std::string& version);
std::string                                  find_lastest_compiler();

} // namespace toolchain
