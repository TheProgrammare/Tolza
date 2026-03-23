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

constexpr const char* VELOX_COMMON_VERSION = "2026.2.0b";

namespace common
{

struct CompCtx {
  CompCtx()
  {
  }

  std::string current_config_file;

  std::map<std::string, std::string> COMPILATION_ARGS;

  int          argc      = 0;
  const char** argv      = nullptr;
  int          llvm_argc = 0;
  const char** llvm_argv = nullptr;


  // target
  std::string target_abi;
  std::string target_arch;
  size_t      target_bits = 64;
  std::string target_os;
  std::string target_libc;
  std::string target_config;

  // profile
  bool   profile_debug           = false;
  size_t profile_opt_level       = 0;
  bool   profile_size_opt        = false;
  bool   profile_extrem_size_opt = false;

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
  bool        emit_bin         = false;
  bool        emit_llvm        = false;
  bool        emit_obj         = false;
  bool        emit_asm         = false;
  bool        emit_bc          = false;
  bool        emit_static_lib  = false;
  bool        emit_dynamic_lib = false;
  bool        pic_mode         = true;
  std::string codegen_build_dir;

  // project
  std::string project_dir;
  std::string source_dir;
  std::string vendor_dir;
  std::string ffi_json_dir;
  std::string binding_dir;

  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;

  const std::string& get_preprocess_dir() const;

  const std::string& get_debug_graph_dir() const;

  const std::string& get_llvmir_dir() const;

  const std::string& get_project_name() const;

  // object, executable, ...
  // name from .config file name
  const std::string& get_out_name() const;

  const std::string& get_config_file() const;


  void                     apply_args(int _argc, const char* _argv[]);
  std::vector<std::string> to_args() const;

  static CompCtx invalid()
  {
    return CompCtx();
  }

  bool is_valid() const
  {
    return argc != 0;
  }

  operator bool()
  {
    return argc != 0;
  }
};


// for sub configuration
struct Sub_CompCtx : CompCtx {
  enum class EMergeMode { _union, _intersection, _anti_intersection };

  std::string current_config_file;

  EMergeMode COMPILATION_ARGS_merge_mode = EMergeMode::_union;

  // profile
  bool has_profile_debug           = false;
  bool has_profile_opt_level       = false;
  bool has_profile_size_opt        = false;
  bool has_profile_extrem_size_opt = false;

  // logs
  bool has_log_all          = false;
  bool has_log_filesystem   = false;
  bool has_log_lexer        = false;
  bool has_log_preprocessor = false;
  bool has_log_parser       = false;
  bool has_log_binder       = false;
  bool has_log_exporter     = false;
  bool has_log_resolver     = false;
  bool has_log_LLVM_IR      = false;
  bool has_log_linker       = false;

  // warnings
  bool has_warn_all       = false;
  bool has_warn_extra     = false;
  bool has_warn_pedantic  = false;
  bool has_warn_level     = false;
  bool has_warn_unused    = false;
  bool has_warn_dead_code = false;
  bool has_warn_as_error  = false;

  // print
  bool has_print_ast = false;

  // defines
  EMergeMode defines_merge_mode = EMergeMode::_union;

  // undefines
  EMergeMode undefines_merge_mode = EMergeMode::_union;

  // codegen
  // codegen
  bool has_emit_bin         = false;
  bool has_emit_llvm        = false;
  bool has_emit_obj         = false;
  bool has_emit_asm         = false;
  bool has_emit_bc          = false;
  bool has_emit_static_lib  = false;
  bool has_emit_dynamic_lib = false;
  bool has_pic_mode         = false;

  CompCtx merge_context(const CompCtx& base_ctx) const;
};


} // namespace common
