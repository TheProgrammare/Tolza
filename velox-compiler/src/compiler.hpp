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

// log color
#define color_RESET         "\033[0m"
#define color_BLACK         "\033[30m" /* Black */
#define color_RED           "\033[31m" /* Red */
#define color_GREEN         "\033[32m" /* Green */
#define color_YELLOW        "\033[33m" /* Yellow */
#define color_BLUE          "\033[34m" /* Blue */
#define color_MAGENTA       "\033[35m" /* Magenta */
#define color_CYAN          "\033[36m" /* Cyan */
#define color_WHITE         "\033[37m" /* White */
//
#define k_pointer_size      sizeof(void*)
#define k_architecture_size sizeof(void*)
#define k_max_path_seg_size 12
#define k_max_keyword_size  32

namespace fs = std::filesystem;


namespace compiler
{

constexpr const char* VELOX_COMPILER_VERSION = "2026.2.0b";
extern bool           in_binding_compilation;
extern bool           command_from_velox_toolchain;


constexpr const char* k_comp_abort =
    R"([velox-compiler] Compilation aborted
[note] You must resolve all stage errors before to pass to the next stage!"
Please see above to locate all errors.
)";

struct CompCtx {
  enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN, STATIC_LIB, DYNAMIC_LIB };

  std::map<std::string, std::string> COMPILATION_ARGS;

  const int    argc = 0;
  const char** argv = nullptr;

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

  // print
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
};

inline CompCtx COMP_CTX;

void parse_args_for_compilation_context(CompCtx& ctx, int argc, const char* argv[]);

void fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args);


enum class EPhase {
  filesystem,
  lexer,
  preprosessor,
  parser,
  binder,
  resolver_symbol,
  resolver_type,
  resolver_semantic,
  llvmir,
  linker
};

[[nodiscard]] fs::path    get_home_dir();
[[nodiscard]] fs::path    get_stdlib_dir();
[[nodiscard]] fs::path    get_user_lib_path();
[[nodiscard]] fs::path    get_exe_path();
[[nodiscard]] fs::path    get_exe_dir();
[[nodiscard]] fs::path    get_packages_dir();
[[nodiscard]] std::string Phase_to_code(EPhase phase);
[[nodiscard]] std::string Phase_to_str(EPhase phase);


} // namespace compiler
