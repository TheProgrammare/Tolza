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
#include <set>

namespace common
{

struct Compiler_Options {
  Compiler_Options()
  {
  }

  enum class ERelocModel { Static, PIC, PIE, ROPI, RWPI, ROPI_RWPI };
  enum class ECodeModel { Tiny, Small, Kernel, Medium, Large };
  enum class EOptimization { O0, O1, O2, O3, Os, Oz };
  enum class EWarnLevel { W0, W1, W2, W3 };
  enum class EEmit { Bin, LLVM, Obj, ASM, BC, s_lib, d_lib };

  static std::string ERelocModel_to_str(ERelocModel reloc);
  static std::string ECodeModel_to_str(ECodeModel code);
  static std::string EOptimization_to_str(EOptimization opt);
  static std::string EWarnLevel_to_str(EWarnLevel wlevel);
  static std::string EEmit_to_str(EEmit emit);
  static EEmit       str_to_EEmit(const std::string& s);

  std::string current_config_file;

  std::map<std::string, std::string> PREPROCESSOR_ARGS;

  int          argc = 0;
  const char** argv = nullptr;

  bool mute = false;

  // target
  std::string     target_project_name;
  std::string     target_arch;
  std::string     target_os;
  std::string     target_vendor;
  std::string     target_abi;
  std::string     target_libc;
  std::string     target_cpu;
  std::string     target_features;
  ECodeModel      target_code_model  = ECodeModel::Small;
  ERelocModel     target_reloc_model = ERelocModel::PIC;
  std::string     target_sub_config;
  std::set<EEmit> target_emits = {EEmit::Bin};

  // profile
  bool          profile_debug        = false;
  EOptimization profile_optimization = EOptimization::O0;

  // logs
  std::set<std::string> logs;

  // warnings
  std::set<std::string> warns;
  EWarnLevel            warn_level = EWarnLevel::W0;

  // debug_printer
  std::set<std::string> debugs;

  // defines
  std::map<std::string, std::string> defines;

  // undefines
  std::vector<std::string> undefines;

  // directories
  std::string dir_project  = "./";
  std::string dir_build    = "./build";
  std::string dir_source   = "./src";
  std::string dir_vendor   = "./vendor";
  std::string dir_binding  = "./binding";
  std::string dir_ffi_json = "./binding/ffi_json";
  std::string dir_compiler = "~/velox-compiler";
  std::string dir_stdlib;
  std::string dir_packages;

  std::string get_dir_project() const;
  std::string get_dir_build() const;
  std::string get_dir_source() const;
  std::string get_dir_vendor() const;
  std::string get_dir_binding() const;
  std::string get_dir_ffi_json() const;
  std::string get_dir_compiler() const;
  std::string get_dir_stdlib() const;
  std::string get_dir_packages() const;


  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;

  // llvm
  std::string              llvm_triple;
  bool                     llvm_verify_module = true;
  std::vector<const char*> llvm_args;

  size_t get_arch_size() const;

  const std::string& get_preprocess_dir() const;

  const std::string& get_debug_graph_dir() const;

  const std::string& get_llvmir_dir() const;

  std::string get_project_name() const;

  // object, executable, ...
  // name from .config file name
  const std::string& get_out_name() const;

  const std::string& get_config_file() const;

  std::string get_target_triple() const;

  void generate_preprocessor_args();

  std::vector<std::string> to_args() const;

