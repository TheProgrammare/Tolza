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

#include <cstdint>
#include <map>
#include <string_view>
#include <vector>
#include <string>

#include <Neargye/magic_enum.hpp>
#include <Neargye/magic_enum_flags.hpp>

#include "environment.hpp"


namespace common::compiler
{


enum class ECStandard : uint8_t { c89, c99, c11, c17, c23, gnu89, gnu99, gnu11, gnu17, gnu23, unknown };


enum class FCSource : uint16_t {
  NONE                   = 0,
  gnu                    = 1ULL << 0,
  bsd                    = 1ULL << 1,
  darwin                 = 1ULL << 2,
  posix                  = 1ULL << 3,
  xopen                  = 1ULL << 4,
  linux                  = 1ULL << 5,
  android                = 1ULL << 6,
  crt_secure_no_warnings = 1ULL << 7,
  custom                 = 1ULL << 8
};


enum class EEnvironment : uint8_t { gnu, musl, msvc, gnuabi, mingw, darwin, baremetal, wasi, custom, unknown };

enum class ELibC : uint8_t { glibc, musl, libsystem, ucrt, msvcrt, mingw_libc, bionic, bsd_libc, custom, unknown };

[[nodiscard]] constexpr size_t file_offset_bits(ELibC libc, env::EArch arch)
{
  // Windows / MSVC ecosystem does not use POSIX LFS model
  switch (libc) {

  case ELibC::ucrt:
  case ELibC::msvcrt:
  case ELibC::mingw_libc: return 0;

  default:                break;
  }

  // 64-bit architectures already have 64-bit off_t
  if (!is_32bit(arch)) return 0;

  // Unix-like libc where LFS macro is meaningful
  switch (libc) {

  case ELibC::glibc:
  case ELibC::musl:
  case ELibC::bionic:
  case ELibC::bsd_libc:
  case ELibC::libsystem: return 64;

  default:               return 0;
  }
}

constexpr size_t time_bits(ELibC libc, env::EArch arch)
{
  // Windows does not use POSIX time ABI
  switch (libc) {

  case ELibC::ucrt:
  case ELibC::msvcrt:
  case ELibC::mingw_libc: return 0;

  default:                break;
  }

  // 64-bit arch already safe
  if (!is_32bit(arch)) return 0;

  switch (libc) {

  case ELibC::glibc:
  case ELibC::musl:
  case ELibC::bionic:    return 64;

  // BSD + Darwin already have 64-bit time_t
  case ELibC::bsd_libc:
  case ELibC::libsystem:

  default:               return 0;
  }
}

enum class ECallingConv : uint8_t {
  cdecl,
  stdcall,
  fastcall,
  thiscall,
  sysv,
  win64,
  aapcs,
  aapcs_vfp,
  vectorcall,
  custom,
  unknown
};

enum class ERelocModel : uint8_t { NONE, STATIC, PIC, PIE, ROPI, RWPI, ROPI_RWPI };
enum class ECodeModel : uint8_t { NONE, tiny, small, kernel, medium, large };
enum class EOptimization : uint8_t { NONE, O0, O1, O2, O3, Os, Oz };
enum class EWarnLevel : uint8_t { NONE, W0, W1, W2, W3 };
enum class FEmit : uint8_t {
  NONE        = 0,
  bin         = 1ULL << 0,
  llvm        = 1ULL << 1,
  obj         = 1ULL << 2,
  Asm         = 1ULL << 3,
  bc          = 1ULL << 4,
  static_lib  = 1ULL << 5,
  dynamic_lib = 1ULL << 6
};
enum class FPass : uint16_t {
  NONE              = 0,
  filesystem        = 1ULL << 0,
  lexer             = 1ULL << 1,
  preprocessor      = 1ULL << 2,
  parser            = 1ULL << 3,
  binder            = 1ULL << 4,
  shipowner         = 1ULL << 5,
  resolver_symbol   = 1ULL << 6,
  resolver_type     = 1ULL << 7,
  resolver_semantic = 1ULL << 8,
  codegen           = 1ULL << 9,
  optimization      = 1ULL << 10,
  emit              = 1ULL << 11,
  linker            = 1ULL << 12,
  all               = 1ULL << 13,
};

enum class FDebugPrinter : uint8_t { NONE = 0, AST = 1ULL << 0 };

enum class EPlatformFlavor : uint8_t {
  linux_glibc,
  linux_musl,

