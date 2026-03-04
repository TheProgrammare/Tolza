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

namespace fs = std::filesystem;

namespace toolchain
{

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

  std::map<std::string, std::string> COMPILATION_ARGS;

  // target
  std::string target_abi;
  std::string target_arch;
  size_t      target_bits;
  std::string target_os;
  std::string target_libc;
  std::string target_config;

  // profile
  bool   profile_debug;
  size_t profile_opt_level;
  bool   profile_size_opt;

  // logs
  bool log_all;
  bool log_filesystem;
  bool log_lexer;
  bool log_preprocessor;
  bool log_parser;
  bool log_binder;
  bool log_exporter;
  bool log_resolver;
  bool log_LLVM_IR;
  bool log_linker;

  // warnings
  bool   warn_all;
  bool   warn_extra;
  bool   warn_pedantic;
  size_t warn_level;
  bool   warn_unused;
  bool   warn_dead_code;
  bool   warn_as_error;

  // dot
  bool dot_ast;
  bool dot_link;


  // defines
  std::map<std::string, std::string> defines;

  // undefines
  std::vector<std::string> undefines;

  // codegen
  EEmitMode codegen_emit_mode;
  fs::path  codegen_build_dir;

  // project
  fs::path project_dir;
  fs::path source_dir;
  fs::path vendor_dir;
  fs::path ffi_json_dir;
  fs::path compiler_file;

  // sub_configs
  std::map<std::string, fs::path> sub_configs;

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
    static auto out = get_build_dir() / "binding";
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
  std::vector<const char*> to_args() const;
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
  EMergeMode                         defines_merge_mode;

  // undefines
  std::vector<std::string> undefines;
  EMergeMode               undefines_merge_mode;

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

inline constexpr const char* VELOX_TOOLCHAIN_VERSION = "2026.2.0b";

inline constexpr const char* DETECTED_OS_NAME =
#ifdef _WIN32
    "windows";
#elif __APPLE__
    "mac";
#elif __linux__
    "linux";
#else
    "unknown";
#endif

inline constexpr const char* DETECTED_ARCH =
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

inline constexpr const char* DETECTED_BITS =
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    "64";
#elif defined(__i386) || defined(_M_IX86) || defined(__arm__)
    "32";
#else
    "0";
#endif

inline constexpr const char* DETECTED_ABI =
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__)
    "LP64";
#elif defined(__ILP32__) || defined(__i386)
    "ILP32";
#else
    "unknown";
#endif

} // namespace toolchain
