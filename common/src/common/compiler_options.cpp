#include "common/compiler_options.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <common/enum_lite.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/utils.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <initializer_list>
#include <ios>
#include <iterator>
#include <marzer/toml++.hpp>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>


namespace fs = std::filesystem;


DEFINE_FLAGS(FTest, uint8_t,   //
             all, 1ULL << 0,   //
             extra, 1ULL << 1, //
)


size_t common::compiler::Target::get_arch_size() const noexcept
{
  switch (triple.arch) {
  case env::EArch::x86_64:
  case env::EArch::aarch64:
  case env::EArch::riscv64:
  case env::EArch::ppc64:
  case env::EArch::mips64:
  case env::EArch::wasm64:
  case env::EArch::sparc64:
  case env::EArch::NONE:
  case env::EArch::custom:  return 64;
  case env::EArch::x86_32:
  case env::EArch::aarch32:
  case env::EArch::riscv32:
  case env::EArch::ppc32:
  case env::EArch::mips32:
  case env::EArch::wasm32:  return 32;
  }

  // fallback
  return 64;
}

std::string common::compiler::Dir::get_dir_build() const noexcept
{
  return common::fileutils::resolve_path(build, dir_project);
}
std::string common::compiler::Dir::get_dir_source() const noexcept
{
  return common::fileutils::resolve_path(source, dir_project);
}
std::string common::compiler::Dir::get_dir_profile() const noexcept
{
  return common::fileutils::resolve_path(profile, dir_project);
}
std::string common::compiler::Dir::get_dir_vendor() const noexcept
{
  return common::fileutils::resolve_path(vendor, dir_project);
}
std::string common::compiler::Dir::get_dir_binding() const noexcept
{
  return common::fileutils::resolve_path(binding, dir_project);
}
std::string common::compiler::Dir::get_dir_ffi_json() const noexcept
{
  return common::fileutils::resolve_path(ffi_json, dir_project);
}
std::string common::compiler::Dir::get_dir_compiler() const noexcept
{
  return common::fileutils::resolve_path(compiler, dir_project);
}
std::string common::compiler::Dir::get_dir_stdlib() const noexcept
{
  return common::fileutils::resolve_path(stdlib, dir_project);
}
std::string common::compiler::Dir::get_dir_packages() const noexcept
{
  return common::fileutils::resolve_path(packages, dir_project);
}

std::string common::compiler::Manifest::get_dir_preprocess() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "preprocess").string();
}

std::string common::compiler::Manifest::get_dir_debug_graph() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "graph").string();
}

std::string common::compiler::Manifest::get_dir_llvmir() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "llvm-ir").string();
}

std::string common::compiler::Manifest::get_dir_binding_profile() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "binding").string();
}

std::string common::compiler::Manifest::get_project_name() const noexcept
{
  if (project_name.empty()) return fs::path(project_path).stem().string();
  return project_name;
}

std::string common::compiler::Manifest::get_project_manifest() const noexcept
{
  return common::fileutils::resolve_path("tolza.toml", project_path);
}


std::string common::compiler::Manifest::get_profile_filename() const noexcept
{
  static std::string out;
  if (!out.empty()) return out;

  if (project_name.empty())
    out = fs::path(project_path).stem();
  else
    out = project_name;

  for (const auto& elem : profiles) std::format_to(std::back_inserter(out), "-{}", elem);
  return out;
}
std::string common::compiler::Manifest::get_dir_build_profile() const noexcept
{
  return common::fileutils::resolve_path((fs::path(dir.get_dir_build()) / get_profile_filename()).string(),
                                         project_path);
}

common::compiler::TargetTriple common::compiler::TargetTriple::parse(std::string_view s) noexcept
{
  TargetTriple result;

  std::vector<std::string_view> parts;

  size_t start = 0;
  while (start < s.size()) {
    size_t end = s.find('-', start);
    if (end == std::string_view::npos) {
      parts.emplace_back(s.substr(start));
      break;
    }

    parts.emplace_back(s.substr(start, end - start));
    start = end + 1;
  }

  // Cas général : arch-vendor-platform-abi
  result.arch = env::EArch_from_str(std::string(parts[0]));
  if (parts.size() >= 2) result.vendor = env::EVendor_from_str(std::string(parts[1]));
  if (parts.size() >= 3) result.platform = env::EPlatform_from_str(std::string(parts[2]));
  if (parts.size() >= 4) result.abi = env::EABI_from_str(std::string(parts[3]));

  return result;
}

std::string common::compiler::TargetTriple::dump() const noexcept
{
  std::string triple;
  auto        str_arch     = env::EArch_to_str(arch);
  auto        str_vendor   = env::EVendor_to_str(vendor);
  auto        str_platform = env::EPlatform_to_str(platform);
  triple += str_arch.empty() ? "unknown" : str_arch;
  triple += "-";
  triple += str_vendor.empty() ? "unknown" : str_vendor;
  triple += "-";
  triple += str_platform.empty() ? "unknown" : str_platform;
  return triple;
}


const std::vector<std::string>& common::compiler::Cffi::generate_preprocessor_args() const noexcept
{
  static std::vector<std::string> out;
  if (!out.empty()) return out;

  std::set<std::string> args;

  // C standard
  if (auto s = ECStandard_to_clang_flag(std); !s.empty()) args.insert(std::string(s));

  // libc environment
  if (auto e = EEnvironment_to_clang_flag(env); !e.empty()) args.insert(std::string(e));

  // libc defines
  if (auto l = ELibC_to_clang_flag(libc); !l.empty()) args.insert(std::string(l));

  // feature macros list
  for (auto& flag : common::env::ECSource_to_clang_flag(c_source)) args.insert(flag);

  return out = {args.begin(), args.end()};
}
const std::vector<std::string>& common::compiler::Manifest::generate_clang_args() const noexcept
{
  static std::vector<std::string> args;
  if (!args.empty()) return args;

  // target triple override
  if (auto triple = target.triple.dump(); !triple.empty()) args.emplace_back("--target=" + triple);

  // sysroot
  if (!c_ffi.sysroot.empty()) args.emplace_back("--sysroot=" + c_ffi.sysroot);

  // builtin control
  if (c_ffi.disable_builtins) args.emplace_back("-fno-builtin");

  // aliasing
  if (!c_ffi.strict_aliasing) args.emplace_back("-fno-strict-aliasing");

  // append preprocessor args
  auto pp = c_ffi.generate_preprocessor_args();
  args.insert(args.end(), pp.begin(), pp.end());

  return args;
}

