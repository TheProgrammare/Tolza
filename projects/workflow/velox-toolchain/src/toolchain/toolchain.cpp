#include "toolchain/toolchain.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <expected>
#include <iostream>

#include <common.hpp>
#include <toolchain_context.hpp>
#include <marzer/toml++.hpp>

namespace fs = std::filesystem;

extern const std::string common::SOFTWARE_NAME = "velox-toolchain";

void toolchain::link_stdlib()
{
  fs::path stdpath = common::get_stdlib_dir();
  fs::path source  = common::resolve_path(fs::path(common::get_exe_dir()) / ".." / ".." / ".." / "libs" / "std");

  fs::remove(stdpath);
  fs::create_directories(stdpath.parent_path());

  // Supprime le lien ou dossier existant si présent
  std::error_code ec;
  if (fs::exists(stdpath, ec)) {
    fs::remove(stdpath, ec);
    if (ec) {
      std::cerr << "Cannot delete old symlink : " << ec.message() << "\n";
      return;
    }
  }

#if defined(_WIN32)
  if (!fs::create_directory_symlink(fs::absolute(source), stdpath, ec)) {
    std::cerr << "Cannot create the symlink: " << ec.message() << "\n";
  }
#elif __unix__
  fs::create_symlink(fs::absolute(source), stdpath, ec);
  if (ec) {
    std::cerr << "cannot create the symlink : " << ec.message() << "\n";
  }
#endif
}


void toolchain::err(const std::string& msg)
{
  std::cerr << "[velox:ERROR] " << msg << std::endl;
}

void toolchain::log(const std::string& msg)
{
  std::cerr << "[velox] " << msg << std::endl;
}


const char* toolchain::VELOX_CONFIG_TEMPLATE =
    R"(
# main velox toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

# all fields will be stored as define element also


# ======================
# velox-compiler section 
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
# It does not affect your Velox preprocessor.

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

const char* toolchain::VELOX_MAIN_TEMPLATE =
    R"(
import ext: C::stdio

fn main() {
  C::printf("hello world!"c_str)
}

)";

const char* toolchain::VELOX_CORE_TEMPLATE =
    R"(
import ext: C::stdio
import usr: ffi::C
    
export {
  fn println(ref msg: str) {
    C::printf(ffi::C::str_to_cstr(msg))
  }
}

)";