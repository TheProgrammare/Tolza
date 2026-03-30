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

#include <string>

namespace toolchain
{


void init_autocompletion();
void link_stdlib();

void log(const std::string& msg);
void err(const std::string& msg);

inline constexpr const char* VELOX_CONFIG_TEMPLATE =
    R"(
# main velox toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

# all fields will be stored as define element also


# ======================
# velox-compiler section 
# ======================

[target]
project_name        = "%target_project_name"        # if empty, the workspace file name will be used
arch                = "%target_arch"                # x86_64, aarch64, wasm32, ...
os                  = "%target_os"                  # linux, windows, macos, ...
vendor              = "%target_vendor"              # apple, pc, ...
abi                 = "%target_abi"                 # gnu, msvc, ...
abi_size            = "%target_size_abi"            # LP64, ILP32, LLP64
libc                = "%target_libc"                # musl, glibc, bionic, msvc, ...
cpu                 = "%target_cpu"                 # overrides host cpu detection
features            = "%target_features"            # ex: "+avx2,+bmi2"
code_model          = "%target_code_model"          # tiny, small, kernel, medium, large
reloc_model         = "%target_reloc_model"         # pic, static, pie, ropi, rwpi, ropi_rwpi
sub_config          = "%target_sub_config"          # sub .toml name to apply after this .toml 
emits               = [                             # bin, llvm, obj, asm, bc, s_lib, d_lib
  %target_emit
]                

[profile]
# the debug profile will override some options
debug               = %profile_debug                # true = disable optimization, enable debug info
optimization        = "%profile_optimization"       # O0,O1,O2,O3,Os,Oz

[log]
# all, filesystem, lexer, preprocessor, parser, binder, exporter, 
# resolver_symbol, resolver_type, resolver_semantic, 
# codegen, optimization, emit, linker
logs = [
  %logs
]                                    

[warning]
# all, extra, pedantic, unused, dead_code, as_error
warnings = [
  %warnings
]
# 1, 2, 3
level               = %warn_level  


[debug]
# ast
debugs = [
  %debugs
]

[define]
%defines

[undefine]
undefines = [
  %undefines
]

[directory]
project             = "%dir_project"
build               = "%dir_build"  
source              = "%dir_source"  
vendor              = "%dir_vendor"  
ffi_json            = "%dir_ffi_json"  
binding             = "%dir_binding"  
compiler            = "%dir_compiler"  
stdlib              = "%dir_stdlib"  
packages            = "%dir_packages"

[config]
# designed to target a sub configuration parameters
%sub_configs

# ============
# LLVM section
# ============
# This configuration is strictly for LLVM passes and code generation.
# It does not affect your Velox preprocessor.

[llvm]
triple              = "%llvm_triple"          # override triple, else auto-generated from target
verify_module       = %llvm_verify_module     # verify before AND after passes
args = [                                      # custom raw flags passed to LLVM
  %llvm_args
]
)";

inline constexpr const char* VELOX_MAIN_TEMPLATE =
    R"(
import ext: C::stdio

fn main() {
  C::printf("hello world!"c_str)
}

)";

inline constexpr const char* VELOX_CORE_TEMPLATE =
    R"(
import ext: C::stdio
import usr: ffi::C
    
export {
  fn println(ref msg: str) {
    C::printf(ffi::C::str_to_cstr(msg))
  }
}

)";


inline constexpr const char* VELOX_AUTOCOMPLETION_BASH = R"(
#!/bin/bash

_velox_completions() {
    local cur prev commands subcommands options
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # Main commands
    commands="compiler create gui check audit build generate-ffi-json pkg"

    # Sub-commands for specific commands
    if [[ $COMP_CWORD == 1 ]]; then
        COMPREPLY=( $(compgen -W "${commands}" -- "${cur}") )
        return 0
    fi

    case "${COMP_WORDS[1]}" in
        compiler)
            subcommands="find set cogito"
            options="--version -v --latest -l --all"
            if [[ $COMP_CWORD == 2 ]]; then
                COMPREPLY=( $(compgen -W "${subcommands}" -- "${cur}") )
                return 0
            fi
            case "${COMP_WORDS[2]}" in
                find|set)
                    COMPREPLY=( $(compgen -W "${options}" -- "${cur}") )
                    return 0
                    ;;
            esac
            ;;
        create)
            subcommands="workspace config"
            if [[ $COMP_CWORD == 2 ]]; then
                COMPREPLY=( $(compgen -W "${subcommands}" -- "${cur}") )
                return 0
            fi
            ;;
        gui)
            ;;
        check)
            subcommands="workspace config"
            if [[ $COMP_CWORD == 2 ]]; then
                COMPREPLY=( $(compgen -W "${subcommands}" -- "${cur}") )
                return 0
            fi
            ;;
        audit)
            ;;
        build)
            options="--help -h --debug -d --release -r --emit-obj --emit-asm --emit-bc --emit-bin --emit-llvm"
            COMPREPLY=( $(compgen -W "${options}" -- "${cur}") )
            return 0
            ;;
        generate-ffi-json)
            ;;
        pkg)
            subcommands="install remove info purge check list update upgrade"
            options="--installed --upgradable"
            if [[ $COMP_CWORD == 2 ]]; then
                COMPREPLY=( $(compgen -W "${subcommands}" -- "${cur}") )
                return 0
            elif [[ $COMP_CWORD == 3 ]]; then
                COMPREPLY=( $(compgen -W "${options}" -- "${cur}") )
                return 0
            fi
            ;;
    esac
}

complete -F _velox_completions velox
)";

} // namespace toolchain
