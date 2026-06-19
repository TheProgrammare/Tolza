#include <common/compiler_options.hpp>

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/utils.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <set>
#include <string>
#include <filesystem>
#include <string_view>
#include <type_traits>
#include <vector>

#include <marzer/toml++.hpp>

#include <Neargye/magic_enum.hpp>
#include <Neargye/magic_enum_flags.hpp>


namespace fs = std::filesystem;


size_t common::compiler::Target::get_arch_size() const noexcept
{
  switch (arch) {

  case env::EArch::x86_64:
  case env::EArch::aarch64:
  case env::EArch::riscv64:
  case env::EArch::ppc64:
  case env::EArch::mips64:
  case env::EArch::wasm64:
  case env::EArch::sparc64:
  case env::EArch::unknown:
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

std::string common::compiler::Dir::get_dir_project() const noexcept
{
  return common::fileutils::resolve_path(project, fs::path(current_config_file).parent_path().string());
}
std::string common::compiler::Dir::get_dir_build() const noexcept
{
  return common::fileutils::resolve_path(build, get_dir_project());
}
std::string common::compiler::Dir::get_dir_source() const noexcept
{
  return common::fileutils::resolve_path(source, get_dir_project());
}
std::string common::compiler::Dir::get_dir_vendor() const noexcept
{
  return common::fileutils::resolve_path(vendor, get_dir_project());
}
std::string common::compiler::Dir::get_dir_binding() const noexcept
{
  return common::fileutils::resolve_path(binding, get_dir_project());
}
std::string common::compiler::Dir::get_dir_ffi_json() const noexcept
{
  return common::fileutils::resolve_path(ffi_json, get_dir_project());
}
std::string common::compiler::Dir::get_dir_compiler() const noexcept
{
  return common::fileutils::resolve_path(compiler, get_dir_project());
}
std::string common::compiler::Dir::get_dir_stdlib() const noexcept
{
  return common::fileutils::resolve_path(stdlib, get_dir_project());
}
std::string common::compiler::Dir::get_dir_packages() const noexcept
{
  return common::fileutils::resolve_path(packages, get_dir_project());
}

std::string common::compiler::Dir::get_preprocess_dir() const noexcept
{
  return (fs::path(build) / "preprocess").string();
}

std::string common::compiler::Dir::get_debug_graph_dir() const noexcept
{
  return (fs::path(build) / "graph").string();
}

std::string common::compiler::Dir::get_llvmir_dir() const noexcept
{
  return (fs::path(build) / "llvm-ir").string();
}

std::string common::compiler::Options::get_project_name() const noexcept
{
  if (project_name.empty()) return fs::path(dir.project).stem().string();
  return project_name;
}

std::string_view common::compiler::Options::get_out_name() const noexcept
{
  static auto out = fs::path(dir.current_config_file).stem().string();
  return out;
}

std::string_view common::compiler::Options::get_config_file() const noexcept
{
  static std::string out;
  if (!out.empty()) return out;

  if (dir.current_config_file.empty() || dir.current_config_file == "self") return out = dir.current_config_file;
  if (auto find = sub_configs.find(dir.current_config_file); find != sub_configs.end()) return out = find->second;

  return out = dir.current_config_file;
}

std::string common::compiler::Target::get_target_triple() const noexcept
{
  std::string triple;
  triple += GET_ENUM_NAME(arch);
  triple += "-";
  triple += GET_ENUM_NAME(vendor);
  triple += "-";
  triple += GET_ENUM_NAME(platform);
  return triple;
}

const std::vector<std::string>& common::compiler::Clang::generate_preprocessor_args() const noexcept
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
  for (auto& flag : common::ECSource_to_clang_flag(c_source)) args.insert(flag);

  return out = {args.begin(), args.end()};
}
const std::vector<std::string>& common::compiler::Options::generate_clang_args() const noexcept
{
  static std::vector<std::string> args;
  if (!args.empty()) return args;

  // target triple override
  if (!target.get_target_triple().empty()) args.emplace_back("--target=" + target.get_target_triple());

  // sysroot
  if (!clang.sysroot.empty()) args.emplace_back("--sysroot=" + clang.sysroot);

  // builtin control
  if (clang.disable_builtins) args.emplace_back("-fno-builtin");

  // aliasing
  if (!clang.strict_aliasing) args.emplace_back("-fno-strict-aliasing");

  // append preprocessor args
  auto pp = clang.generate_preprocessor_args();
  args.insert(args.end(), pp.begin(), pp.end());

  return args;
}

const std::map<std::string, std::string>& common::compiler::Options::generate_preprocessor_args() const noexcept
{
  static std::map<std::string, std::string> args;
  if (!args.empty()) return args;

  auto to_std_arg = [](std::string_view _str) {
    std::string tmp = std::string(_str);
    std::ranges::transform(tmp, tmp.begin(), ::toupper);
    return "__" + tmp + "__";
  };

  auto build_arg = [&](std::string_view _str, std::string_view key) {
    const std::string std_arg = to_std_arg(_str);
    args[std::string(key)]    = std_arg;
    args[std_arg]             = "true";
  };

  args["project_name"] = get_project_name();

  build_arg(GET_ENUM_NAME(target.arch), "target.arch");
  build_arg(GET_ENUM_NAME(target.platform), "target.platform");
  build_arg(GET_ENUM_NAME(target.vendor), "target.vendor");
  build_arg(GET_ENUM_NAME(target.abi), "target.abi");
  build_arg(target.cpu, "target.cpu");
  for (auto& feature : common::utils::split_flags(magic_enum::enum_flags_name(target.features, ','))) {
    args["target.feature." + feature] = "true";
  }
  build_arg(GET_ENUM_NAME(target.calling_conv), "target.calling_convention");
  build_arg(GET_ENUM_NAME(target.reloc_model), "target.reloc_model");
  build_arg(GET_ENUM_NAME(target.code_model), "target.code_model");
  for (auto& emit : common::utils::split_flags(magic_enum::enum_flags_name(target.emits, ','))) {
    args["target.emit." + emit] = "true";
  }

  args["llvm.verify_module"] = llvm.verify_module ? "true" : "false";

  build_arg(GET_ENUM_NAME(clang.libc), "clang.libc");
  args["clang.libc.version"] =
      std::to_string(clang.libc_version.major) + "." + std::to_string(clang.libc_version.minor);
  build_arg(GET_ENUM_NAME(clang.std), "clang.std");
  build_arg(GET_ENUM_NAME(clang.env), "clang.env");
  build_arg(std::to_string(clang.disable_builtins), "clang.disable_builtins");
  build_arg(std::to_string(clang.strict_aliasing), "clang.strict_aliasing");
  build_arg(clang.sysroot, "clang.sysroot");

  if (profile.debug) {
    build_arg("debug", "profile.build_mode");
  } else {
    build_arg("release", "profile.build_mode");
  }

  build_arg(GET_ENUM_NAME(profile.optimization), "optimization");

  for (auto& log : common::utils::split_flags(magic_enum::enum_flags_name(log.logs, ','))) build_arg(log, "log");

  for (auto& warn : common::utils::split_flags(magic_enum::enum_flags_name(warn.warns)))
    build_arg("true", "warning." + warn);
  build_arg(GET_ENUM_NAME(warn.level), "warning.level");

  for (auto& debug : common::utils::split_flags(magic_enum::enum_flags_name(debug.debugs)))
    build_arg("true", "debug." + debug);

  for (const auto& [key, val] : preprocessor.defines) args[key] = val;

  for (const auto& undef : preprocessor.undefines) args[undef] = "true";

  return args;
}

const std::vector<std::string>& common::compiler::Options::to_args() const noexcept
{
  static std::vector<std::string> out;
  if (!out.empty()) return out;
  std::vector<std::string> llvm_out;

  out.emplace_back("--project-name=\"" + project_name + "\"");
  out.emplace_back("--preset=\"" + GET_ENUM_NAME(platform_flavor) + "\"");

  // target
  out.emplace_back("--arch=\"" + GET_ENUM_NAME(target.arch) + "\"");
  out.emplace_back("--platform=\"" + GET_ENUM_NAME(target.platform) + "\"");
  out.emplace_back("--vendor=\"" + GET_ENUM_NAME(target.vendor) + "\"");
  out.emplace_back("--abi=\"" + GET_ENUM_NAME(target.abi) + "\"");
  out.emplace_back("--cpu=\"" + target.cpu + "\"");
  out.emplace_back("--features=\"" + GET_FLAGS_NAME(target.features) + "\"");
  out.emplace_back("--calling_convention=\"" + GET_ENUM_NAME(target.calling_conv) + "\"");
  out.emplace_back("--reloc-model=\"" + GET_ENUM_NAME(target.reloc_model) + "\"");
  out.emplace_back("--code-model=\"" + GET_ENUM_NAME(target.code_model) + "\"");
  out.emplace_back("--emits=\"" + GET_FLAGS_NAME(target.emits) + "\"");

  out.emplace_back("--libc=\"" + GET_ENUM_NAME(clang.libc) + "\"");

  if (sub_config != "self") out.emplace_back("--sub-config=\"" + sub_config + "\"");

  out.emplace_back(profile.debug ? "--debug" : "--release");
  out.emplace_back("-" + GET_ENUM_NAME(profile.optimization));

  out.emplace_back("--logs=\"" + GET_FLAGS_NAME(log.logs) + "\"");
  out.emplace_back("--warns=\"" + GET_FLAGS_NAME(warn.warns) + "\"");
  out.emplace_back("-" + GET_ENUM_NAME(warn.level));
  out.emplace_back("--debugs=\"" + GET_FLAGS_NAME(debug.debugs) + "\"");

  for (const auto& [name, val] : preprocessor.defines) out.emplace_back("-D" + name + "=" + val);

  for (const auto& udef : preprocessor.undefines) out.emplace_back("-U" + std::string(udef));

  out.emplace_back("--dir-project=\"" + dir.project + "\"");
  out.emplace_back("--dir-build=\"" + dir.build + "\"");
  out.emplace_back("--dir-src=\"" + dir.source + "\"");
  out.emplace_back("--dir-vendor=\"" + dir.vendor + "\"");
  out.emplace_back("--dir-ffi-json=\"" + dir.ffi_json + "\"");
  out.emplace_back("--dir-binding=\"" + dir.binding + "\"");
  out.emplace_back("--dir-compiler=\"" + dir.compiler + "\"");
  out.emplace_back("--dir-stdlib=\"" + dir.stdlib + "\"");
  out.emplace_back("--dir-packages=\"" + dir.packages + "\"");

  out.emplace_back("--error-mode=" + GET_ENUM_NAME(error_mode));

  if (llvm.verify_module) out.emplace_back("--verify-module");

  if (!llvm.args.empty()) out.emplace_back("--llvm");
  for (const auto& llvm_arg : llvm.args) out.emplace_back(llvm_arg);

  if (!clang.args.empty()) out.emplace_back("--clang");
  for (const auto& clang_arg : clang.args) out.emplace_back(clang_arg);

  return out;
}

common::compiler::Clang::LibCVersion common::compiler::Clang::LibCVersion::parse(std::string_view s) noexcept
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
std::string common::compiler::Clang::LibCVersion::print() const noexcept
{
  return std::to_string(major) + "." + std::to_string(minor);
}


common::compiler::Options common::compiler::Options::get_current(std::string_view name) noexcept
{
  return {
      .project_name = std::string(name),
      .target{
              .arch     = common::env::PlatformDetection::ARCH,
              .platform = common::env::PlatformDetection::PLATFORM,
              .vendor   = common::env::PlatformDetection::VENDOR,
              .abi      = common::env::PlatformDetection::ABI,
              }
  };
}
common::compiler::Options common::compiler::Options::get_preset(EPlatformFlavor platform) noexcept
{
  switch (platform) {

  case EPlatformFlavor::linux_glibc:
    return {
        .platform_flavor = EPlatformFlavor::linux_glibc,
        .target{
                .platform     = env::EPlatform::linux,
                .abi          = env::EABI::sysv,
                .calling_conv = ECallingConv::sysv,
                .reloc_model  = ERelocModel::PIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::glibc,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::gnu,
                .env              = EEnvironment::gnu,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::linux_musl:
    return {
        .platform_flavor = EPlatformFlavor::linux_musl,
        .target{
                .platform     = env::EPlatform::linux,
                .abi          = env::EABI::sysv,
                .calling_conv = ECallingConv::sysv,
                .reloc_model  = ERelocModel::PIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::musl,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::gnu,
                .env              = EEnvironment::musl,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::apple_macos:
    return {
        .platform_flavor = EPlatformFlavor::apple_macos,
        .target{
                .platform     = env::EPlatform::macos,
                .abi          = env::EABI::sysv,
                .calling_conv = ECallingConv::sysv,
                .reloc_model  = ERelocModel::PIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::libsystem,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::darwin,
                .env              = EEnvironment::darwin,
                .disable_builtins = false,
                .strict_aliasing  = false,
                }
    };

  case EPlatformFlavor::apple_ios:
    return {
        .platform_flavor = EPlatformFlavor::apple_ios,
        .target{
                .platform     = env::EPlatform::ios,
                .abi          = env::EABI::darwin_arm64,
                .calling_conv = ECallingConv::aapcs,
                .reloc_model  = ERelocModel::PIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::libsystem,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::darwin,
                .env              = EEnvironment::darwin,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::windows_msvc:
    return {
        .platform_flavor = EPlatformFlavor::windows_msvc,
        .target{
                .platform     = env::EPlatform::windows,
                .abi          = env::EABI::win64,
                .calling_conv = ECallingConv::win64,
                .reloc_model  = ERelocModel::STATIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::ucrt,
                .std              = ECStandard::c17,
                .c_source         = FCSource::crt_secure_no_warnings,
                .env              = EEnvironment::msvc,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::windows_mingw:
    return {
        .platform_flavor = EPlatformFlavor::windows_mingw,
        .target{
                .platform     = env::EPlatform::windows,
                .abi          = env::EABI::win64,
                .calling_conv = ECallingConv::win64,
                .reloc_model  = ERelocModel::STATIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::mingw_libc,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::gnu,
                .env              = EEnvironment::mingw,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::bsd_generic:
    return {
        .platform_flavor = EPlatformFlavor::bsd_generic,
        .target{
                .platform     = env::EPlatform::freebsd,
                .abi          = env::EABI::sysv,
                .calling_conv = ECallingConv::sysv,
                .reloc_model  = ERelocModel::PIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::bsd_libc,
                .std              = ECStandard::gnu17,
                .c_source         = FCSource::bsd,
                .env              = EEnvironment::gnu,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::wasi:
    return {
        .platform_flavor = EPlatformFlavor::wasi,
        .target{
                .platform     = env::EPlatform::unknown,
                .abi          = env::EABI::wasm32,
                .calling_conv = ECallingConv::cdecl,
                .reloc_model  = ERelocModel::STATIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::unknown,
                .std              = ECStandard::c17,
                .c_source         = FCSource::NONE,
                .env              = EEnvironment::wasi,
                .disable_builtins = false,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::baremetal:
    return {
        .platform_flavor = EPlatformFlavor::baremetal,
        .target{
                .platform     = env::EPlatform::unknown,
                .abi          = env::EABI::custom,
                .calling_conv = ECallingConv::cdecl,
                .reloc_model  = ERelocModel::STATIC,
                .code_model   = ECodeModel::small,
                },
        .clang{
                .libc             = ELibC::unknown,
                .std              = ECStandard::c17,
                .c_source         = FCSource::NONE,
                .env              = EEnvironment::baremetal,
                .disable_builtins = true,
                .strict_aliasing  = false,
                },
    };

  case EPlatformFlavor::unknown:
  case EPlatformFlavor::custom:  return Options::invalid();
  }

  return Options::invalid();
}

template <typename Enum>
void merge_bitwise(Enum& _dest, Enum _val, common::compiler::Sub_Compiler_Options::EMergeMode mode) noexcept
{
  using _underlying = std::underlying_type_t<Enum>;

  static_assert(std::is_enum_v<Enum>, "merge_bitwise: must be an enum");

  const auto  d = static_cast<_underlying>(_dest);
  const auto  v = static_cast<_underlying>(_val);
  _underlying out;

  switch (mode) {
  case common::compiler::Sub_Compiler_Options::EMergeMode::_union: {
    out = d | v;
    break;
  }
  case common::compiler::Sub_Compiler_Options::EMergeMode::_intersection: {
    out = d & v;
    break;
  }
  case common::compiler::Sub_Compiler_Options::EMergeMode::_anti_intersection: {
    out = d & ~v;
    break;
  }
  }

  _dest = static_cast<Enum>(out);
}

common::compiler::Options common::compiler::Sub_Compiler_Options::merge_context(const Options& base_ctx) const noexcept
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

  auto merge_list = [&](std::vector<const char*>& _dest, const std::vector<const char*>& _val, EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      _dest.insert(_dest.end(), _val.begin(), _val.end());
      break;
    }
    case EMergeMode::_intersection: {
      std::vector<const char*> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<const char*>(_val.begin(), _val.end());
      for (const auto& elem : _dest) {
        if (tmp_set.find(elem) != tmp_set.end()) tmp.emplace_back(elem);
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::vector<const char*> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<const char*>(_val.begin(), _val.end());
      for (const auto& elem : _dest) {
        if (tmp_set.find(elem) == tmp_set.end()) tmp.emplace_back(elem);
      }
      _dest = tmp;
      break;
    }
    }
  };

  auto merge_map = [&](std::map<std::string, std::string>& _dest, const std::map<std::string, std::string>& _val,
                       EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      for (const auto& [val_key, val_val] : _val) _dest[val_key] = val_val;

      break;
    }
    case EMergeMode::_intersection: {
      std::map<std::string, std::string> tmp;
      for (const auto& [key, val] : _dest) {
        if (_val.find(key) != _val.end()) tmp[key] = val;
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::map<std::string, std::string> tmp;
      for (const auto& [key, val] : _dest) {
        if (_val.find(key) == _val.end()) tmp[key] = val;
      }
      _dest = tmp;
      break;
    }
    }
  };


  Options out = base_ctx;

  apply_str(out.sub_config, sub_config);

  merge_map(out.PREPROCESSOR_ARGS, PREPROCESSOR_ARGS, COMPILATION_ARGS_merge_mode);

  apply_str(out.project_name, project_name);
  if (platform_flavor != EPlatformFlavor::unknown) out.platform_flavor = platform_flavor;

  // target
  if (target.arch != env::EArch::unknown) out.target.arch = target.arch;
  if (target.platform != env::EPlatform::unknown) out.target.platform = target.platform;
  if (target.vendor != env::EVendor::unknown) out.target.vendor = target.vendor;
  if (target.abi != env::EABI::unknown) out.target.abi = target.abi;
  apply_str(out.target.cpu, target.cpu);
  merge_bitwise(out.target.features, target.features, features_merge_mode);
  if (target.calling_conv != ECallingConv::unknown) out.target.calling_conv = target.calling_conv;
  if (target.reloc_model != ERelocModel::NONE) out.target.abi = target.abi;
  if (target.code_model != ECodeModel::NONE) out.target.code_model = target.code_model;
  merge_bitwise(out.target.emits, target.emits, emits_merge_mode);

  // profile
  apply_bool(out.profile.debug, profile.debug, has_profile_debug);
  if (profile.optimization != EOptimization::NONE) out.profile.optimization = profile.optimization;

  // logs
  merge_bitwise(out.log.logs, log.logs, logs_merge_mode);

  // warns
  merge_bitwise(out.warn.warns, warn.warns, warnings_merge_mode);
  if (warn.level != EWarnLevel::NONE) out.warn.level = warn.level;

  // debugs
  merge_bitwise(out.debug.debugs, debug.debugs, debug_merge_mode);

  // dirs
  out.dir.current_config_file = dir.current_config_file;
  apply_str(out.dir.project, dir.project);
  apply_str(out.dir.build, dir.build);
  apply_str(out.dir.source, dir.source);
  apply_str(out.dir.vendor, dir.vendor);
  apply_str(out.dir.binding, dir.binding);
  apply_str(out.dir.ffi_json, dir.ffi_json);
  apply_str(out.dir.compiler, dir.compiler);
  apply_str(out.dir.stdlib, dir.stdlib);
  apply_str(out.dir.packages, dir.packages);

  // preprocessor
  merge_map(out.preprocessor.defines, preprocessor.defines, defines_merge_mode);
  merge_list(out.preprocessor.undefines, preprocessor.undefines, undefines_merge_mode);

  // llvm
  apply_bool(out.llvm.verify_module, llvm.verify_module, has_llvm_verify_module);
  merge_list(out.llvm.args, llvm.args, llvm_args_merge_mode);

  if (clang.libc != ELibC::unknown) out.clang.libc = clang.libc;
  if (clang.libc_version.minor != 0 && clang.libc_version.major != 0) out.clang.libc_version = clang.libc_version;
  if (clang.std != ECStandard::unknown) out.clang.std = clang.std;
  merge_bitwise(out.clang.c_source, clang.c_source, clang_c_source_merge_mode);
  if (clang.env != EEnvironment::unknown) out.clang.env = clang.env;
  apply_bool(out.clang.disable_builtins, clang.disable_builtins, has_clang_disable_builtins);
  apply_bool(out.clang.strict_aliasing, clang.strict_aliasing, has_clang_strict_aliasing);
  apply_str(out.clang.sysroot, clang.sysroot);
  merge_list(out.clang.args, clang.args, clang_args_merge_mode);

  return out;
}

common::compiler::Options common::compiler::Options::read_config(std::string_view path) noexcept
{
#define get_enum(_path, _enum_kind, _enum_default)                                                                     \
  magic_enum::enum_cast<_enum_kind>(utils::str_to_snake(tbl.at_path(_path).value_or("")))                              \
      .value_or(_enum_kind::_enum_default)

#define get_str(_path) tbl.at_path("target.cpu").value_or("")

#define get_bool(_path) tbl.at_path(_path).value_or(false)

  auto resolve_path = [&path](std::string_view _path) -> fs::path {
    if (fs::path(_path).is_relative()) {
      return fs::weakly_canonical(fs::path(path).parent_path() / _path);
    }

    return fs::absolute(_path);
  };

  auto is_same_key = [](std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
  };

  auto out = common::compiler::Options::invalid();


  toml::table tbl;
  tbl = toml::parse_file(path);

  out.project_name    = get_str("project_name");
  out.platform_flavor = get_enum("preset", EPlatformFlavor, unknown);

  out.dir.current_config_file = path;
  out.sub_config              = get_str("sub_config");

  // target
  out.target.arch        = get_enum("target.arch", env::EArch, unknown);
  out.target.platform    = get_enum("target.platform", env::EPlatform, unknown);
  out.target.vendor      = get_enum("target.vendor", env::EVendor, unknown);
  out.target.abi         = get_enum("target.abi", env::EABI, unknown);
  out.target.cpu         = get_str("target.cpu");
  out.target.features    = get_enum("target.features", FCPUFeature, NONE);
  out.target.reloc_model = get_enum("target.reloc_model", ERelocModel, PIC);
  out.target.code_model  = get_enum("target.code_model", ECodeModel, small);

  // profile
  out.profile.debug        = get_bool("profile.debug");
  out.profile.optimization = get_enum("profile.optimization", EOptimization, O0);

  // log
  out.log.logs = get_enum("log.logs", FPass, NONE);

  // warn
  out.warn.warns = get_enum("warn.warns", FPass, NONE);
  out.warn.level = get_enum("warn.level", EWarnLevel, W0);

  // llvm
  out.llvm.verify_module = get_bool("llvm.verify_module");
  if (const toml::array* llvm_args = tbl.at_path("llvm.args").as_array()) {
    for (const auto& key : *llvm_args) out.llvm.args.emplace_back(key.value_or(""));
  }

  // directories
  out.dir.project  = resolve_path(get_str("directory.project"));
  out.dir.build    = resolve_path(get_str("directory.build"));
  out.dir.source   = resolve_path(get_str("directory.source"));
  out.dir.vendor   = resolve_path(get_str("directory.vendor"));
  out.dir.binding  = resolve_path(get_str("directory.binding"));
  out.dir.ffi_json = resolve_path(get_str("directory.ffi_json"));
  out.dir.compiler = resolve_path(get_str("directory.compiler"));
  out.dir.stdlib   = resolve_path(get_str("directory.stdlib"));
  out.dir.packages = resolve_path(get_str("directory.packages"));

  if (const toml::table* defines = tbl.at_path("preprocessor.defines").as_table()) {
    for (const auto& [key, val] : *defines) out.preprocessor.defines[std::string(key.str())] = val.value_or("");
  }

  if (const toml::array* undefines = tbl.at_path("preprocessor.undefines").as_array()) {
    for (const auto& undef : *undefines) out.preprocessor.undefines.emplace_back(undef.value_or(""));
  }

  if (const toml::array* clang_args = tbl.at_path("clang.args").as_array()) {
    for (const auto& key : *clang_args) out.clang.args.emplace_back(key.value_or(""));
  }

  if (const toml::table* sub_configs = tbl.at_path("config").as_table()) {
    for (const auto& [key, val] : *sub_configs) out.sub_configs[std::string(key)] = val.value_or("./");
  }

  return out;

#undef get_enum
#undef get_str
#undef get_bool
}

bool common::compiler::Options::write_config(std::string_view path) noexcept
{
  auto btos = [](bool _in) -> std::string { return _in ? "true" : "false"; };

  std::string config_txt(common::OPTIONS_TEMPLATE);

  std::string _defines;
  for (auto& [key, val] : preprocessor.defines) _defines += key + " = \"" + val + "\"\n";
  std::string _undefines;
  for (auto& val : preprocessor.undefines) _undefines += "\"" + std::string(val) + "\n";
  std::string _sub_configs;
  for (auto& [key, val] : sub_configs) _defines += key + " = \"" + val + "\"\n";
  std::string _llvm_args;
  for (auto& val : llvm.args) _llvm_args += "\"" + std::string(val) + "\",\n";
  std::string _clang_c_source;
  for (auto& scr : common::utils::split_flags(GET_FLAGS_NAME(clang.c_source))) _clang_c_source += "\"" + scr + "\",\n";
  std::string _clang_args;
  for (auto& val : clang.args) _clang_args += "\"" + std::string(val) + "\",\n";
  std::string _features;
  for (auto& feature : common::utils::split_flags(GET_FLAGS_NAME(target.features)))
    _features += "\"" + std::string(feature) + "\",\n";
  std::string _logs;
  for (auto& log : common::utils::split_flags(GET_FLAGS_NAME(log.logs))) _logs += "\"" + std::string(log) + "\",\n";
  std::string _warns;
  for (auto& warn : common::utils::split_flags(GET_FLAGS_NAME(warn.warns)))
    _warns += "\"" + std::string(warn) + "\",\n";
  std::string _debugs;
  for (auto& debug : common::utils::split_flags(GET_FLAGS_NAME(debug.debugs)))
    _debugs += "\"" + std::string(debug) + "\",\n";
  std::string _emits;
  for (auto& emit : common::utils::split_flags(GET_FLAGS_NAME(target.emits)))
    _emits += "\"" + std::string(emit) + "\",\n";

  const std::string _libc_version =
      std::to_string(clang.libc_version.major) + "." + std::to_string(clang.libc_version.minor);

  std::map<std::string_view, std::string_view> config_params = {
      // project
      {"project_name",           project_name                       },
      {"preset",                 GET_ENUM_NAME(platform_flavor)     },
      // target
      {"sub_config",             sub_config                         },
      {"target.arch",            GET_ENUM_NAME(target.arch)         },
      {"target.platform",        GET_ENUM_NAME(target.platform)     },
      {"target.vendor",          GET_ENUM_NAME(target.vendor)       },
      {"target.abi",             GET_ENUM_NAME(target.abi)          },
      {"target.cpu",             target.cpu                         },
      {"target.features",        _features                          },
      {"target.code_model",      GET_ENUM_NAME(target.code_model)   },
      {"target.reloc_model",     GET_ENUM_NAME(target.reloc_model)  },
      {"target.emit",            _emits                             },
      // profile
      {"profile.debug",          btos(profile.debug)                },
      {"profile.optimization",   GET_ENUM_NAME(profile.optimization)},
      // logs
      {"log.logs",               _logs                              },
      // warnings
      {"warn.warnings",          _warns                             },
      {"warn.level",             GET_ENUM_NAME(warn.level)          },
      // debug printer
      {"debug.debugs",           _debugs                            },
      // defines
      {"preprocessor.defines",   _defines                           },
      // undefines
      {"preprocessor.undefines", _undefines                         },
      // directories
      {"dir.project",            dir.project                        },
      {"dir.build",              dir.build                          },
      {"dir.source",             dir.source                         },
      {"dir.vendor",             dir.vendor                         },
      {"dir.ffi_json",           dir.ffi_json                       },
      {"dir.binding",            dir.binding                        },
      {"dir.compiler",           common::env::get_compilers_dir()   },
      {"dir.stdlib",             common::env::get_stdlib_dir()      },
      {"dir.packages",           common::env::get_packages_dir()    },
      // sub_configs
      {"config.sub_configs",     _sub_configs                       },
      // llvm
      {"llvm.verify_module",     btos(llvm.verify_module)           },
      {"llvm.args",              _llvm_args                         },
      // clang
      {"clang.libc",             GET_ENUM_NAME(clang.libc)          },
      {"clang.libc_version",     _libc_version                      },
      {"clang.std",              GET_ENUM_NAME(clang.std)           },
      {"clang.c_source",         _clang_c_source                    },
      {"clang.environment",      GET_ENUM_NAME(clang.env)           },
      {"clang.disable_builtins", btos(clang.disable_builtins)       },
      {"clang.strict_aliasing",  btos(clang.strict_aliasing)        },
      {"clang.sysroot",          clang.sysroot                      },
      {"clang.args",             _clang_args                        },
  };

  common::utils::fmt_template(config_txt, config_params);

  try {
    std::ofstream f(path.data());
    f.clear();
    f << config_txt << std::flush;
    f.close();
  } catch (...) {
    return false;
  }

  return true;
}