const std::vector<std::pair<std::string, std::string>>&
common::compiler::Manifest::generate_preprocessor_args() const noexcept
{
  using Pair = std::pair<std::string, std::string>;
  static std::vector<Pair> args;
  if (!args.empty()) return args;

  auto to_std_arg = [](std::string_view _str) {
    std::string tmp = std::string(_str);
    std::ranges::transform(tmp, tmp.begin(), ::toupper);
    return "__" + tmp + "__";
  };

  auto build_arg = [&](std::string_view _str, std::string_view key) {
    const std::string std_arg = to_std_arg(_str);
    args.emplace_back(std::string(key), std_arg);
    args.emplace_back(std_arg, "true");
  };

  args.emplace_back("project_name", get_project_name());

  build_arg(env::EArch_to_str(target.triple.arch), "target.arch");
  build_arg(env::EPlatform_to_str(target.triple.platform), "target.platform");
  build_arg(env::EVendor_to_str(target.triple.vendor), "target.vendor");
  build_arg(env::EABI_to_str(target.triple.abi), "target.abi");
  build_arg(target.cpu, "target.cpu");
  for (const auto& feature : FCPUFeature_to_vec_str(target.features)) {
    args.emplace_back(std::format("target.feature.{}", feature), "true");
  }
  build_arg(env::ECallConvention_to_str(target.call_convention), "target.calling_convention");
  build_arg(ERelocModel_to_str(target.reloc_model), "target.reloc_model");
  build_arg(ECodeModel_to_str(target.code_model), "target.code_model");
  for (const auto& emit : FEmit_to_vec_str(target.emits)) {
    args.emplace_back(std::format("target.emit.{}", emit), "true");
  }

  args.emplace_back("llvm.verify_module", llvm.verify_module ? "true" : "false");

  build_arg(env::ELibC_to_str(c_ffi.libc), "clang.libc");
  args.emplace_back("clang.libc.version", c_ffi.libc_version.dump());
  build_arg(env::ECStandard_to_str(c_ffi.std), "clang.std");
  build_arg(env::EEnvironment_to_str(c_ffi.env), "clang.env");
  build_arg(std::to_string(c_ffi.disable_builtins), "clang.disable_builtins");
  build_arg(std::to_string(c_ffi.strict_aliasing), "clang.strict_aliasing");
  build_arg(c_ffi.sysroot, "clang.sysroot");

  if (profile.debug) {
    build_arg("debug", "profile.build_mode");
  } else {
    build_arg("release", "profile.build_mode");
  }

  build_arg(EOptimization_to_str(profile.optimization), "optimization");

  for (const auto& log : FPass_to_vec_str(log.logs)) build_arg(log, "log");
  build_arg(ELogLevel_to_str(log.level), "log.level");

  for (const auto& warn : FWarnMode_to_vec_str(warn.warns)) build_arg("true", std::format("warning.{}", warn));
  build_arg(EWarnLevel_to_str(warn.level), "warning.level");

  for (const auto& debug : FDebugPrinter_to_vec_str(debug.debugs)) build_arg("true", std::format("debug.{}", debug));

  for (const auto& [key, val] : preprocessor.defines) args.emplace_back(key, val);

  for (const auto& undef : preprocessor.undefines) args.emplace_back(undef, "true");

  return args;
}

const std::vector<std::string>& common::compiler::Manifest::to_args() const noexcept
{
  static std::vector<std::string> out;
  if (!out.empty()) return out;
  std::vector<std::string> llvm_out;

  out.emplace_back(std::format(R"(--project-name="{}")", project_name));
  if (is_check_mode) out.emplace_back("--check");

  // target
  if (target.triple.arch != env::EArch::NONE)
    out.emplace_back(std::format(R"(--arch="{}")", env::EArch_to_str(target.triple.arch)));
  if (target.triple.platform != env::EPlatform::NONE)
    out.emplace_back(std::format(R"(--platform="{}")", env::EPlatform_to_str(target.triple.platform)));
  if (target.triple.vendor != env::EVendor::NONE)
    out.emplace_back(std::format(R"(--vendor="{}")", env::EVendor_to_str(target.triple.vendor)));
  if (target.triple.abi != env::EABI::NONE)
    out.emplace_back(std::format(R"(--abi="{}")", env::EABI_to_str(target.triple.abi)));
  if (!target.cpu.empty()) out.emplace_back(std::format(R"(--cpu="{}")", target.cpu));
  if (target.features != FCPUFeature::NONE)
    out.emplace_back(std::format(R"(--features="{}")", FCPUFeature_to_str(target.features)));
  if (target.call_convention != env::ECallConvention::NONE)
    out.emplace_back(std::format(R"(--calling_convention="{}")", env::ECallConvention_to_str(target.call_convention)));
  if (target.reloc_model != ERelocModel::NONE)
    out.emplace_back(std::format(R"(--reloc-model="{}")", ERelocModel_to_str(target.reloc_model)));
  if (target.code_model != ECodeModel::NONE)
    out.emplace_back(std::format(R"(--code-model="{}")", ECodeModel_to_str(target.code_model)));
  if (target.emits != FEmit::NONE) out.emplace_back(std::format(R"(--emits="{}")", FEmit_to_str(target.emits)));

  if (c_ffi.libc != env::ELibC::NONE) out.emplace_back(std::format(R"(--libc="{}")", env::ELibC_to_str(c_ffi.libc)));

  if (!profiles.empty()) {
    std::string _profile;
    for (const auto& elem : profiles) std::format_to(std::back_inserter(_profile), "{}|", elem);
    _profile = _profile.substr(0, _profile.size() - 1);
    out.emplace_back(std::format(R"(--profile="{}")", _profile));
  }

  out.emplace_back(profile.debug ? "--debug" : "");
  if (profile.optimization != EOptimization::NONE)
    out.emplace_back(std::format("-", EOptimization_to_str(profile.optimization)));
  if (log.logs != FPass::NONE) out.emplace_back(std::format(R"(--logs="{}")", FPass_to_str(log.logs)));
  if (log.level != ELogLevel::NONE) out.emplace_back(std::format(R"(--log-level="{}")", ELogLevel_to_str(log.level)));
  if (warn.warns != FWarnMode::NONE) out.emplace_back(std::format(R"(--warns="{}")", FWarnMode_to_str(warn.warns)));
  if (warn.level != EWarnLevel::NONE) out.emplace_back(std::format("-", EWarnLevel_to_str(warn.level)));
  if (debug.debugs != FDebugPrinter::NONE)
    out.emplace_back(std::format(R"(--debugs="{}")", FDebugPrinter_to_str(debug.debugs)));

  for (const auto& [name, val] : preprocessor.defines) out.emplace_back("-D" + name + "=" + val);

  for (const auto& udef : preprocessor.undefines) out.emplace_back("-U" + std::string(udef));

  if (!dir.overlay.empty()) out.emplace_back(std::format(R"(--overlay="{}")", dir.overlay));
  if (!dir.build.empty()) out.emplace_back(std::format(R"(--dir-build="{}")", dir.build));
  if (!dir.source.empty()) out.emplace_back(std::format(R"(--dir-src="{}")", dir.source));
  if (!dir.profile.empty()) out.emplace_back(std::format(R"(--dir-profile="{}")", dir.profile));
  if (!dir.vendor.empty()) out.emplace_back(std::format(R"(--dir-vendor="{}")", dir.vendor));
  if (!dir.ffi_json.empty()) out.emplace_back(std::format(R"(--dir-ffi-json="{}")", dir.ffi_json));
  if (!dir.binding.empty()) out.emplace_back(std::format(R"(--dir-binding="{}")", dir.binding));
  if (!dir.compiler.empty()) out.emplace_back(std::format(R"(--dir-compiler="{}")", dir.compiler));
  if (!dir.stdlib.empty()) out.emplace_back(std::format(R"(--dir-stdlib="{}")", dir.stdlib));
  if (!dir.packages.empty()) out.emplace_back(std::format(R"(--dir-packages="{}")", dir.packages));

  if (diagnostic.error != EErrorMode::NONE)
    out.emplace_back(std::format(R"(--error-mode="{}")", EErrorMode_to_str(diagnostic.error)));
  if (diagnostic.out_format != EDiagnosticFormat::NONE)
    out.emplace_back(std::format(R"(--diagnostic-format="{}")", EDiagnosticFormat_to_str(diagnostic.out_format)));

  if (llvm.verify_module) out.emplace_back("--verify-module");

  if (!llvm.args.empty()) out.emplace_back("--llvm");
  for (const auto& llvm_arg : llvm.args) out.emplace_back(llvm_arg);

  if (!c_ffi.args.empty()) out.emplace_back("--clang");
  for (const auto& clang_arg : c_ffi.args) out.emplace_back(clang_arg);

  return out;
}