  apple_macos,
  apple_ios,

  windows_msvc,
  windows_mingw,

  bsd_generic,

  wasi,

  baremetal,

  custom,
  unknown,
};

enum class FCPUFeature : uint16_t {
  NONE = 0,

  sse    = 1ULL << 0,
  sse2   = 1ULL << 1,
  sse3   = 1ULL << 2,
  ssse3  = 1ULL << 3,
  sse4_1 = 1ULL << 4,
  sse4_2 = 1ULL << 5,

  avx    = 1ULL << 6,
  avx2   = 1ULL << 7,
  avx512 = 1ULL << 8,

  neon = 1ULL << 9,
  sve  = 1ULL << 10,

  rvc = 1ULL << 11,
  rvv = 1ULL << 12,

  custom = 1ULL << 13,
};


struct Target {
  env::EArch     arch     = env::EArch::unknown;
  env::EPlatform platform = env::EPlatform::unknown;
  env::EVendor   vendor   = env::EVendor::unknown;
  env::EABI      abi      = env::EABI::unknown;
  std::string    cpu;
  FCPUFeature    features     = FCPUFeature::NONE;
  ECallingConv   calling_conv = ECallingConv::unknown;
  ERelocModel    reloc_model  = ERelocModel::PIC;
  ECodeModel     code_model   = ECodeModel::small;
  FEmit          emits        = FEmit::bin;

  [[nodiscard]] std::string get_target_triple() const noexcept;
  [[nodiscard]] size_t      get_arch_size() const noexcept;
};

struct LLVM {
  bool                     verify_module = true;
  std::vector<const char*> args;
};

struct Clang {
  // e.g. 10.20
  struct LibCVersion {
    int major = 0;
    int minor = 0;

    [[nodiscard]] static LibCVersion parse(std::string_view s) noexcept;
    [[nodiscard]] std::string        print() const noexcept;
  };

  ELibC        libc         = ELibC::unknown;
  LibCVersion  libc_version = {.major = 0, .minor = 0};
  ECStandard   std          = ECStandard::unknown;
  FCSource     c_source;
  EEnvironment env              = EEnvironment::unknown;
  bool         disable_builtins = false;
  bool         strict_aliasing  = false;
  std::string  sysroot;

  std::vector<const char*> args;

  [[nodiscard]] const std::vector<std::string>& generate_preprocessor_args() const noexcept;
};


struct Dir {
  // directories
  std::string current_config_file;
  std::string project  = "./";
  std::string build    = "./build";
  std::string source   = "./src";
  std::string vendor   = "./vendor";
  std::string binding  = "./binding";
  std::string ffi_json = "./binding/ffi_json";
  std::string compiler = "~/velox-compiler";
  std::string stdlib;
  std::string packages;

