/*
 *	The Tolza programming language - Apache License, Version 2.0
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

#include <string_view>

namespace toolchain
{


void              link_stdlib();
[[nodiscard]] int exec_compiler_cmd(std::string_view cmd) noexcept;

constexpr std::string_view SOFTWARE_ABOUT =
    "Tolza-Toolchain\n"
    "  Version: " SOFTWARE_VERSION
    "\n"
    "  Tolza version: " TOLZA_VERSION
    "\n"
    "  License: Apache License, Version 2.0\n"
    "  Author: Florian Foz\n"
    "  Source: https://github.com/TheProgrammare/Tolza";

constexpr std::string_view TOLZA_CONFIG_TEMPLATE =
    R"(
# main tolza toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

# all fields will be stored as define element also


# ======================
# tolza-compiler section 
# ======================

[target]
# if empty, the workspace file name will be used
project_name        = "%target_project_name"        
# x86_64, aarch64, x86, arm, wasm32, wasm64, powerpc64 ...
arch                = "%target_arch"                
# linux, windows, macos, ...
os                  = "%target_os"                  
# apple, pc, unknown ...
vendor              = "%target_vendor"              
# gnu, msvc, ...
abi                 = "%target_abi"           
# musl, glibc, bionic, msvc, ...
libc                = "%target_libc"                
# overrides host cpu detection
cpu                 = "%target_cpu"                 
# ex: "+avx2,+bmi2"
features            = "%target_features"            
# tiny, small, kernel, medium, large
code_model          = "%target_code_model"          
# pic, static, pie, ropi, rwpi, ropi_rwpi
reloc_model         = "%target_reloc_model"         
# sub .toml name to apply after this .toml 
sub_config          = "%target_sub_config"          
# bin, llvm, obj, asm, bc, s_lib, d_lib
emits               = [                             
  %target_emit
]                

[profile]
# the debug profile will override some options

# true = disable optimization, enable debug info
debug               = %profile_debug
# O0,O1,O2,O3,Os,Oz
optimization        = "%profile_optimization"       

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
# It does not affect your Tolza preprocessor.

[llvm]
# override triple, else auto-generated from target
triple              = "%llvm_triple"          
# verify before AND after passes
verify_module       = %llvm_verify_module     
# custom raw flags passed to LLVM
args = [                                      
  %llvm_args
]
)";

constexpr std::string_view TOLZA_MAIN_TEMPLATE =
    R"(
import ext: C::stdio

fn main() {
  C::printf("hello world!"c_str)
}

)";

constexpr std::string_view TOLZA_CORE_TEMPLATE =
    R"(
import ext: C::stdio
import usr: ffi::C
    
export {
  fn println(ref msg: str) {
    C::printf(ffi::C::str_to_cstr(msg))
  }
}

)";

} // namespace toolchain