common::compiler::Cffi::LibCVersion common::compiler::Cffi::LibCVersion::parse(std::string_view s) noexcept
{
  LibCVersion v{};

  auto dot = s.find('.');
  if (dot == std::string_view::npos) {
    // Format invalide → on considère tout comme major
    std::from_chars(s.data(), s.data() + s.size(), v.major);
    return v;
  }

  // parse major
  std::from_chars(s.data(), s.data() + dot, v.major);

  // parse minor
  std::string_view minor_part = s.substr(dot + 1);
  std::from_chars(minor_part.data(), minor_part.data() + minor_part.size(), v.minor);

  return v;
}
std::string common::compiler::Cffi::LibCVersion::dump() const noexcept
{
  return (major == 0 && minor == 0) ? "" : std::format("{}.{}", major, minor);
}


common::compiler::Manifest common::compiler::Manifest::get_current(std::string_view name) noexcept
{
  return {
      .project_name = std::string(name),
      .target{
              .triple{
              .arch     = common::env::PlatformDetection::ARCH,
              .vendor   = common::env::PlatformDetection::VENDOR,
              .platform = common::env::PlatformDetection::PLATFORM,
              .abi      = common::env::PlatformDetection::ABI,
          }, .call_convention = common::env::PlatformDetection::CALL_CONVENTION,

              },
      .c_ffi{
              .std      = common::env::PlatformDetection::C_STANDARD,
              .c_source = common::env::PlatformDetection::C_SOURCE,
              .env      = common::env::PlatformDetection::ENVIRONMENT,
              }
  };
}

