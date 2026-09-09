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

#include <common/enum_lite.hpp>
#include <common/forward.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace common::compiler
{


DEFINE_ENUM(ERelocModel, uint8_t, //
            STATIC, 1,            //
            PIC, 2,               //
            PIE, 3,               //
            ROPI, 4,              //
            RWPI, 5,              //
            ROPI_RWPI, 6          //
)
DEFINE_ENUM(ECodeModel, uint8_t, //
            tiny, 1,             //
            small, 2,            //
            kernel, 3,           //
            medium, 4,           //
            large, 5             //
)
DEFINE_ENUM(EOptimization, uint8_t, //
            O0, 1,                  //
            O1, 2,                  //
            O2, 3,                  //
            O3, 4,                  //
            Os, 5,                  //
            Oz, 6                   //
)
DEFINE_ENUM(EWarnLevel, uint8_t, //
            W0, 1,               //
            W1, 2,               //
            W2, 3,               //
            W3, 4                //
)

DEFINE_FLAGS(FWarnMode, uint8_t,  //
             all, 1ULL << 0,      //
             extra, 1ULL << 1,    //
             pedantic, 1ULL << 2, //
             unused,
             1ULL << 3,            //
             dead_code, 1ULL << 4, //
             as_error, 1ULL << 5   //
)
DEFINE_ENUM(ELogLevel, uint8_t, //
            quiet, 1,           //
            normal, 2,          //
            verbose, 3          //
)
DEFINE_FLAGS(FEmit, uint8_t,        //
             bin, 1ULL << 0,        //
             llvm, 1ULL << 1,       //
             obj, 1ULL << 2,        //
             Asm, 1ULL << 3,        //
             bc, 1ULL << 4,         //
             static_lib, 1ULL << 5, //
             dynamic_lib, 1ULL << 6 //
)
DEFINE_FLAGS(FPass, uint16_t,              //
             filesystem, 1ULL << 0,        //
             lexer, 1ULL << 1,             //
             preprocessor, 1ULL << 2,      //
             parser, 1ULL << 3,            //
             binder, 1ULL << 4,            //
             shipowner, 1ULL << 5,         //
             resolver_symbol, 1ULL << 6,   //
             resolver_type, 1ULL << 7,     //
             resolver_semantic, 1ULL << 8, //
             codegen, 1ULL << 9,           //
             optimization, 1ULL << 10,     //
             emit, 1ULL << 11,             //
             linker, 1ULL << 12,           //
             all, 1ULL << 13               //
)
DEFINE_FLAGS(FDebugPrinter, uint8_t, //
             AST, 1ULL << 0          //
)
DEFINE_FLAGS(FCPUFeature, uint16_t, //
             sse, 1ULL << 0,        //
             sse2, 1ULL << 1,       //
             sse3, 1ULL << 2,       //
             ssse3, 1ULL << 3,      //
             sse4_1, 1ULL << 4,     //
             sse4_2, 1ULL << 5,     //
             avx, 1ULL << 6,        //
             avx2, 1ULL << 7,       //
             avx512, 1ULL << 8,     //
             neon, 1ULL << 9,       //
             sve, 1ULL << 10,       //
             rvc, 1ULL << 11,       //
             rvv, 1ULL << 12,       //
             custom, 1ULL << 13     //
)

DEFINE_ENUM(EDiagnosticFormat, uint8_t, //
            userfriendly, 1,            //
            json, 2,                    //
            github, 3                   //
)


DEFINE_ENUM(EErrorMode, uint8_t, //
            fail_fatal, 1,       // stop on first error
            fail_recover, 2      // try recovering, accumulate errors
)

struct TargetTriple final {
  env::EArch     arch;
  env::EVendor   vendor;
  env::EPlatform platform;
  env::EABI      abi;

  [[nodiscard]] static TargetTriple parse(std::string_view s) noexcept;
  [[nodiscard]] std::string         dump() const noexcept;
};

struct Target final {
  TargetTriple         triple;
  env::ECallConvention call_convention;

  std::string cpu;
  FCPUFeature features    = FCPUFeature::NONE;
  ERelocModel reloc_model = ERelocModel::PIE;
  ECodeModel  code_model  = ECodeModel::small;
  FEmit       emits       = FEmit::bin;

  [[nodiscard]] size_t get_arch_size() const noexcept;
};

struct LLVM final {
  bool                     verify_module = true;
  std::vector<const char*> args;
};

struct Cffi final {
  // e.g. 10.20
  struct LibCVersion {
    int major = 0;
    int minor = 0;

    [[nodiscard]] static LibCVersion parse(std::string_view s) noexcept;
    [[nodiscard]] std::string        dump() const noexcept;
  };

  env::ELibC        libc;
  LibCVersion       libc_version = {.major = 0, .minor = 0};
  env::ECStandard   std;
  env::FCSource     c_source;
  env::EEnvironment env;
  bool              disable_builtins = false;
  bool              strict_aliasing  = false;
  std::string       sysroot;

  std::vector<const char*> args;

  [[nodiscard]] const std::vector<std::string>& generate_preprocessor_args() const noexcept;
};


struct Dir final {
  Dir(std::string_view p_dir_project)
    : dir_project(p_dir_project)
  {
  }

  // directories
  std::string build    = "./build";
  std::string source   = "./src";
  std::string profile  = "./profile";
  std::string vendor   = "./vendor";
  std::string binding  = "./binding";
  std::string ffi_json = "./binding/ffi_json";
  std::string compiler = "~/tolza-compiler";
  std::string overlay;
  std::string stdlib;
  std::string packages;

  [[nodiscard]] std::string get_dir_build() const noexcept;
  [[nodiscard]] std::string get_dir_source() const noexcept;
  [[nodiscard]] std::string get_dir_profile() const noexcept;
  [[nodiscard]] std::string get_dir_vendor() const noexcept;
  [[nodiscard]] std::string get_dir_binding() const noexcept;
  [[nodiscard]] std::string get_dir_ffi_json() const noexcept;
  [[nodiscard]] std::string get_dir_compiler() const noexcept;
  [[nodiscard]] std::string get_dir_stdlib() const noexcept;
  [[nodiscard]] std::string get_dir_packages() const noexcept;

private:
  std::string dir_project;
};

struct Option_Profile final {
  bool          debug        = false;
  EOptimization optimization = EOptimization::O0;
};

struct Log final {
  FPass     logs   = FPass::NONE;
  ELogLevel level  = ELogLevel::NONE;
  bool      notify = true;
};

struct Warn final {
  FWarnMode  warns = FWarnMode::NONE;
  EWarnLevel level = EWarnLevel::W0;
};

struct Debug final {
  FDebugPrinter debugs = FDebugPrinter::NONE;
};

struct Preprocessor final {
  std::vector<std::pair<std::string, std::string>> defines;
  std::vector<const char*>                         undefines;
};


struct Diagnostic final {
  EErrorMode        error      = EErrorMode::fail_fatal;
  EDiagnosticFormat out_format = EDiagnosticFormat::userfriendly;
};

struct Dependency final {
  std::string dependency;

  [[nodiscard]] std::string name() const
  {
    auto start = dependency.find(':');
    auto end   = dependency.find('@', start);
    return dependency.substr((start == std::string::npos) ? 0 : start + 1,
                             (end == std::string::npos) ? std::string::npos : end - start);
  }

  [[nodiscard]] std::string source() const
  {
    const auto colon = dependency.find(':');
    if (colon == std::string_view::npos) return {};
    return dependency.substr(0, colon);
  }

  [[nodiscard]] std::string version() const
  {
    const auto at = dependency.find('@');
    if (at == std::string_view::npos) return {};
    return dependency.substr(at + 1);
  }
};


struct Manifest {
  [[nodiscard]] static Manifest read_manifest(std::string_view project_manifest) noexcept;
  [[nodiscard]] bool            write_manifest(std::string_view project_manifest, bool use_env = false) noexcept;

  std::string project_name;
  std::string project_path;

  // name
  std::vector<std::string> profiles;
  // name or name@version or origin:name@version
  std::vector<Dependency>  dependencies;

  std::vector<std::pair<std::string, std::string>> PREPROCESSOR_ARGS;

  bool is_check_mode = false;

  Target         target;
  Option_Profile profile;
  Log            log;
  Warn           warn;
  Debug          debug;
  Diagnostic     diagnostic;

  Dir dir = Dir("");

  Preprocessor preprocessor;

  LLVM llvm;
  Cffi c_ffi;

  bool valid = true;


  [[nodiscard]] std::string get_project_name() const noexcept;

  // return path of tolza.toml
  [[nodiscard]] std::string get_project_manifest() const noexcept;
  [[nodiscard]] std::string get_profile_filename() const noexcept;
  [[nodiscard]] std::string get_dir_build_profile() const noexcept;
  [[nodiscard]] std::string get_dir_preprocess() const noexcept;
  [[nodiscard]] std::string get_dir_debug_graph() const noexcept;
  [[nodiscard]] std::string get_dir_llvmir() const noexcept;
  [[nodiscard]] std::string get_dir_binding_profile() const noexcept;

  [[nodiscard]] const std::vector<std::pair<std::string, std::string>>& generate_preprocessor_args() const noexcept;

  [[nodiscard]] const std::vector<std::string>& generate_clang_args() const noexcept;

  [[nodiscard]] const std::vector<std::string>& to_args() const noexcept;

  [[nodiscard]] static Manifest get_preset(const TargetTriple& triple) noexcept;
  [[nodiscard]] static Manifest get_current(std::string_view project_name) noexcept;

  [[nodiscard]] static Manifest invalid() noexcept
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


DEFINE_ENUM(EMergeMode, uint8_t,   //
            _union, 1,             //
            _override, 2,          //
            _intersection, 3,      //
            _anti_intersection, 4, //
)


// for sub configuration
struct Profile : Manifest {
  Profile() = default;
  Profile(const Manifest& manifest)
    : Manifest(manifest)
  {
  }

  [[nodiscard]] static Profile read_profile(std::string_view path, std::string_view project_path) noexcept;
  [[nodiscard]] bool write_profile(std::string_view path, bool use_env = false, bool is_debug = false) noexcept;


  // profile file path
  std::string profile_path;

  EMergeMode COMPILATION_ARGS_merge_mode = EMergeMode::NONE;

  // target
  EMergeMode target_features_merge_mode = EMergeMode::NONE;
  EMergeMode target_emits_merge_mode    = EMergeMode::NONE;


  // c ffi
  EMergeMode c_ffi_c_source_merge_mode  = EMergeMode::NONE;
  bool       has_c_ffi_disable_builtins = false;
  bool       has_c_ffi_strict_aliasing  = false;
  EMergeMode c_ffi_args_merge_mode      = EMergeMode::NONE;

  // profile
  bool has_profile_debug = false;

  // logs
  EMergeMode logs_merge_mode = EMergeMode::NONE;

  // warnings
  EMergeMode warnings_merge_mode = EMergeMode::NONE;

  // debugs
  EMergeMode debug_merge_mode = EMergeMode::NONE;

  // defines
  EMergeMode defines_merge_mode = EMergeMode::NONE;

  // undefines
  EMergeMode undefines_merge_mode = EMergeMode::NONE;

  // llvm
  bool       has_llvm_verify_module = false;
  EMergeMode llvm_args_merge_mode   = EMergeMode::NONE;


  [[nodiscard]] Manifest merge_context(const Manifest& base_ctx) const noexcept;
};


} // namespace common::compiler

namespace common
{

extern std::string MANIFEST_TOML_TEMPLATE;
extern std::string PROFILE_TOML_TEMPLATE;

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

} // namespace common