  [[nodiscard]] std::string get_dir_project() const noexcept;
  [[nodiscard]] std::string get_dir_build() const noexcept;
  [[nodiscard]] std::string get_dir_source() const noexcept;
  [[nodiscard]] std::string get_dir_vendor() const noexcept;
  [[nodiscard]] std::string get_dir_binding() const noexcept;
  [[nodiscard]] std::string get_dir_ffi_json() const noexcept;
  [[nodiscard]] std::string get_dir_compiler() const noexcept;
  [[nodiscard]] std::string get_dir_stdlib() const noexcept;
  [[nodiscard]] std::string get_dir_packages() const noexcept;
  [[nodiscard]] std::string get_preprocess_dir() const noexcept;
  [[nodiscard]] std::string get_debug_graph_dir() const noexcept;
  [[nodiscard]] std::string get_llvmir_dir() const noexcept;
};

struct Profile {
  bool          debug        = false;
  EOptimization optimization = EOptimization::O0;
};

struct Log {
  FPass logs = FPass::NONE;
};

struct Warn {
  FPass      warns = FPass::NONE;
  EWarnLevel level = EWarnLevel::W0;
};

struct Debug {
  FDebugPrinter debugs = FDebugPrinter::NONE;
};

struct Preprocessor {
  std::map<std::string, std::string> defines;
  std::vector<const char*>           undefines;
};

enum class EErrorMode : uint8_t {
  fail_fatal,   // stop on first error
  fail_recover, // try recovering, accumulate errors
};


struct Options {
  [[nodiscard]] static Options read_config(std::string_view path) noexcept;
  [[nodiscard]] bool           write_config(std::string_view path) noexcept;

  std::string project_name;
  std::string sub_config;

  EPlatformFlavor platform_flavor = EPlatformFlavor::unknown;

  std::map<std::string, std::string> PREPROCESSOR_ARGS;

  bool mute = false;

  EErrorMode error_mode = EErrorMode::fail_fatal;

  Target  target;
  Profile profile;
  Log     log;
  Warn    warn;
  Debug   debug;

  Dir dir;

  Preprocessor preprocessor;

  LLVM  llvm;
  Clang clang;


  // sub_configs
  // key, path
  std::map<std::string, std::string> sub_configs;

  bool valid = true;


  [[nodiscard]] std::string get_project_name() const noexcept;

  // object, executable, ...
  // name from .config file name
  [[nodiscard]] std::string_view get_out_name() const noexcept;

  [[nodiscard]] std::string_view get_config_file() const noexcept;

  [[nodiscard]] const std::map<std::string, std::string>& generate_preprocessor_args() const noexcept;

  [[nodiscard]] const std::vector<std::string>& generate_clang_args() const noexcept;

  [[nodiscard]] const std::vector<std::string>& to_args() const noexcept;

  [[nodiscard]] static Options get_preset(EPlatformFlavor platform) noexcept;
  [[nodiscard]] static Options get_current(std::string_view project_name) noexcept;

  [[nodiscard]] static Options invalid() noexcept
  {
    return {};
  }

  [[nodiscard]] bool is_valid() const noexcept
  {
    return valid;
  }

  [[nodiscard]] operator bool() const noexcept
  {
    return is_valid();
  }
};


inline Options OPTIONS = Options::invalid();


// for sub configuration
struct Sub_Compiler_Options : Options {
  enum class EMergeMode : uint8_t { _union, _intersection, _anti_intersection };

  std::string current_config_file;

  EMergeMode COMPILATION_ARGS_merge_mode = EMergeMode::_union;

  // target
  EMergeMode features_merge_mode = EMergeMode::_union;
  EMergeMode emits_merge_mode    = EMergeMode::_union;

  // llvm
  EMergeMode llvm_args_merge_mode = EMergeMode::_union;

  // clang
  EMergeMode clang_c_source_merge_mode  = EMergeMode::_union;
  bool       has_clang_disable_builtins = false;
  bool       has_clang_strict_aliasing  = false;
  EMergeMode clang_args_merge_mode      = EMergeMode::_union;

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

  [[nodiscard]] Options merge_context(const Options& base_ctx) const noexcept;
};

} // namespace common::compiler


template <>
struct magic_enum::customize::enum_range<common::compiler::FCSource> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FCPUFeature> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FEmit> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FPass> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FDebugPrinter> {
  static constexpr bool is_flags = true;
};


