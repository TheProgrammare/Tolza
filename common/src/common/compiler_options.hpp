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

enum class ERelocModel : uint8_t { NONE, STATIC, PIC, PIE, ROPI, RWPI, ROPI_RWPI };
enum class ECodeModel : uint8_t { NONE, tiny, small, kernel, medium, large };
enum class EOptimization : uint8_t { NONE, O0, O1, O2, O3, Os, Oz };
enum class EWarnLevel : uint8_t { NONE, W0, W1, W2, W3 };
enum class FWarnMode : uint8_t {
  NONE      = 0,
  all       = 1ULL << 0,
  extra     = 1ULL << 1,
  pedantic  = 1ULL << 2,
  unused    = 1ULL << 3,
  dead_code = 1ULL << 4,
  as_error  = 1ULL << 5
};
enum class ELogLevel : uint8_t {
  NONE,
  quiet,
  normal,
  verbose,
};
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

enum class FDebugPrinter : uint8_t {
  NONE = 0,
  AST  = 1ULL << 0,
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

enum class EDiagnosticFormat : uint8_t {
  NONE,
  userfriendly,
  json,
  github,
};


struct TargetTriple final {
  env::EArch     arch     = env::EArch::NONE;
  env::EVendor   vendor   = env::EVendor::NONE;
  env::EPlatform platform = env::EPlatform::NONE;
  env::EABI      abi      = env::EABI::NONE;

  [[nodiscard]] static TargetTriple parse(std::string_view s) noexcept;
  [[nodiscard]] std::string         dump() const noexcept;
};

struct Target final {
  TargetTriple         triple;
  env::ECallConvention call_convention = env::ECallConvention::NONE;

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

  env::ELibC        libc         = env::ELibC::NONE;
  LibCVersion       libc_version = {.major = 0, .minor = 0};
  env::ECStandard   std          = env::ECStandard::NONE;
  env::FCSource     c_source;
  env::EEnvironment env              = env::EEnvironment::NONE;
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
  std::map<std::string, std::string> defines;
  std::vector<const char*>           undefines;
};

enum class EErrorMode : uint8_t {
  NONE,
  fail_fatal,   // stop on first error
  fail_recover, // try recovering, accumulate errors
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

  std::map<std::string, std::string> PREPROCESSOR_ARGS;

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

  [[nodiscard]] const std::map<std::string, std::string>& generate_preprocessor_args() const noexcept;

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


// for sub configuration
struct Profile : Manifest {
  Profile() = default;
  Profile(const Manifest& manifest)
    : Manifest(manifest)
  {
  }

  [[nodiscard]] static Profile read_profile(std::string_view path, std::string_view project_path) noexcept;
  [[nodiscard]] bool write_profile(std::string_view path, bool use_env = false, bool is_debug = false) noexcept;

  enum class EMergeMode : uint8_t { NONE, _union, _override, _intersection, _anti_intersection };

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


template <>
struct magic_enum::customize::enum_range<common::compiler::FCPUFeature> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FEmit> {
  static constexpr bool is_flags = true;
};
template <>
struct magic_enum::customize::enum_range<common::compiler::FWarnMode> {
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


namespace common::compiler
{

#define GET_ENUM_NAMES_TO_STRING(_enum_type)                                                                           \
  ([]() -> std::string {                                                                                               \
    std::string out;                                                                                                   \
    bool        first = true;                                                                                          \
    for (auto name : magic_enum::enum_names<_enum_type>()) {                                                           \
      if (!first) out += ", ";                                                                                         \
      out += name;                                                                                                     \
      first = false;                                                                                                   \
    }                                                                                                                  \
    return out;                                                                                                        \
  }())
#define GET_FLAGS_NAMES_TO_STRING(_flag_type)                                                                          \
  ([]() -> std::string {                                                                                               \
    std::string out;                                                                                                   \
    bool        first = true;                                                                                          \
    for (auto name : magic_enum::enum_names<_flag_type>()) {                                                           \
      if (!first) out += "|";                                                                                          \
      out += name;                                                                                                     \
      first = false;                                                                                                   \
    }                                                                                                                  \
    return out;                                                                                                        \
  }())


constexpr std::string& ERelocModel_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::ERelocModel);
  return s;
}
constexpr std::string& ECodeModel_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::ECodeModel);
  return s;
}
constexpr std::string& EOptimization_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::EOptimization);
  return s;
}
constexpr std::string& ELogLevel_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::ELogLevel);
  return s;
}
constexpr std::string& EWarnLevel_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::EWarnLevel);
  return s;
}
constexpr std::string& FWarnMode_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::compiler::FWarnMode);
  return s;
}
constexpr std::string& FEmit_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::compiler::FEmit);
  return s;
}
constexpr std::string& FPass_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::compiler::FPass);
  return s;
}
constexpr std::string& FCPUFeature_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::compiler::FCPUFeature);
  return s;
}
constexpr std::string& FDebugPrinter_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::compiler::FDebugPrinter);
  return s;
}
constexpr std::string& EErrorMode_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::EErrorMode);
  return s;
}
constexpr std::string& EDiagnosticFormat_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::compiler::EDiagnosticFormat);
  return s;
}