  static Compiler_Options invalid()
  {
    return Compiler_Options();
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
struct Sub_Compiler_Options : Compiler_Options {
  enum class EMergeMode { _union, _intersection, _anti_intersection };

  std::string current_config_file;

  EMergeMode COMPILATION_ARGS_merge_mode = EMergeMode::_union;

  // target
  bool       has_target_code_model  = false;
  bool       has_target_reloc_model = false;
  EMergeMode emits_merge_mode       = EMergeMode::_union;


  // profile
  bool has_profile_debug        = false;
  bool has_profile_optimization = false;

  // logs
  EMergeMode logs_merge_mode = EMergeMode::_union;

  // warnings
  EMergeMode warnings_merge_mode = EMergeMode::_union;
  bool       has_warn_level      = false;

  // debugs
  EMergeMode debug_merge_mode = EMergeMode::_union;

  // defines
  EMergeMode defines_merge_mode = EMergeMode::_union;

  // undefines
  EMergeMode undefines_merge_mode = EMergeMode::_union;

  // codegen

  // llvm
  bool has_llvm_verify_module = false;

  Compiler_Options merge_context(const Compiler_Options& base_ctx) const;
};


// config key argument linker
struct Key_Arg_Asso {
  std::string config_key;
  std::string config_val;
  std::string arg;
  std::string arg_alt;

  // for set comparison
  auto operator<=>(const Key_Arg_Asso& other) const = default;

  static bool               val_is_valid(const std::string& key, const std::string& val);
  static const std::string& val_to_arg(const std::string& key, const std::string& val);
  static const std::string& arg_to_val(const std::string& arg);
};

inline const std::set<Key_Arg_Asso> kas = {
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "bin",               .arg = "--emit-bin"                   },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "llvm",              .arg = "--emit-llvm"                  },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "obj",               .arg = "--emit-obj"                   },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "asm",               .arg = "--emit-asm"                   },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "bc",                .arg = "--emit-bc"                    },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "s_lib",             .arg = "--emit-s-lib"                 },
    Key_Arg_Asso{.config_key = "target.emits",         .config_val = "d_lib",             .arg = "--emit-d-lib"                 },
    Key_Arg_Asso{.config_key = "target.code_model",    .config_val = "tiny",              .arg = "--code-model=tiny"            },
    Key_Arg_Asso{.config_key = "target.code_model",    .config_val = "small",             .arg = "--code-model=small"           },
    Key_Arg_Asso{.config_key = "target.code_model",    .config_val = "kernel",            .arg = "--code-model=kernel"          },
    Key_Arg_Asso{.config_key = "target.code_model",    .config_val = "medium",            .arg = "--code-model=medium"          },
    Key_Arg_Asso{.config_key = "target.code_model",    .config_val = "large",             .arg = "--code-model=large"           },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "static",            .arg = "--reloc-model-model=static"   },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "pic",               .arg = "--reloc-model-model=pic"      },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "pie",               .arg = "--reloc-model-model=pie"      },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "ropi",              .arg = "--reloc-model-model=ropi"     },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "rwpi",              .arg = "--reloc-model-model=rwpi"     },
    Key_Arg_Asso{.config_key = "target.reloc_model",   .config_val = "ropi_rwpi",         .arg = "--reloc-model-model=ropi_rwpi"},
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "O0",                .arg = "-O0"                          },
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "O1",                .arg = "-O1"                          },
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "O2",                .arg = "-O2"                          },
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "O3",                .arg = "-O3"                          },
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "Os",                .arg = "-Os"                          },
    Key_Arg_Asso{.config_key = "profile.optimization", .config_val = "Oz",                .arg = "-Oz"                          },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "all",               .arg = "--log-all"                    },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "filesystem",        .arg = "--log-filesystem"             },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "lexer",             .arg = "--log-lexer"                  },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "preprocessor",      .arg = "--log-preprocessor"           },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "parser",            .arg = "--log-parser"                 },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "binder",            .arg = "--log-binder"                 },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "exporter",          .arg = "--log-exporter"               },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "resolver_symbol",   .arg = "--log-resolver_symbol"        },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "resolver_type",     .arg = "--log-resolver_type"          },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "resolver_semantic", .arg = "--log-resolver_semantic"      },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "codegen",           .arg = "--log-codegen"                },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "optimization",      .arg = "--log-optimization"           },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "emit",              .arg = "--log-emit"                   },
    Key_Arg_Asso{.config_key = "log.logs",             .config_val = "linker",            .arg = "--log-linker"                 },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "all",               .arg = "--warn-all"                   },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "extra",             .arg = "--warn-extra"                 },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "pedantic",          .arg = "--warn-pedantic"              },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "unused",            .arg = "--warn-unused"                },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "dead_code",         .arg = "--warn-dead_code"             },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "as_error",          .arg = "--warn-as_error"              },
    Key_Arg_Asso{.config_key = "warning.warnings",     .config_val = "level",             .arg = "--warn-level"                 },
    Key_Arg_Asso{.config_key = "warning.level",        .config_val = "W0",                .arg = "-W0"                          },
    Key_Arg_Asso{.config_key = "warning.level",        .config_val = "W1",                .arg = "-W1"                          },
    Key_Arg_Asso{.config_key = "warning.level",        .config_val = "W2",                .arg = "-W2"                          },
    Key_Arg_Asso{.config_key = "warning.level",        .config_val = "W3",                .arg = "-W3"                          },
    Key_Arg_Asso{.config_key = "debug.debugs",         .config_val = "ast",               .arg = "--debug-ast"                  },
};


} // namespace common
