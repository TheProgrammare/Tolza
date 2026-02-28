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

namespace fs = std::filesystem;

struct CompCtx {
  enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN };

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
  fs::path  codegen_output_dir;
  fs::path  codegen_dest_file;

  // project
  fs::path project_dir;
  fs::path source_dir;
  fs::path thrid_party_dir;

  // sub_configs
  std::map<std::string, fs::path> sub_configs;
};


inline std::map<std::string, std::string> COMPILATION_ARGS;
inline CompCtx                            COMP_CTX;

inline constexpr const char* VELOX_COMPILER_VERSION = "2026.2.0b";

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
