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

namespace compiler
{

struct CompCtx {
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

  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;

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

  const std::string& get_binding_dir() const
  {
    return binding_dir;
  }

  const std::string& get_preprocess_dir() const;

  const std::string& get_debug_graph_dir() const;

  const std::string& get_llvmir_dir() const;

  const std::string& get_ffi_json_dir() const
  {
    return ffi_json_dir;
  }
};


} // namespace compiler
