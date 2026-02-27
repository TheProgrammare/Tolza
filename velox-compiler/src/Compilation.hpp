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

/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */

#pragma once

#include <map>
#include <optional>
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
  bool log_EMBinder;
  bool log_exporter;
  bool log_resolver;
  bool log_LLVM_IR;
  bool log_Linker;

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

bool                   new_velox_workspace();
std::optional<CompCtx> init_compilation_context(const fs::path& path);
bool                   check_workspace_sanity(const fs::path& ws_path);
void                   generate_velox_workspace(const std::string& project_name, const fs::path& path);
void                   check_velox_config_sanity(const fs::path& file, bool full_config);
CompCtx                parse_compilation_context(const fs::path& config_path);


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


// key: section | key: field
static std::map<std::string, std::string> k_config_map = {
    {"target",      "abi"            },
    {"target",      "arch"           },
    {"target",      "bits"           },
    {"target",      "os"             },
    {"target",      "libc"           },
    {"target",      "config"         },
    {"profile",     "debug"          },
    {"profile",     "opt_level"      },
    {"profile",     "size_opt"       },
    {"logs",        "all"            },
    {"logs",        "filesystem"     },
    {"logs",        "lexer"          },
    {"logs",        "preprocessor"   },
    {"logs",        "parser"         },
    {"logs",        "embinder"       },
    {"logs",        "exporter"       },
    {"logs",        "resolver"       },
    {"logs",        "llvm-ir"        },
    {"logs",        "linker"         },
    {"defines",     ""               },
    {"undefines",   ""               },
    {"codegen",     "emit_mode"      },
    {"codegen",     "output_dir"     },
    {"codegen",     "dest_file_dir"  },
    {"project",     "project_dir"    },
    {"project",     "source_dir"     },
    {"project",     "thrid_party_dir"},
    {"sub_configs", ""               },
};


// %0 project_name
// %1 abi
// %2 arch
// %3 bits
// %4 os
// %5 debug
inline constexpr const char* VELOX_CONFIG_TEMPLATE =
    R"(
# main velox compiler config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

[target]
project_name = "%0"
abi   = "%1"
arch  = "%2"
bits  = %3
os    = "%4"
libc  = ""
# self is for default configuration
config = self

[profile]
debug           = %5
opt_level       = 0
size_opt        = false

[logs]
# override all log options
all             = %5
filesystem      = %5
lexer           = %5
preprocessor    = %5
parser          = %5
EMBinder        = %5
exporter        = %5
resolver        = %5
LLVM_IR         = %5
Linker          = %5

[defines]
VERSION = "1.0"

[undefines]
EXAMPLE

[codegen]
# LLVM | OBJ | ASM | BC | BIN
emit_mode  = "BIN"
output_dir = "./build"
dest_file_dir  = "./build/app"

[project]
project_dir = "./"
source_dir  = "./src"
thrid_party_dir = "./thirdparty"

[sub_configs]
debug = "./config/debug.config"
)";

inline constexpr const char* VELOX_MAIN_TEMPLATE =
    R"(
import std::core

fn main() {
  println("hello world!")
}

)";