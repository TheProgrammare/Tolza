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
#include <filesystem>
#include <optional>
#include <expected>

namespace fs = std::filesystem;


namespace toolchain
{

void log(const std::string& msg);
void err(const std::string& msg);

struct CompCtx {
  enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN };
  std::string EEmitMode_to_str() const
  {
    switch (codegen_emit_mode) {
    case EEmitMode::LLVM: return "llvm";
    case EEmitMode::OBJ:  return "obj";
    case EEmitMode::ASM:  return "asm";
    case EEmitMode::BC:   return "bc";
    case EEmitMode::BIN:  return "bin";
    }
  };

  fs::path config_path;

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
  EEmitMode codegen_emit_mode = EEmitMode::BIN;
  fs::path  codegen_build_dir;

  // project
  fs::path project_dir;
  fs::path source_dir;
  fs::path vendor_dir;
  fs::path ffi_json_dir;
  fs::path binding_dir;
  fs::path compiler_file;

  // sub_configs
  std::map<std::string, fs::path> sub_configs;

  const fs::path& get_config_file() const
  {
    static fs::path out;
    if (!out.empty()) return out;

    if (target_config.empty() || target_config == "self") return out = config_path;
    if (auto find = sub_configs.find(target_config); find != sub_configs.end()) return out = find->second;

    return out = config_path;
  }

  const fs::path& get_project_dir() const
  {
    return project_dir;
  }

  const fs::path& get_source_dir() const
  {
    return source_dir;
  }

  const fs::path& get_vendor_dir() const
  {
    return vendor_dir;
  }

  const fs::path& get_build_dir() const
  {
    return codegen_build_dir;
  }

  const fs::path& get_compiler_file() const
  {
    return compiler_file;
  }

  const fs::path& get_preprocess_dir() const
  {
    static auto out = get_build_dir() / "preprocess";
    return out;
  }

  const fs::path& get_binding_dir() const
  {
    static auto out = binding_dir;
    return out;
  }

  const fs::path& get_debug_graph_dir() const
  {
    static auto out = get_build_dir() / "graph";
    return out;
  }

  const fs::path& get_llvmir_dir() const
  {
    static auto out = get_build_dir() / "llvm-ir";
    return out;
  }

  const fs::path& get_ffi_json_dir() const
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
  std::optional<std::string> target_abi;
  std::optional<std::string> target_arch;
  std::optional<size_t>      target_bits;
  std::optional<std::string> target_os;
  std::optional<std::string> target_libc;
  std::optional<std::string> target_config;

  // profile
  std::optional<bool>   profile_debug;
  std::optional<size_t> profile_opt_level;
  std::optional<bool>   profile_size_opt;

  // logs
  std::optional<bool> log_all;
  std::optional<bool> log_filesystem;
  std::optional<bool> log_lexer;
  std::optional<bool> log_preprocessor;
  std::optional<bool> log_parser;
  std::optional<bool> log_binder;
  std::optional<bool> log_exporter;
  std::optional<bool> log_resolver;
  std::optional<bool> log_LLVM_IR;
  std::optional<bool> log_linker;

  // warnings
  std::optional<bool>   warn_all;
  std::optional<bool>   warn_extra;
  std::optional<bool>   warn_pedantic;
  std::optional<size_t> warn_level;
  std::optional<bool>   warn_unused;
  std::optional<bool>   warn_dead_code;
  std::optional<bool>   warn_as_error;

  // dot
  std::optional<bool> dot_ast;
  std::optional<bool> dot_link;


  // defines
  std::map<std::string, std::string> defines;
  EMergeMode                         defines_merge_mode = EMergeMode::_union;

  // undefines
  std::vector<std::string> undefines;
  EMergeMode               undefines_merge_mode = EMergeMode::_union;

  // codegen
  std::optional<EEmitMode> codegen_emit_mode;
  std::optional<fs::path>  codegen_build_dir;

  // project
  std::optional<fs::path> project_dir;
  std::optional<fs::path> source_dir;
  std::optional<fs::path> vendor_dir;
  std::optional<fs::path> ffi_json_dir;
  std::optional<fs::path> compiler_file;

  // sub_configs
  std::map<std::string, fs::path> sub_configs;

  fs::path get_project_dir() const
  {
    return project_dir.value();
  }
};

inline constexpr std::string VELOX_TOOLCHAIN_VERSION = "2026.2.0b";

inline constexpr std::string DETECTED_OS_NAME =
#ifdef _WIN32
    "windows";
#elif __APPLE__
    "mac";
#elif __linux__
    "linux";
#else
    "unknown";
#endif

inline constexpr std::string DETECTED_ARCH =
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

inline constexpr std::string DETECTED_BITS =
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    "64";
#elif defined(__i386) || defined(_M_IX86) || defined(__arm__)
    "32";
#else
    "0";
#endif

inline constexpr std::string DETECTED_ABI =
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__)
    "LP64";
#elif defined(__ILP32__) || defined(__i386)
    "ILP32";
#else
    "unknown";
#endif


namespace fs = std::filesystem;

struct Version {
  int  year, month, day;
  char suffix; // '\0' = stable, 'b' = beta, 'p' = preview

  Version(const std::string& str, const std::string& separator = "-")
  {
    suffix        = '\0';
    size_t first  = str.find(separator);
    size_t second = str.find(separator, first + 1);
    if (first == std::string::npos || second == std::string::npos)
      throw std::invalid_argument("Invalid format version: " + str);

    year  = std::stoi(str.substr(0, first));
    month = std::stoi(str.substr(first + 1, second - first - 1));

    std::string dayPart = str.substr(second + 1);
    if (!dayPart.empty() && !isdigit(dayPart.back())) {
      suffix = dayPart.back();
      dayPart.pop_back();
    }
    day = std::stoi(dayPart);
  }

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

std::vector<std::pair<Version, fs::path>> find_all_compilers();
std::optional<fs::path>                   find_compiler_version(const std::string& version);
std::optional<fs::path>                   find_lastest_compiler();

} // namespace toolchain