#undef GET_ENUM_NAMES_TO_STRING
#undef GET_FLAGS_NAMES_TO_STRING

} // namespace common::compiler


namespace common
{
[[nodiscard]] inline std::string_view ECStandard_to_clang_flag(common::env::ECStandard v) noexcept
{
  switch (v) {
  case common::env::ECStandard::c89:   return "-std=c89";
  case common::env::ECStandard::c99:   return "-std=c99";
  case common::env::ECStandard::c11:   return "-std=c11";
  case common::env::ECStandard::c17:   return "-std=c17";
  case common::env::ECStandard::c23:   return "-std=c23";
  case common::env::ECStandard::gnu89: return "-std=gnu89";
  case common::env::ECStandard::gnu99: return "-std=gnu99";
  case common::env::ECStandard::gnu11: return "-std=gnu11";
  case common::env::ECStandard::gnu17: return "-std=gnu17";
  case common::env::ECStandard::gnu23: return "-std=gnu23";

  default:                             return {};
  }
}

[[nodiscard]] inline std::string_view EEnvironment_to_clang_flag(common::env::EEnvironment v) noexcept
{
  switch (v) {
  case common::env::EEnvironment::gnu:       return "-D_GNU_SOURCE";
  case common::env::EEnvironment::musl:      return "-D_MUSL_SOURCE";
  case common::env::EEnvironment::msvc:      return "-fms-compatibility";
  case common::env::EEnvironment::gnuabi:    return {};
  case common::env::EEnvironment::mingw:     return "-D__MINGW32__";
  case common::env::EEnvironment::darwin:    return "-D_DARWIN_C_SOURCE";
  case common::env::EEnvironment::baremetal: return "-ffreestanding";
  case common::env::EEnvironment::wasi:      return "-D__wasi__";
  default:                                   return {};
  }
}

[[nodiscard]] inline std::string_view ELibC_to_clang_flag(common::env::ELibC v) noexcept
{
  switch (v) {
  case common::env::ELibC::glibc:      return "-D__GLIBC__";
  case common::env::ELibC::musl:       return "-D__MUSL__";
  case common::env::ELibC::libsystem:  return "-D__APPLE__";
  case common::env::ELibC::ucrt:       return "-D_UCRT";
  case common::env::ELibC::msvcrt:     return "-D_MSVCRT";
  case common::env::ELibC::mingw_libc: return "-D__MINGW32__";
  case common::env::ELibC::bionic:     return "-D__ANDROID_API__";
  case common::env::ELibC::bsd_libc:   return "-D__BSD_VISIBLE";
  default:                             return {};
  }
}


[[nodiscard]] inline std::vector<std::string> ECSource_to_clang_flag(common::env::FCSource v) noexcept
{
  using namespace magic_enum::bitwise_operators;

  std::vector<std::string> out;
  if (magic_enum::enum_flags_test(v, common::env::FCSource::gnu)) out.emplace_back("-D_GNU_SOURCE");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::bsd)) out.emplace_back("-D_BSD_SOURCE");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::darwin)) out.emplace_back("-D_DARWIN_C_SOURCE");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::posix)) out.emplace_back("-D_POSIX_C_SOURCE=200809L");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::xopen)) out.emplace_back("-D_XOPEN_SOURCE=700");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::linux)) out.emplace_back("-D__linux__");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::android)) out.emplace_back("-D__ANDROID__");
  if (magic_enum::enum_flags_test(v, common::env::FCSource::crt_secure_no_warnings))
    out.emplace_back("-D_CRT_SECURE_NO_WARNINGS");
  return out;
}


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