namespace common
{
[[nodiscard]] inline std::string_view ECStandard_to_clang_flag(common::compiler::ECStandard v) noexcept
{
  switch (v) {
  case common::compiler::ECStandard::c89:   return "-std=c89";
  case common::compiler::ECStandard::c99:   return "-std=c99";
  case common::compiler::ECStandard::c11:   return "-std=c11";
  case common::compiler::ECStandard::c17:   return "-std=c17";
  case common::compiler::ECStandard::c23:   return "-std=c23";
  case common::compiler::ECStandard::gnu89: return "-std=gnu89";
  case common::compiler::ECStandard::gnu99: return "-std=gnu99";
  case common::compiler::ECStandard::gnu11: return "-std=gnu11";
  case common::compiler::ECStandard::gnu17: return "-std=gnu17";
  case common::compiler::ECStandard::gnu23: return "-std=gnu23";

  default:                                  return "";
  }
}

[[nodiscard]] inline std::string_view EEnvironment_to_clang_flag(common::compiler::EEnvironment v) noexcept
{
  switch (v) {
  case common::compiler::EEnvironment::gnu:       return "-D_GNU_SOURCE";
  case common::compiler::EEnvironment::musl:      return "-D_MUSL_SOURCE";
  case common::compiler::EEnvironment::msvc:      return "-fms-compatibility";
  case common::compiler::EEnvironment::gnuabi:    return "";
  case common::compiler::EEnvironment::mingw:     return "-D__MINGW32__";
  case common::compiler::EEnvironment::darwin:    return "-D_DARWIN_C_SOURCE";
  case common::compiler::EEnvironment::baremetal: return "-ffreestanding";
  case common::compiler::EEnvironment::wasi:      return "-D__wasi__";
  default:                                        return "";
  }
}

[[nodiscard]] inline std::string_view ELibC_to_clang_flag(common::compiler::ELibC v) noexcept
{
  switch (v) {
  case common::compiler::ELibC::glibc:      return "-D__GLIBC__";
  case common::compiler::ELibC::musl:       return "-D__MUSL__";
  case common::compiler::ELibC::libsystem:  return "-D__APPLE__";
  case common::compiler::ELibC::ucrt:       return "-D_UCRT";
  case common::compiler::ELibC::msvcrt:     return "-D_MSVCRT";
  case common::compiler::ELibC::mingw_libc: return "-D__MINGW32__";
  case common::compiler::ELibC::bionic:     return "-D__ANDROID_API__";
  case common::compiler::ELibC::bsd_libc:   return "-D__BSD_VISIBLE";
  default:                                  return "";
  }
}


[[nodiscard]] inline std::vector<std::string> ECSource_to_clang_flag(common::compiler::FCSource v) noexcept
{
  using namespace magic_enum::bitwise_operators;

  std::vector<std::string> out;
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::gnu)) out.emplace_back("-D_GNU_SOURCE");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::bsd)) out.emplace_back("-D_BSD_SOURCE");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::darwin)) out.emplace_back("-D_DARWIN_C_SOURCE");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::posix)) out.emplace_back("-D_POSIX_C_SOURCE=200809L");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::xopen)) out.emplace_back("-D_XOPEN_SOURCE=700");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::linux)) out.emplace_back("-D__linux__");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::android)) out.emplace_back("-D__ANDROID__");
  if (magic_enum::enum_flags_test(v, common::compiler::FCSource::crt_secure_no_warnings))
    out.emplace_back("-D_CRT_SECURE_NO_WARNINGS");
  return out;
}


constexpr std::string_view OPTIONS_TEMPLATE =
    R"(
# main velox toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

# all fields will be stored as define element also


# ======================
# velox-compiler section 
# ======================

# if empty, the workspace file name will be used
project_name        = "%project_name"
# linux_glibc, linux_musl, apple_macos, apple_ios, windows_msvc, windows_mingw, bsd_generic, wasi, baremetal, custom
preset              = "%preset"    
# sub .toml name to apply after this .toml 
sub_config          = "%sub_config"  
    