common::compiler::Manifest common::compiler::Manifest::get_preset(const TargetTriple& triple) noexcept
{
  Manifest options{
      .target{
              .triple{
              .platform = triple.platform,
              .abi      = triple.abi,
          }, .call_convention = env::call_convention(triple.platform, triple.arch, triple.abi),
              .reloc_model     = ERelocModel::PIE,
              .code_model      = ECodeModel::small,
              },
  };


  switch (triple.platform) {

  case env::EPlatform::_linux:
    options.c_ffi = {
        .libc = triple.abi == env::EABI::gnu ? env::ELibC::glibc : env::ELibC::musl,

        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::_gnu,
        .env              = env::EEnvironment::gnu,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::_macos:
  case env::EPlatform::_ios:
    options.c_ffi = {
        .libc             = env::ELibC::libsystem,
        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::_darwin,
        .env              = env::EEnvironment::darwin,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::_windows:
    options.c_ffi = {
        .libc             = env::ELibC::ucrt,
        .std              = env::ECStandard::c17,
        .c_source         = env::FCSource::_crt_secure_no_warnings,
        .env              = env::EEnvironment::msvc,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    options.target.reloc_model = ERelocModel::STATIC;
    return options;


  case env::EPlatform::_freebsd:
  case env::EPlatform::_openbsd:
  case env::EPlatform::_netbsd:
  case env::EPlatform::_dragonflybsd:
    options.c_ffi = {
        .libc             = env::ELibC::bsd_libc,
        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::_bsd,
        .env              = env::EEnvironment::gnu,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::NONE:
  case env::EPlatform::_android:
  case env::EPlatform::_solaris:
  default:                       break;
  }


  // WASI / WebAssembly
  if (triple.abi == env::EABI::wasm32 || triple.arch == env::EArch::wasm32) {
    options.target.reloc_model = ERelocModel::STATIC;

    options.c_ffi = {
        .libc             = env::ELibC::NONE,
        .std              = env::ECStandard::c17,
        .c_source         = env::FCSource::NONE,
        .env              = env::EEnvironment::wasi,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;
  }


  // Bare metal or unknown
  options.target = {
      .triple{
              .platform = triple.platform,
              .abi      = triple.abi,
              },
      .call_convention = env::call_convention(triple.platform, triple.arch, triple.abi),
      .reloc_model     = ERelocModel::STATIC,
      .code_model      = ECodeModel::small,
  };

  options.c_ffi = {
      .libc             = env::ELibC::NONE,
      .std              = env::ECStandard::c17,
      .c_source         = env::FCSource::NONE,
      .env              = env::EEnvironment::baremetal,
      .disable_builtins = true,
      .strict_aliasing  = false,
  };

  return options;
}

template <typename Enum>
void merge_bitwise(Enum& _dest, Enum _val, common::compiler::EMergeMode mode) noexcept
{
  using _underlying = std::underlying_type_t<Enum>;

  static_assert(std::is_enum_v<Enum>, "merge_bitwise: must be an enum");

  const auto  d = static_cast<_underlying>(_dest);
  const auto  v = static_cast<_underlying>(_val);
  _underlying out;

  switch (mode) {
  case common::compiler::EMergeMode::_override: out = d; break;
  case common::compiler::EMergeMode::NONE:
  case common::compiler::EMergeMode::_union:    {
    out = d | v;
    break;
  }
  case common::compiler::EMergeMode::_intersection: {
    out = d & v;
    break;
  }
  case common::compiler::EMergeMode::_anti_intersection: {
    out = d & ~v;
    break;
  }
  }

  _dest = static_cast<Enum>(out);
}

common::compiler::Manifest common::compiler::Profile::merge_context(const Manifest& base_ctx) const noexcept
{
  auto apply_str = [&](std::string& _dest, std::string_view _str) {
    if (_str.empty()) return;
    _dest = _str;
  };

  auto apply_bool = [&](bool& _dest, bool _val, bool _can_apply) {
    if (!_can_apply) return;
    _dest = _val;
  };

  auto apply_int = [&](size_t& _dest, size_t _val, bool _can_apply) {
    if (!_can_apply) return;
    _dest = _val;
  };

  Manifest out = base_ctx;

  utils::merge_list_str(out.profiles, profiles, COMPILATION_ARGS_merge_mode);

  utils::merge_map(out.PREPROCESSOR_ARGS, PREPROCESSOR_ARGS, COMPILATION_ARGS_merge_mode);

  apply_str(out.project_name, project_name);

  // target
  if (target.triple.arch != env::EArch::NONE) out.target.triple.arch = target.triple.arch;
  if (target.triple.platform != env::EPlatform::NONE) out.target.triple.platform = target.triple.platform;
  if (target.triple.vendor != env::EVendor::NONE) out.target.triple.vendor = target.triple.vendor;
  if (target.triple.abi != env::EABI::NONE) out.target.triple.abi = target.triple.abi;
  apply_str(out.target.cpu, target.cpu);
  merge_bitwise(out.target.features, target.features, target_features_merge_mode);
  if (target.call_convention != env::ECallConvention::NONE) out.target.call_convention = target.call_convention;
  if (target.reloc_model != ERelocModel::NONE) out.target.triple.abi = target.triple.abi;
  if (target.code_model != ECodeModel::NONE) out.target.code_model = target.code_model;
  merge_bitwise(out.target.emits, target.emits, target_emits_merge_mode);

  // profile
  apply_bool(out.profile.debug, profile.debug, has_profile_debug);
  if (profile.optimization != EOptimization::NONE) out.profile.optimization = profile.optimization;

  // logs
  merge_bitwise(out.log.logs, log.logs, logs_merge_mode);
  if (log.level != ELogLevel::NONE) out.log.level = log.level;

  // warns
  merge_bitwise(out.warn.warns, warn.warns, warnings_merge_mode);
  if (warn.level != EWarnLevel::NONE) out.warn.level = warn.level;

  // debugs
  merge_bitwise(out.debug.debugs, debug.debugs, debug_merge_mode);

  // diagnostic
  if (diagnostic.error != EErrorMode::NONE) out.diagnostic.error = diagnostic.error;
  if (diagnostic.out_format != EDiagnosticFormat::NONE) out.diagnostic.out_format = diagnostic.out_format;

  // dirs
  apply_str(out.dir.overlay, dir.overlay);
  apply_str(out.dir.build, dir.build);
  apply_str(out.dir.source, dir.source);
  apply_str(out.dir.profile, dir.profile);
  apply_str(out.dir.vendor, dir.vendor);
  apply_str(out.dir.binding, dir.binding);
  apply_str(out.dir.ffi_json, dir.ffi_json);
  apply_str(out.dir.compiler, dir.compiler);
  apply_str(out.dir.stdlib, dir.stdlib);
  apply_str(out.dir.packages, dir.packages);

  // preprocessor
  utils::merge_map(out.preprocessor.defines, preprocessor.defines, defines_merge_mode);
  utils::merge_list_cstr(out.preprocessor.undefines, preprocessor.undefines, undefines_merge_mode);

  // llvm
  apply_bool(out.llvm.verify_module, llvm.verify_module, has_llvm_verify_module);
  utils::merge_list_cstr(out.llvm.args, llvm.args, llvm_args_merge_mode);

  if (c_ffi.libc != env::ELibC::NONE) out.c_ffi.libc = c_ffi.libc;
  if (c_ffi.libc_version.minor != 0 && c_ffi.libc_version.major != 0) out.c_ffi.libc_version = c_ffi.libc_version;
  if (c_ffi.std != env::ECStandard::NONE) out.c_ffi.std = c_ffi.std;
  merge_bitwise(out.c_ffi.c_source, c_ffi.c_source, c_ffi_c_source_merge_mode);
  if (c_ffi.env != env::EEnvironment::NONE) out.c_ffi.env = c_ffi.env;
  apply_bool(out.c_ffi.disable_builtins, c_ffi.disable_builtins, has_c_ffi_disable_builtins);
  apply_bool(out.c_ffi.strict_aliasing, c_ffi.strict_aliasing, has_c_ffi_strict_aliasing);
  apply_str(out.c_ffi.sysroot, c_ffi.sysroot);
  utils::merge_list_cstr(out.c_ffi.args, c_ffi.args, c_ffi_args_merge_mode);

  return out;
}

common::compiler::Manifest common::compiler::Manifest::read_manifest(std::string_view project_manifest) noexcept
{
#define get_enum(_path, _enum_kind) _enum_kind##_from_str(tbl.at_path(_path).value_or(""))

#define get_flags(_path, _flag_kind) _flag_kind##_from_str(tbl.at_path(_path).value_or(""))

#define get_str(_path) tbl.at_path(_path).value_or("")

#define get_bool(_path) tbl.at_path(_path).value_or(false)

  auto is_same_key = [](std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
  };

  auto out = common::compiler::Manifest::invalid();

  toml::table tbl;
  tbl = toml::parse_file(project_manifest);

  out.project_name = get_str("project_name");
  out.project_path = fs::path(project_manifest).parent_path().string();
  out.dir          = Dir(out.project_path);

  {
    std::stringstream ss(get_str("profiles"));
    std::string       item;

    while (std::getline(ss, item, '|')) out.profiles.push_back(item);
  }

  // target
  out.target.triple.arch     = get_enum("target.arch", env::EArch);
  out.target.triple.platform = get_enum("target.platform", env::EPlatform);
  out.target.triple.vendor   = get_enum("target.vendor", env::EVendor);
  out.target.triple.abi      = get_enum("target.abi", env::EABI);
  out.target.cpu             = get_str("target.cpu");
  out.target.features        = get_flags("target.features", FCPUFeature);
  out.target.reloc_model     = get_enum("target.reloc_model", ERelocModel);
  if (out.target.reloc_model == ERelocModel::NONE) out.target.reloc_model = ERelocModel::PIE;
  out.target.code_model = get_enum("target.code_model", ECodeModel);
  if (out.target.code_model == ECodeModel::NONE) out.target.code_model = ECodeModel::small;
  out.target.emits = get_flags("target.emits", FEmit);
  if (out.target.emits == FEmit::NONE) out.target.emits = FEmit::bin;

  // profile
  out.profile.debug        = get_bool("profile.debug");
  out.profile.optimization = get_enum("profile.optimization", EOptimization);
  if (out.profile.optimization == EOptimization::NONE) out.profile.optimization = EOptimization::O0;

  // log
  out.log.logs  = get_flags("log.logs", FPass);
  out.log.level = get_enum("log.level", ELogLevel);

  // warn
  out.warn.warns = get_flags("warning.warnings", FWarnMode);
  out.warn.level = get_enum("warning.level", EWarnLevel);
  if (out.warn.level == EWarnLevel::NONE) out.warn.level = EWarnLevel::W0;

  // diagnostic
  out.diagnostic.error = get_enum("diagnostic.error", EErrorMode);
  if (out.diagnostic.error == EErrorMode::NONE) out.diagnostic.error = EErrorMode::fail_fatal;
  out.diagnostic.out_format = get_enum("diagnostic.out_format", EDiagnosticFormat);
  if (out.diagnostic.out_format == EDiagnosticFormat::NONE) out.diagnostic.out_format = EDiagnosticFormat::userfriendly;

  // llvm
  out.llvm.verify_module = get_bool("llvm.verify_module");
  if (const toml::array* llvm_args = tbl.at_path("llvm.args").as_array()) {
    for (const auto& key : *llvm_args) out.llvm.args.emplace_back(key.value_or(""));
  }

  // directories
  out.dir.build    = common::fileutils::resolve_path(get_str("directory.build"), out.project_path);
  out.dir.source   = common::fileutils::resolve_path(get_str("directory.source"), out.project_path);
  out.dir.profile  = common::fileutils::resolve_path(get_str("directory.profile"), out.project_path);
  out.dir.vendor   = common::fileutils::resolve_path(get_str("directory.vendor"), out.project_path);
  out.dir.binding  = common::fileutils::resolve_path(get_str("directory.binding"), out.project_path);
  out.dir.ffi_json = common::fileutils::resolve_path(get_str("directory.ffi_json"), out.project_path);
  out.dir.compiler = common::fileutils::resolve_path(get_str("directory.compiler"), out.project_path);
  out.dir.stdlib   = common::fileutils::resolve_path(get_str("directory.stdlib"), out.project_path);
  out.dir.packages = common::fileutils::resolve_path(get_str("directory.packages"), out.project_path);

  if (const toml::table* defines = tbl.at_path("preprocessor.defines").as_table()) {
    for (const auto& [key, val] : *defines)
      out.preprocessor.defines.emplace_back(std::string(key.str()), val.value_or(""));
  }

  if (const toml::array* undefines = tbl.at_path("preprocessor.undefines").as_array()) {
    for (const auto& undef : *undefines) out.preprocessor.undefines.emplace_back(undef.value_or(""));
  }

  if (const toml::array* clang_args = tbl.at_path("c_ffi.args").as_array()) {
    for (const auto& key : *clang_args) out.c_ffi.args.emplace_back(key.value_or(""));
  }

  out.c_ffi.libc             = get_enum("c_ffi.libc", env::ELibC);
  out.c_ffi.libc_version     = Cffi::LibCVersion::parse(get_str("c_ffi.libc_version"));
  out.c_ffi.std              = get_enum("c_ffi.std", env::ECStandard);
  out.c_ffi.c_source         = get_enum("c_ffi.c_source", env::FCSource);
  out.c_ffi.env              = get_enum("c_ffi.environment", env::EEnvironment);
  out.c_ffi.disable_builtins = get_str("c_ffi.disable_builtins");
  out.c_ffi.strict_aliasing  = get_str("c_ffi.strict_aliasing");
  out.c_ffi.sysroot          = get_str("c_ffi.sysroot");

  return out;

#undef get_enum
#undef get_flags
#undef get_str
#undef get_bool
}

bool common::compiler::Manifest::write_manifest(std::string_view project_manifest, bool use_env) noexcept
{
  auto btos = [](bool _in) -> std::string { return _in ? "true" : "false"; };

  std::string txt(common::MANIFEST_TOML_TEMPLATE);

  std::string _defines;
  for (auto& [key, val] : preprocessor.defines) std::format_to(std::back_inserter(_defines), "= \"{}\"\n", val);
  std::string _undefines;
  for (auto& val : preprocessor.undefines) std::format_to(std::back_inserter(_undefines), "\"{}\"\n", val);
  std::string _profiles;
  for (auto& elem : profiles) std::format_to(std::back_inserter(_profiles), "{}|", elem);
  _profiles = _profiles.substr(0, _profiles.size() - 1);
  std::string _dependencies;
  for (auto& elem : dependencies) std::format_to(std::back_inserter(_dependencies), "\"{}\",\n", elem.dependency);

  std::string _llvm_args;
  for (auto& val : llvm.args) std::format_to(std::back_inserter(_llvm_args), "\"{}\",\n", val);
  std::string _clang_args;
  for (auto& val : c_ffi.args) std::format_to(std::back_inserter(_clang_args), "\"{}\",\n", val);

  const std::string _libc_version = c_ffi.libc_version.dump();

  const auto _c_source = env::FCSource_to_str(c_ffi.c_source);

  std::initializer_list<std::pair<std::string, std::string>> params = {
      // project
      {"project_name",           project_name                                                                   },
      // target
      {"profiles",               _profiles                                                                      },
      {"dependencies",           _dependencies                                                                  },
      {"target.arch",            use_env ? std::string(env::EArch_to_str(target.triple.arch)) : ""              },
      {"target.platform",        use_env ? std::string(env::EPlatform_to_str(target.triple.platform)) : ""      },
      {"target.vendor",          use_env ? std::string(env::EVendor_to_str(target.triple.vendor)) : ""          },
      {"target.abi",             use_env ? std::string(env::EABI_to_str(target.triple.abi)) : ""                },
      {"target.call_convention", use_env ? std::string(env::ECallConvention_to_str(target.call_convention)) : ""},
      {"target.cpu",             use_env ? target.cpu : ""                                                      },
      {"target.features",        use_env ? FCPUFeature_to_str(target.features) : ""                             },
      {"target.code_model",      use_env ? std::string(ECodeModel_to_str(target.code_model)) : ""               },
      {"target.reloc_model",     use_env ? std::string(ERelocModel_to_str(target.reloc_model)) : ""             },
      {"target.emits",           FEmit_to_str(target.emits)                                                     },
      // profile
      {"profile.debug",          "true"                                                                         },
      {"profile.optimization",   std::string(EOptimization_to_str(EOptimization::O0))                           },
      // logs
      {"log.logs",               FPass_to_str(FPass::all)                                                       },
      {"log.level",              std::string(ELogLevel_to_str(ELogLevel::normal))                               },
      // warnings
      {"warn.warnings",          FWarnMode_to_str(warn.warns)                                                   },
      {"warn.level",             std::string(EWarnLevel_to_str(warn.level).substr(1))                           },
      {"diagnostic.error.mode",  std::string(EErrorMode_to_str(diagnostic.error))                               },
      {"diagnostic.out.format",  std::string(EDiagnosticFormat_to_str(diagnostic.out_format))                   },
      // debug printer
      {"debug.debugs",           FDebugPrinter_to_str(debug.debugs)                                             },
      // defines
      {"preprocessor.defines",   use_env ? _defines : ""                                                        },
      // undefines
      {"preprocessor.undefines", use_env ? _undefines : ""                                                      },
      // directories
      {"dir.build",              dir.build                                                                      },
      {"dir.source",             dir.source                                                                     },
      {"dir.profile",            dir.profile                                                                    },
      {"dir.vendor",             dir.vendor                                                                     },
      {"dir.ffi_json",           dir.ffi_json                                                                   },
      {"dir.binding",            dir.binding                                                                    },
      {"dir.compiler",           std::string(common::env::get_compilers_dir())                                  },
      {"dir.stdlib",             std::string(common::env::get_stdlib_dir())                                     },
      {"dir.packages",           std::string(common::env::get_packages_dir())                                   },
      // llvm
      {"llvm.verify_module",     use_env ? btos(llvm.verify_module) : "true"                                    },
      {"llvm.args",              use_env ? _llvm_args : ""                                                      },
      // c_ffi
      {"c_ffi.libc_version",     use_env ? _libc_version : ""                                                   },
      {"c_ffi.libc",             use_env ? std::string(env::ELibC_to_str(c_ffi.libc)) : ""                      },
      {"c_ffi.std",              use_env ? std::string(env::ECStandard_to_str(c_ffi.std)) : ""                  },
      {"c_ffi.c_source",         use_env ? _c_source : ""                                                       },
      {"c_ffi.environment",      use_env ? std::string(env::EEnvironment_to_str(c_ffi.env)) : ""                },
      {"c_ffi.disable_builtins", use_env ? btos(c_ffi.disable_builtins) : "false"                               },
      {"c_ffi.strict_aliasing",  use_env ? btos(c_ffi.strict_aliasing) : "false"                                },
      {"c_ffi.sysroot",          use_env ? c_ffi.sysroot : ""                                                   },
      {"c_ffi.args",             use_env ? _clang_args : ""                                                     },
  };

  common::utils::fmt_template(txt, params);

  try {
    std::ofstream f = std::ofstream(std::string(project_manifest), std::ios::binary);
    if (!f) return false;
    f.write(txt.data(), static_cast<std::streamsize>(txt.size()));
    if (!f) return false;
    f.close();
  } catch (...) {
    return false;
  }

  return true;
}

common::compiler::Profile common::compiler::Profile::read_profile(std::string_view profile_path,
                                                                  std::string_view project_path) noexcept
{
#define get_enum(_path, _enum_kind) _enum_kind##_from_str(tbl.at_path(_path).value_or(""))

#define get_flags(_path, _flag_kind) _flag_kind##_from_str(tbl.at_path(_path).value_or(""))

#define get_str(_path) tbl.at_path(_path).value_or("")

#define get_bool(_path) tbl.at_path(_path).value_or(false)

#define has(_path) tbl.contains(_path)

  auto is_same_key = [](std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
  };

  auto out = common::compiler::Profile(common::compiler::Manifest{});

  toml::table tbl;
  tbl = toml::parse_file(profile_path);

  {
    std::stringstream ss(get_str("profiles"));
    std::string       item;

    while (std::getline(ss, item, '|')) out.profiles.push_back(item);
  }

  out.COMPILATION_ARGS_merge_mode = get_enum("compilation_args_mode", EMergeMode);

  // target
  out.target.triple.arch = get_enum("target.arch", env::EArch);

  out.target.triple.vendor       = get_enum("target.vendor", env::EVendor);
  out.target.triple.platform     = get_enum("target.platform", env::EPlatform);
  out.target.triple.abi          = get_enum("target.abi", env::EABI);
  out.target.cpu                 = get_str("target.cpu");
  out.target.features            = get_flags("target.features", FCPUFeature);
  out.target_features_merge_mode = get_enum("target.features_mode", EMergeMode);
  out.target.reloc_model         = get_enum("target.reloc_model", ERelocModel);
  out.target.code_model          = get_enum("target.code_model", ECodeModel);
  out.target.emits               = get_flags("target.emits", FEmit);
  out.target_emits_merge_mode    = get_enum("target.emits_mode", EMergeMode);

  // profile
  out.profile.debug        = get_bool("profile.debug");
  out.has_profile_debug    = has("profile.debug");
  out.profile.optimization = get_enum("profile.optimization", EOptimization);

  // log
  out.log.logs        = get_flags("log.logs", FPass);
  out.logs_merge_mode = get_enum("log.logs_mode", EMergeMode);
  out.log.level       = get_enum("log.level", ELogLevel);

  // warn
  out.warn.warns          = get_flags("warning.warnings", FWarnMode);
  out.warnings_merge_mode = get_enum("warning.warnings_mode", EMergeMode);
  out.warn.level          = get_enum("warning.level", EWarnLevel);

  // diagnostic
  out.diagnostic.error      = get_enum("diagnostic.error.mode", EErrorMode);
  out.diagnostic.out_format = get_enum("diagnostic.out.format", EDiagnosticFormat);

  // llvm
  out.llvm.verify_module     = get_bool("llvm.verify_module");
  out.has_llvm_verify_module = has("llvm.verify_module");
  if (const toml::array* llvm_args = tbl.at_path("llvm.args").as_array()) {
    for (const auto& key : *llvm_args) out.llvm.args.emplace_back(key.value_or(""));
  }
  out.llvm_args_merge_mode = get_enum("llvm.args_mode", EMergeMode);

  // directories
  out.dir.build    = common::fileutils::resolve_path(get_str("directory.build"), project_path);
  out.dir.source   = common::fileutils::resolve_path(get_str("directory.source"), project_path);
  out.dir.profile  = common::fileutils::resolve_path(get_str("directory.profile"), project_path);
  out.dir.vendor   = common::fileutils::resolve_path(get_str("directory.vendor"), project_path);
  out.dir.binding  = common::fileutils::resolve_path(get_str("directory.binding"), project_path);
  out.dir.ffi_json = common::fileutils::resolve_path(get_str("directory.ffi_json"), project_path);
  out.dir.compiler = common::fileutils::resolve_path(get_str("directory.compiler"), project_path);
  out.dir.stdlib   = common::fileutils::resolve_path(get_str("directory.stdlib"), project_path);
  out.dir.packages = common::fileutils::resolve_path(get_str("directory.packages"), project_path);

  if (const toml::table* defines = tbl.at_path("preprocessor.defines").as_table()) {
    for (const auto& [key, val] : *defines)
      out.preprocessor.defines.emplace_back(std::string(key.str()), val.value_or(""));
  }
  out.defines_merge_mode = get_enum("preprocessor.defines_mode", EMergeMode);

  if (const toml::array* undefines = tbl.at_path("preprocessor.undefines").as_array()) {
    for (const auto& undef : *undefines) out.preprocessor.undefines.emplace_back(undef.value_or(""));
  }
  out.undefines_merge_mode = get_enum("preprocessor.undefines_mode", EMergeMode);

  if (const toml::array* clang_args = tbl.at_path("c_ffi.args").as_array()) {
    for (const auto& key : *clang_args) out.c_ffi.args.emplace_back(key.value_or(""));
  }
  out.c_ffi_args_merge_mode = get_enum("c_ffi.args_mode", EMergeMode);

  out.c_ffi.libc                 = get_enum("c_ffi.libc", env::ELibC);
  out.c_ffi.libc_version         = Cffi::LibCVersion::parse(get_str("c_ffi.libc_version"));
  out.c_ffi.std                  = get_enum("c_ffi.std", env::ECStandard);
  out.c_ffi.c_source             = get_enum("c_ffi.c_source", env::FCSource);
  out.c_ffi_c_source_merge_mode  = get_enum("c_ffi.c_source_mode", EMergeMode);
  out.c_ffi.env                  = get_enum("c_ffi.environment", env::EEnvironment);
  out.c_ffi.disable_builtins     = get_str("c_ffi.disable_builtins");
  out.has_c_ffi_disable_builtins = has("c_ffi.disable_builtins");
  out.c_ffi.strict_aliasing      = get_str("c_ffi.strict_aliasing");
  out.has_c_ffi_strict_aliasing  = has("c_ffi.strict_aliasing");
  out.c_ffi.sysroot              = get_str("c_ffi.sysroot");

  return out;

#undef get_enum
#undef get_flags
#undef get_str
#undef get_bool
}
bool common::compiler::Profile::write_profile(std::string_view profile_path, bool use_env, bool is_debug) noexcept
{
  auto btos = [](bool _in) -> std::string { return _in ? "true" : "false"; };

  std::string txt(common::PROFILE_TOML_TEMPLATE);

  std::string _defines;
  for (auto& [key, val] : preprocessor.defines) std::format_to(std::back_inserter(_defines), "= \"{}\"\n", val);
  std::string _undefines;
  for (auto& val : preprocessor.undefines) std::format_to(std::back_inserter(_undefines), "\"{}\"\n", val);
  std::string _profiles;
  for (auto& elem : profiles) std::format_to(std::back_inserter(_profiles), "{}|", elem);
  _profiles = _profiles.substr(0, _profiles.size() - 1);
  std::string _dependencies;
  for (auto& elem : dependencies) std::format_to(std::back_inserter(_dependencies), "\"{}\",\n", elem.dependency);

  std::string _llvm_args;
  for (auto& val : llvm.args) std::format_to(std::back_inserter(_llvm_args), "\"{}\",\n", val);
  std::string _clang_args;
  for (auto& val : c_ffi.args) std::format_to(std::back_inserter(_clang_args), "\"{}\",\n", val);

  const std::string _libc_version = std::format("{}.{}", c_ffi.libc_version.major, c_ffi.libc_version.minor);

  const auto _c_source = env::FCSource_to_str(c_ffi.c_source);

  std::initializer_list<std::pair<std::string, std::string>> params = {
      // project
      {"profiles",                    _profiles                                                                                },
      {"dependencies",                _dependencies                                                                            },
      {"compilation_args_mode",       "union"                                                                                  },
      {"target.arch",                 std::string(use_env ? env::EArch_to_str(target.triple.arch) : "")                        },
      {"target.platform",             std::string(use_env ? env::EPlatform_to_str(target.triple.platform) : "")                },
      {"target.vendor",               std::string(use_env ? env::EVendor_to_str(target.triple.vendor) : "")                    },
      {"target.abi",                  std::string(use_env ? env::EABI_to_str(target.triple.abi) : "")                          },
      {"target.call_convention",      std::string(use_env ? env::ECallConvention_to_str(target.call_convention) : "")          },
      {"target.cpu",                  use_env ? target.cpu : ""                                                                },
      {"target.features",             use_env ? FCPUFeature_to_str(target.features) : ""                                       },
      {"target.features_mode",        "union"                                                                                  },
      {"target.code_model",           std::string(use_env ? ECodeModel_to_str(target.code_model) : "")                         },
      {"target.reloc_model",          std::string(use_env ? ERelocModel_to_str(target.reloc_model) : "")                       },
      {"target.emits",                use_env ? FEmit_to_str(target.emits) : ""                                                },
      {"target.emits_mode",           "union"                                                                                  },
      {"profile.debug",               is_debug ? "true" : btos(profile.debug)                                                  },
      {"profile.optimization",
       std::string(is_debug ? EOptimization_to_str(EOptimization::O0) : EOptimization_to_str(profile.optimization))            },
      {"log.logs",                    is_debug ? FPass_to_str(FPass::all) : FPass_to_str(log.logs)                             },
      {"log.logs_mode",               "union"                                                                                  },
      {"log.level",                   std::string(is_debug ? ELogLevel_to_str(ELogLevel::normal) : ELogLevel_to_str(log.level))},
      {"warn.warnings",               FWarnMode_to_str(warn.warns)                                                             },
      {"warn.warnings_mode",          "union"                                                                                  },
      {"warn.level",                  std::string(EWarnLevel_to_str(warn.level).substr(1))                                     },
      {"diagnostic.error",            std::string(EErrorMode_to_str(diagnostic.error))                                         },
      {"diagnostic.error_mode",       "union"                                                                                  },
      {"diagnostic.out_format",       std::string(EDiagnosticFormat_to_str(diagnostic.out_format))                             },
      {"debug.debugs",                FDebugPrinter_to_str(debug.debugs)                                                       },
      {"debug.debugs_mode",           "union"                                                                                  },
      {"preprocessor.defines",        use_env ? _defines : ""                                                                  },
      {"preprocessor.defines_mode",   "union"                                                                                  },
      {"preprocessor.undefines",      use_env ? _undefines : ""                                                                },
      {"preprocessor.undefines_mode", "union"                                                                                  },
      {"dir.build",                   dir.build                                                                                },
      {"dir.source",                  dir.source                                                                               },
      {"dir.profile",                 dir.profile                                                                              },
      {"dir.vendor",                  dir.vendor                                                                               },
      {"dir.ffi_json",                dir.ffi_json                                                                             },
      {"dir.binding",                 dir.binding                                                                              },
      {"dir.compiler",                std::string(common::env::get_compilers_dir())                                            },
      {"dir.stdlib",                  std::string(common::env::get_stdlib_dir())                                               },
      {"dir.packages",                std::string(common::env::get_packages_dir())                                             },
      {"llvm.verify_module",          use_env ? btos(llvm.verify_module) : "false"                                             },
      {"llvm.args",                   _llvm_args                                                                               },
      {"llvm.args_mode",              "union"                                                                                  },
      {"c_ffi.libc_version",          use_env ? _libc_version : ""                                                             },
      {"c_ffi.libc",                  std::string(use_env ? env::ELibC_to_str(c_ffi.libc) : "")                                },
      {"c_ffi.std",                   std::string(use_env ? env::ECStandard_to_str(c_ffi.std) : "")                            },
      {"c_ffi.c_source",              use_env ? _c_source : ""                                                                 },
      {"c_ffi.c_source_mode",         "union"                                                                                  },
      {"c_ffi.environment",           std::string(use_env ? env::EEnvironment_to_str(c_ffi.env) : "")                          },
      {"c_ffi.disable_builtins",      use_env ? btos(c_ffi.disable_builtins) : "false"                                         },
      {"c_ffi.strict_aliasing",       use_env ? btos(c_ffi.strict_aliasing) : "false"                                          },
      {"c_ffi.sysroot",               use_env ? c_ffi.sysroot : ""                                                             },
      {"c_ffi.args",                  use_env ? _clang_args : ""                                                               },
      {"c_ffi.args_mode",             "union"                                                                                  },
  };

  common::utils::fmt_template(txt, params);

  try {
    std::ofstream f = std::ofstream(std::string(profile_path), std::ios::binary);
    if (!f) return false;
    f.write(txt.data(), static_cast<std::streamsize>(txt.size()));
    if (!f) return false;
    f.close();
  } catch (...) {
    return false;
  }

  return true;
}


std::string common::MANIFEST_TOML_TEMPLATE = R"(
### tolza manifest

### it's the default configuration
### set profiles field to specify a combinaison of other options

### all fields will be stored as define element also

### if empty, the workspace file name will be used
project_name                        = "%project_name"
### at ./profile/*.toml name to apply after this
### e.g. "debug", "release", "windows|debug", "linux|posix|release",
profiles                            = "%profiles"

### package dependencies
### text list: "json", "scientific", ...
### package with version: "scientific@1.3.1"
dependencies                        = [
  %dependencies
]
    
[target]
### enum: )" + std::string(common::env::EArch_values)
                                             +
                                             R"( ...
arch                                = "%target.arch"                
### enum: )" + std::string(common::env::EPlatform_values)
                                             +
                                             R"( ...
platform                            = "%target.platform"                  
### enum: )" + std::string(common::env::EVendor_values)
                                             +
                                             R"( ...
vendor                              = "%target.vendor"              
### enum: )" + std::string(common::env::EABI_values)
                                             +
                                             R"( ...
abi                                 = "%target.abi"           
### text: overrides host cpu detection
cpu                                 = "%target.cpu"                 
### flags: )" + std::string(compiler::FCPUFeature_values)
                                             +
                                             R"( ...
features                            = "%target.features"
### enum: )" + std::string(common::env::ECallConvention_values)
                                             +
                                             R"( ...
call_convention                     = "%target.call_convention"
### enum: )" + std::string(compiler::ECodeModel_values)
                                             +
                                             R"( ...
code_model                          = "%target.code_model"          
### enum: )" + std::string(compiler::ERelocModel_values)
                                             +
                                             R"( ...
reloc_model                         = "%target.reloc_model"         
### flags: )" + std::string(compiler::FEmit_values)
                                             +
                                             R"( ...
emits                               = "%target.emits"               

[profile]
### the debug profile will override some options

### true = disable optimization, enable debug info
debug                               = %profile.debug
### enum: )" + std::string(compiler::EOptimization_values)
                                             +
                                             R"( ...
optimization                        = "%profile.optimization"       

[log]
### flags: )" + std::string(compiler::FPass_values)
                                             +
                                             R"( ...
logs                                = "%log.logs"        
### enum: )" + std::string(compiler::ELogLevel_values)
                                             + R"( ...
level                               = "%log.level"

[warning]
### flags: )" + std::string(compiler::FWarnMode_values)
                                             +
                                             R"( ...
warnings                            = "%warn.warnings"
### 1, 2, 3
level                               = %warn.level  

[debug]
### flags: ast
debugs                              = "%debug.debugs"

[diagnostic]
### enum: )" + std::string(compiler::EErrorMode_values)
                                             + R"( ...
error.mode                          = "%diagnostic.error.mode"
### enum: )" + std::string(compiler::EDiagnosticFormat_values)
                                             + R"( ...
out.format                          = "%diagnostic.out.format"

[preprocessor]
### text list : "my_undef", "my_other_undef", ...
undefines                           = [
  %preprocessor.undefines
]
[preprocessor.defines]
### map : key_name = "text_value"
%preprocessor.defines

[directory]
### relative to project root
build                               = "%dir.build"  
source                              = "%dir.source"  
profile                             = "%dir.profile"  
vendor                              = "%dir.vendor"  
ffi_json                            = "%dir.ffi_json"  
binding                             = "%dir.binding"  
compiler                            = "%dir.compiler"  
stdlib                              = "%dir.stdlib"  
packages                            = "%dir.packages"

### ============
### LLVM section
### ============
### This configuration is strictly for LLVM passes and code generation.
### It does not affect your Tolza preprocessor.

[llvm]
### verify before AND after passes
verify_module                       = %llvm.verify_module     
### custom raw flags passed to LLVM
### text list : "my_arg1", "my_arg2", ...
args                                = [                                      
  %llvm.args
]

### =============
### C ffi section
### =============
### This configuration is strictly for C ffi passes on C std lib

[c_ffi]
### enum: )" + std::string(common::env::ELibC_values)
                                             +
                                             R"( ...
libc                                = "%c_ffi.libc"
### major.minor format: e.g. 10.2
libc_version                        = "%c_ffi.libc_version"
### enum: )" + std::string(common::env::ECStandard_values)
                                             +
                                             R"( ...
std                                 = "%c_ffi.std"
### flags: )" + std::string(common::env::FCSource_values)
                                             +
                                             R"( ...
c_source                            = "%c_ffi.c_source"
### enum: )" + std::string(common::env::EEnvironment_values)
                                             +
                                             R"( ...
environment                         = "%c_ffi.environment"
### true, false
disable_builtins                    = %c_ffi.disable_builtins
### true, false
strict_aliasing                     = %c_ffi.strict_aliasing
### path
sysroot                             = "%c_ffi.sysroot"
### custom raw flags passed to the c linker
### text list : "my_arg1", "my_arg2", ...
args                                = [
  %c_ffi.args
]
)";


std::string common::PROFILE_TOML_TEMPLATE = R"(
### tolza-compiler profile

### it's a compositional profile configuration
### enable a field to override options configuration to compile 
### some options can specify a combinaison mode `*_mode` 
### you can remove a item/flag of combinaison with the prefix `!` e.g. `emits = "!bin|llvm"` `dependencies = [ "!wintext" ]`
### all fields will be stored as define element also


### at ./profile/*.toml name to apply after this
### e.g. "debug", "release", "windows|debug", "linux|posix|release",
# profiles                          = "%profiles"  

### package dependencies
### text list: "json", "scientific", ...
### package with version: "scientific@1.3.1"
### a duplicate package with different version will be overrided by the last profile specified
# dependencies                      = [
#   %dependencies
# ]

# compilation_args_mode             = "%compilation_args_mode" ### enum: "union", "inter", "!inter", "override"
    
[target]
### enum: )" + std::string(common::env::EArch_values)
                                            +
                                            R"( ...
# arch                              = "%target.arch"                
### enum: )" + std::string(common::env::EPlatform_values)
                                            +
                                            R"( ...
# platform                          = "%target.platform"                  
### enum: )" + std::string(common::env::EVendor_values)
                                            +
                                            R"( ...
# vendor                            = "%target.vendor"              
### enum: )" + std::string(common::env::EABI_values)
                                            +
                                            R"( ...
# abi                               = "%target.abi"           
### text: overrides host cpu detection
# cpu                               = "%target.cpu"                 
### flags: )" + std::string(compiler::FCPUFeature_values)
                                            +
                                            R"( ...
# features                          = "%target.features"
# features_mode                     = "%target.features_mode" ### enum: "union", "inter", "!inter", "override"
### enum: )" + std::string(common::env::ECallConvention_values)
                                            +
                                            R"( ...
# call_convention                   = "%target.call_convention"
### enum: )" + std::string(compiler::ECodeModel_values)
                                            +
                                            R"( ...
# code_model                        = "%target.code_model"          
### enum: )" + std::string(compiler::ERelocModel_values)
                                            +
                                            R"( ...
# reloc_model                       = "%target.reloc_model"         
### flags: )" + std::string(compiler::FEmit_values)
                                            +
                                            R"( ...
# emits                             = "%target.emits"
# emits_mode                        = "%target.emits_mode" ### enum: "union", "inter", "!inter", "override"

[profile]
### the debug profile will override some options

# true = disable optimization, enable debug info
# debug                             = %profile.debug
### enum: )" + std::string(compiler::EOptimization_values)
                                            +
                                            R"( ...
# optimization                      = "%profile.optimization"       

[log]
### flags: )" + std::string(compiler::FPass_values)
                                            +
                                            R"( ...
# logs                              = "%log.logs"        
# logs_mode                         = "%log.logs_mode" ### enum: "union", "inter", "!inter", "override"
### enum: )" + std::string(compiler::ELogLevel_values)
                                            + R"( ...
# level                             = "%log.level"

[warning]
### flags: )" + std::string(compiler::FWarnMode_values)
                                            +
                                            R"( ...
# warnings                          = "%warn.warnings"
# warnings_mode                        = "%warn.warnings_mode" ### enum: "union", "inter", "!inter", "override"

### 1, 2, 3
# level                             = %warn.level  

[debug]
### flags: ast
# debugs                            = "%debug.debugs"
# debugs_mode                       = "%debug.debugs_mode" ### enum: "union", "inter", "!inter", "override"


[diagnostic]
### enum: )" + std::string(compiler::EErrorMode_values)
                                            + R"( ...
# error                             = "%diagnostic.error"
### enum: )" + std::string(compiler::EDiagnosticFormat_values)
                                            + R"( ...
# out_format                        = "%diagnostic.out_format"

[preprocessor]
### text list : "my_undef", "my_other_undef", ...
# undefines                         = [
#   %preprocessor.undefines
# ]
# undefines_mode                    = "%preprocessor.undefines_mode" ### enum: "union", "inter", "!inter", "override"

[preprocessor.defines]
### map : key_name = "text_value"
### %preprocessor.defines
# defines_mode                      = "%preprocessor.defines_mode" ### enum: "union", "inter", "!inter", "override"

[directory]
### all path
# build                             = "%dir.build"  
# source                            = "%dir.source"  
# profile                           = "%dir.profile"  
# vendor                            = "%dir.vendor"  
# ffi_json                          = "%dir.ffi_json"  
# binding                           = "%dir.binding"  
# compiler                          = "%dir.compiler"  
# stdlib                            = "%dir.stdlib"  
# packages                          = "%dir.packages"

### ============
### LLVM section
### ============
### This configuration is strictly for LLVM passes and code generation.
### It does not affect your Tolza preprocessor.

[llvm]
### verify before AND after passes
# verify_module                     = %llvm.verify_module     
### custom raw flags passed to LLVM
### text list : "my_arg1", "my_arg2", ...
# args                              = [                                      
#   %llvm.args
# ]
# args_mode                         = "%llvm.args_mode" ### enum: "union", "inter", "!inter", "override"

### =============
### C ffi section
### =============
### This configuration is strictly for C ffi passes on C std lib

[c_ffi]
### enum: )" + std::string(common::env::ELibC_values)
                                            +
                                            R"( ...
# libc                              = "%c_ffi.libc"
### major.minor format: e.g. 10.2
# libc_version                      = "%c_ffi.libc_version"
### enum: )" + std::string(common::env::ECStandard_values)
                                            +
                                            R"( ...
# std                               = "%c_ffi.std"
### flags: )" + std::string(common::env::FCSource_values)
                                            +
                                            R"( ...
# c_source                          = "%c_ffi.c_source"
# c_source_mode                     = "%c_ffi.c_source_mode" ### enum: "union", "inter", "!inter", "override"
### enum: )" + std::string(common::env::EEnvironment_values)
                                            +
                                            R"( ...
# environment                       = "%c_ffi.environment"
### true, false
# disable_builtins                  = %c_ffi.disable_builtins
### true, false
# strict_aliasing                   = %c_ffi.strict_aliasing
### path
# sysroot                           = "%c_ffi.sysroot"
### custom raw flags passed to the c linker
### text list : "my_arg1", "my_arg2", ...
# args                              = [
#   %c_ffi.args
# ]
# args_mode                         = "%c_ffi.args_mode" ### enum: "union", "inter", "!inter", "override"                                                             
)";