[target]
# x86_64, x86_32, arm64, arm32, ppc64, ppc32, mips64, mips32, wasm64, wasm32, sparc64, custom ...
arch                = "%target.arch"                
# linux, macos, windows, freebsd, openbsd, netbsd, dragonflybsd, android, ios, solaris, custom ...
os                  = "%target.os"                  
# apple, pc, w64 ...
vendor              = "%target.vendor"              
# sysv, win64, gnu, aapcs, aapcs64, darwin_arm64, msvc_x86, msvc_x64, riscv_ilp32, riscv_lp64, wasm64, wasm32, custom ...
abi                 = "%target.abi"           
# overrides host cpu detection
cpu                 = "%target.cpu"                 
# e.g. "sse,sse2,sse3,ssse3,avx,avx2,avx512,neon,sve,rvc,rvv,custom"
features            = [ 
  %target.features
]
# e.g. sysv, cdecl, stdcall, fastcall, thiscall, win64, aapcs, aapcs_vfp, vectorcall, custom
call_convention     = %target.call_convention
# tiny, small, kernel, medium, large
code_model          = "%target.code_model"          
# pic, static, pie, ropi, rwpi, ropi_rwpi
reloc_model         = "%target.reloc_model"         
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
  %warn.warnings
]
# 1, 2, 3
level               = %warn.level  


[debug]
# ast
debugs = [
  %debug.debugs
]

[preprocessor]
undefines = [
  %preprocessor.undefines
]
[preprocessor.defines]
%preprocessor.defines

[directory]
# all path
project             = "%dir.project"
build               = "%dir.build"  
source              = "%dir.source"  
vendor              = "%dir.vendor"  
ffi_json            = "%dir.ffi_json"  
binding             = "%dir.binding"  
compiler            = "%dir.compiler"  
stdlib              = "%dir.stdlib"  
packages            = "%dir.packages"

[config]
# designed to target a sub configuration parameters
%sub_configs

# ============
# LLVM section
# ============
# This configuration is strictly for LLVM passes and code generation.
# It does not affect your Velox preprocessor.

[llvm]
# verify before AND after passes
verify_module       = %llvm.verify_module     
# custom raw flags passed to LLVM
args = [                                      
  %llvm.args
]

# =============
# Clang section
# =============
# This configuration is strictly for clang passes on C std lib

[clang]
# one: GLIBC, MUSL, LIBSYSTEM, UCRT, MSVCRT, MINGW_LIBC, BIONIC, BSD_LIBC, CUSTOM
libc                = "%clang.libc"
# major.minor format: e.g. 10.2
libc_version        = "%clang.libc_version"
# one: C89, C99, C11, C17, C23, GNU89, GNU99, GNU11, GNU17, GNU23 
std                 = "%clang.std"
# multiple: GNU, BSD, DARWIN, POSIX, XOPEN, LINUX, ANDROID, CRT_SECURE_NO_WARNINGS, CUSTOM
c_source            = [
  %clang.c_source
]
# one: GNU, MUSL, MSVC, GNUABI, MINGW, DARWIN, BAREMETAL, WASI, CUSTOM 
environment         = "%clang.env"
# true, false
disable_builtins    = "%clang.disable_builtins"
# true, false
strict_aliasing     = "%clang.strict_aliasing"
# path
sysroot             = "%clang.sysroot"
# custom raw flags passed to clang
args = [
  %clang.args
]
)";

constexpr std::string_view VELOX_MAIN_TEMPLATE =
    R"(
import ext: C::stdio

fn main() {
  C::printf("hello world!"c_str)
}

)";

constexpr std::string_view VELOX_CORE_TEMPLATE =
    R"(
import ext: C::stdio
import usr: ffi::C
    
export {
  fn println(ref msg: str) {
    C::printf(ffi::C::str_to_cstr(msg))
  }
}

)";

} // namespace common