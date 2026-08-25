#include <common/compiler_options.hpp>

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/utils.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
  switch (triple.arch) {
  case env::EArch::DEFAULT:
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
  return common::fileutils::resolve_path(project);
}
std::string common::compiler::Dir::get_dir_build() const noexcept
{
  return common::fileutils::resolve_path(build, get_dir_project());
}
std::string common::compiler::Dir::get_dir_source() const noexcept
{
  return common::fileutils::resolve_path(source, get_dir_project());
}
std::string common::compiler::Dir::get_dir_profile() const noexcept
{
  return common::fileutils::resolve_path(profile, get_dir_project());
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

std::string common::compiler::Options::get_dir_preprocess() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "preprocess").string();
}

std::string common::compiler::Options::get_dir_debug_graph() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "graph").string();
}

std::string common::compiler::Options::get_dir_llvmir() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "llvm-ir").string();
}

std::string common::compiler::Options::get_dir_binding_profile() const noexcept
{
  return (fs::path(get_dir_build_profile()) / "binding").string();
}

std::string common::compiler::Options::get_project_name() const noexcept
{
  if (project_name.empty()) return fs::path(dir.project).stem().string();
  return project_name;
}

std::string common::compiler::Options::get_out_name() const noexcept
{
  static auto out = std::format("{}-{}", project_name, get_profile_filename());
  return out;
}

std::string common::compiler::Options::get_profile_filename() const noexcept
{
  static std::string out;
  if (!out.empty()) return out;

  for (const auto& elem : profiles) std::format_to(std::back_inserter(out), "{}-", elem);
  return out = out.substr(0, out.size() - 1);
}
std::string common::compiler::Options::get_dir_build_profile() const noexcept
{
  return common::fileutils::resolve_path((fs::path(dir.get_dir_build()) / get_profile_filename()).string(),
                                         dir.get_dir_project());
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
  result.arch = magic_enum::enum_cast<env::EArch>(std::string(parts[0])).value_or(env::EArch::unknown);
  if (parts.size() >= 2)
    result.vendor = magic_enum::enum_cast<env::EVendor>(std::string(parts[1])).value_or(env::EVendor::unknown);
  if (parts.size() >= 3)
    result.platform = magic_enum::enum_cast<env::EPlatform>(std::string(parts[2])).value_or(env::EPlatform::unknown);
  if (parts.size() >= 4)
    result.abi = magic_enum::enum_cast<env::EABI>(std::string(parts[3])).value_or(env::EABI::unknown);

  return result;
}

std::string common::compiler::TargetTriple::dump() const noexcept
{
  std::string triple;
  triple += magic_enum::enum_name(arch);
  triple += "-";
  triple += magic_enum::enum_name(vendor);
  triple += "-";
  triple += magic_enum::enum_name(platform);
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
  for (auto& flag : common::ECSource_to_clang_flag(c_source)) args.insert(flag);

  return out = {args.begin(), args.end()};
}
const std::vector<std::string>& common::compiler::Options::generate_clang_args() const noexcept
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

  build_arg(GET_ENUM_NAME(target.triple.arch), "target.arch");
  build_arg(GET_ENUM_NAME(target.triple.platform), "target.platform");
  build_arg(GET_ENUM_NAME(target.triple.vendor), "target.vendor");
  build_arg(GET_ENUM_NAME(target.triple.abi), "target.abi");
  build_arg(target.cpu, "target.cpu");
  for (auto& feature : common::utils::split_flags(magic_enum::enum_flags_name(target.features, ','))) {
    args["target.feature." + feature] = "true";
  }
  build_arg(GET_ENUM_NAME(target.call_convention), "target.calling_convention");
  build_arg(GET_ENUM_NAME(target.reloc_model), "target.reloc_model");
  build_arg(GET_ENUM_NAME(target.code_model), "target.code_model");
  for (auto& emit : common::utils::split_flags(magic_enum::enum_flags_name(target.emits, ','))) {
    args["target.emit." + emit] = "true";
  }

  args["llvm.verify_module"] = llvm.verify_module ? "true" : "false";

  build_arg(GET_ENUM_NAME(c_ffi.libc), "clang.libc");
  args["clang.libc.version"] =
      std::to_string(c_ffi.libc_version.major) + "." + std::to_string(c_ffi.libc_version.minor);
  build_arg(GET_ENUM_NAME(c_ffi.std), "clang.std");
  build_arg(GET_ENUM_NAME(c_ffi.env), "clang.env");
  build_arg(std::to_string(c_ffi.disable_builtins), "clang.disable_builtins");
  build_arg(std::to_string(c_ffi.strict_aliasing), "clang.strict_aliasing");
  build_arg(c_ffi.sysroot, "clang.sysroot");

  if (profile.debug) {
    build_arg("debug", "profile.build_mode");
  } else {
    build_arg("release", "profile.build_mode");
  }

  build_arg(GET_ENUM_NAME(profile.optimization), "optimization");

  for (auto& log : common::utils::split_flags(magic_enum::enum_flags_name(log.logs, ','))) build_arg(log, "log");
  build_arg(GET_ENUM_NAME(log.level), "log.level");

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

  out.emplace_back(std::format(R"(--project-name="{}")", project_name));
  if (is_check_mode) out.emplace_back("--check");

  // target
  out.emplace_back(std::format(R"(--arch="{}")", GET_ENUM_NAME(target.triple.arch)));
  out.emplace_back(std::format(R"(--platform="{}")", GET_ENUM_NAME(target.triple.platform)));
  out.emplace_back(std::format(R"(--vendor="{}")", GET_ENUM_NAME(target.triple.vendor)));
  out.emplace_back(std::format(R"(--abi="{}")", GET_ENUM_NAME(target.triple.abi)));
  out.emplace_back(std::format(R"(--cpu="{}")", target.cpu));
  out.emplace_back(std::format(R"(--features="{}")", GET_FLAGS_NAME(target.features)));
  out.emplace_back(std::format(R"(--calling_convention="{}")", GET_ENUM_NAME(target.call_convention)));
  out.emplace_back(std::format(R"(--reloc-model="{}")", GET_ENUM_NAME(target.reloc_model)));
  out.emplace_back(std::format(R"(--code-model="{}")", GET_ENUM_NAME(target.code_model)));
  out.emplace_back(std::format(R"(--emits="{}")", GET_FLAGS_NAME(target.emits)));

  out.emplace_back(std::format(R"(--libc="{}")", GET_ENUM_NAME(c_ffi.libc)));

  std::string _profile;
  for (const auto& elem : profiles) std::format_to(std::back_inserter(_profile), "{}|", elem);
  _profile = _profile.substr(0, _profile.size() - 1);
  out.emplace_back(std::format(R"(--profile="{}")", _profile));

  out.emplace_back(profile.debug ? "--debug" : "--release");
  out.emplace_back("-" + GET_ENUM_NAME(profile.optimization));

  out.emplace_back(std::format(R"(--logs="{}")", GET_FLAGS_NAME(log.logs)));
  out.emplace_back(std::format(R"(--log-level="{}")", GET_ENUM_NAME(log.level)));
  out.emplace_back(std::format(R"(--warns="{}")", GET_FLAGS_NAME(warn.warns)));
  out.emplace_back("-" + GET_ENUM_NAME(warn.level));
  out.emplace_back(std::format(R"(--debugs="{}")", GET_FLAGS_NAME(debug.debugs)));

  for (const auto& [name, val] : preprocessor.defines) out.emplace_back("-D" + name + "=" + val);

  for (const auto& udef : preprocessor.undefines) out.emplace_back("-U" + std::string(udef));

  out.emplace_back(std::format(R"(--dir-project="{}")", dir.project));
  out.emplace_back(std::format(R"(--dir-build="{}")", dir.build));
  out.emplace_back(std::format(R"(--dir-src="{}")", dir.source));
  out.emplace_back(std::format(R"(--dir-profile="{}")", dir.profile));
  out.emplace_back(std::format(R"(--dir-vendor="{}")", dir.vendor));
  out.emplace_back(std::format(R"(--dir-ffi-json="{}")", dir.ffi_json));
  out.emplace_back(std::format(R"(--dir-binding="{}")", dir.binding));
  out.emplace_back(std::format(R"(--dir-compiler="{}")", dir.compiler));
  out.emplace_back(std::format(R"(--dir-stdlib="{}")", dir.stdlib));
  out.emplace_back(std::format(R"(--dir-packages="{}")", dir.packages));

  out.emplace_back(std::format(R"(--error-mode="{}")", GET_ENUM_NAME(diagnostic.error_mode)));
  out.emplace_back(std::format(R"(--diagnostic-format="{}")", GET_ENUM_NAME(diagnostic.out_format)));

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
std::string common::compiler::Cffi::LibCVersion::print() const noexcept
{
  return std::to_string(major) + "." + std::to_string(minor);
}


common::compiler::Options common::compiler::Options::get_current(std::string_view name) noexcept
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

common::compiler::Options common::compiler::Options::get_preset(const TargetTriple& triple) noexcept
{
  Options options{
      .target{
              .triple{
              .platform = triple.platform,
              .abi      = triple.abi,
          }, .call_convention = env::call_convention(triple.platform, triple.arch, triple.abi),
              .reloc_model     = ERelocModel::PIC,
              .code_model      = ECodeModel::small,
              },
  };


  switch (triple.platform) {

  case env::EPlatform::linux:
    options.c_ffi = {
        .libc = triple.abi == env::EABI::gnu ? env::ELibC::glibc : env::ELibC::musl,

        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::gnu,
        .env              = env::EEnvironment::gnu,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::macos:
    options.c_ffi = {
        .libc             = env::ELibC::libsystem,
        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::darwin,
        .env              = env::EEnvironment::darwin,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::ios:
    options.c_ffi = {
        .libc             = env::ELibC::libsystem,
        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::darwin,
        .env              = env::EEnvironment::darwin,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::windows:
    if (triple.abi == env::EABI::win64 || triple.abi == env::EABI::msvc_x64) {
      options.c_ffi = {
          .libc             = env::ELibC::ucrt,
          .std              = env::ECStandard::c17,
          .c_source         = env::FCSource::crt_secure_no_warnings,
          .env              = env::EEnvironment::msvc,
          .disable_builtins = false,
          .strict_aliasing  = false,
      };

    } else {
      options.c_ffi = {
          .libc             = env::ELibC::ucrt,
          .std              = env::ECStandard::c17,
          .c_source         = env::FCSource::crt_secure_no_warnings,
          .env              = env::EEnvironment::msvc,
          .disable_builtins = false,
          .strict_aliasing  = false,
      };
    }

    options.target.reloc_model = ERelocModel::STATIC;
    return options;


  case env::EPlatform::freebsd:
  case env::EPlatform::openbsd:
  case env::EPlatform::netbsd:
  case env::EPlatform::dragonflybsd:
    options.c_ffi = {
        .libc             = env::ELibC::bsd_libc,
        .std              = env::ECStandard::gnu17,
        .c_source         = env::FCSource::bsd,
        .env              = env::EEnvironment::gnu,
        .disable_builtins = false,
        .strict_aliasing  = false,
    };

    return options;


  case env::EPlatform::unknown:
  case env::EPlatform::android:
  case env::EPlatform::solaris:
  default:                      break;
  }


  // WASI / WebAssembly
  if (triple.abi == env::EABI::wasm32 || triple.arch == env::EArch::wasm32) {
    options.target.reloc_model = ERelocModel::STATIC;

    options.c_ffi = {
        .libc             = env::ELibC::unknown,
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
      .libc             = env::ELibC::unknown,
      .std              = env::ECStandard::c17,
      .c_source         = env::FCSource::NONE,
      .env              = env::EEnvironment::baremetal,
      .disable_builtins = true,
      .strict_aliasing  = false,
  };

  return options;
}

template <typename Enum>
void merge_bitwise(Enum& _dest, Enum _val, common::compiler::Sub_Options::EMergeMode mode) noexcept
{
  using _underlying = std::underlying_type_t<Enum>;

  static_assert(std::is_enum_v<Enum>, "merge_bitwise: must be an enum");

  const auto  d = static_cast<_underlying>(_dest);
  const auto  v = static_cast<_underlying>(_val);
  _underlying out;

  switch (mode) {
  case common::compiler::Sub_Options::EMergeMode::_union: {
    out = d | v;
    break;
  }
  case common::compiler::Sub_Options::EMergeMode::_intersection: {
    out = d & v;
    break;
  }
  case common::compiler::Sub_Options::EMergeMode::_anti_intersection: {
    out = d & ~v;
    break;
  }
  }

  _dest = static_cast<Enum>(out);
}

common::compiler::Options common::compiler::Sub_Options::merge_context(const Options& base_ctx) const noexcept
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
  auto merge_list_str = [&](std::vector<std::string>& _dest, const std::vector<std::string>& _val, EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      _dest.insert(_dest.end(), _val.begin(), _val.end());
      break;
    }
    case EMergeMode::_intersection: {
      std::vector<std::string> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<std::string_view>(_val.begin(), _val.end());
      for (const auto& elem : _dest) {
        if (tmp_set.find(elem) != tmp_set.end()) tmp.emplace_back(elem);
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::vector<std::string> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<std::string_view>(_val.begin(), _val.end());
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

  merge_list_str(out.profiles, profiles, COMPILATION_ARGS_merge_mode);

  merge_map(out.PREPROCESSOR_ARGS, PREPROCESSOR_ARGS, COMPILATION_ARGS_merge_mode);

  apply_str(out.project_name, project_name);

  // target
  if (target.triple.arch != env::EArch::unknown) out.target.triple.arch = target.triple.arch;
  if (target.triple.platform != env::EPlatform::unknown) out.target.triple.platform = target.triple.platform;
  if (target.triple.vendor != env::EVendor::unknown) out.target.triple.vendor = target.triple.vendor;
  if (target.triple.abi != env::EABI::unknown) out.target.triple.abi = target.triple.abi;
  apply_str(out.target.cpu, target.cpu);
  merge_bitwise(out.target.features, target.features, features_merge_mode);
  if (target.call_convention != env::ECallConvention::unknown) out.target.call_convention = target.call_convention;
  if (target.reloc_model != ERelocModel::DEFAULT) out.target.triple.abi = target.triple.abi;
  if (target.code_model != ECodeModel::DEFAULT) out.target.code_model = target.code_model;
  merge_bitwise(out.target.emits, target.emits, emits_merge_mode);

  // profile
  apply_bool(out.profile.debug, profile.debug, has_profile_debug);
  if (profile.optimization != EOptimization::DEFAULT) out.profile.optimization = profile.optimization;

  // logs
  merge_bitwise(out.log.logs, log.logs, logs_merge_mode);
  if (log.level != ELogLevel::DEFAULT) out.log.level = log.level;

  // warns
  merge_bitwise(out.warn.warns, warn.warns, warnings_merge_mode);
  if (warn.level != EWarnLevel::DEFAULT) out.warn.level = warn.level;

  // debugs
  merge_bitwise(out.debug.debugs, debug.debugs, debug_merge_mode);

  // dirs
  apply_str(out.dir.project, dir.project);
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
  merge_map(out.preprocessor.defines, preprocessor.defines, defines_merge_mode);
  merge_list(out.preprocessor.undefines, preprocessor.undefines, undefines_merge_mode);

  // llvm
  apply_bool(out.llvm.verify_module, llvm.verify_module, has_llvm_verify_module);
  merge_list(out.llvm.args, llvm.args, llvm_args_merge_mode);

  if (c_ffi.libc != env::ELibC::unknown) out.c_ffi.libc = c_ffi.libc;
  if (c_ffi.libc_version.minor != 0 && c_ffi.libc_version.major != 0) out.c_ffi.libc_version = c_ffi.libc_version;
  if (c_ffi.std != env::ECStandard::unknown) out.c_ffi.std = c_ffi.std;
  merge_bitwise(out.c_ffi.c_source, c_ffi.c_source, clang_c_source_merge_mode);
  if (c_ffi.env != env::EEnvironment::unknown) out.c_ffi.env = c_ffi.env;
  apply_bool(out.c_ffi.disable_builtins, c_ffi.disable_builtins, has_clang_disable_builtins);
  apply_bool(out.c_ffi.strict_aliasing, c_ffi.strict_aliasing, has_clang_strict_aliasing);
  apply_str(out.c_ffi.sysroot, c_ffi.sysroot);
  merge_list(out.c_ffi.args, c_ffi.args, clang_args_merge_mode);

  return out;
}

common::compiler::Options common::compiler::Options::read_config(std::string_view path) noexcept
{
#define get_enum(_path, _enum_kind, _enum_default)                                                                     \
  magic_enum::enum_cast<_enum_kind>(utils::str_to_snake(tbl.at_path(_path).value_or("")))                              \
      .value_or(_enum_kind::_enum_default)

#define get_flags(_path, _flag_kind, _flag_default)                                                                    \
  magic_enum::enum_flags_cast<_flag_kind>(utils::str_to_snake(tbl.at_path(_path).value_or("")))                        \
      .value_or(_flag_kind::_flag_default)

#define get_str(_path) tbl.at_path(_path).value_or("")

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

  out.project_name = get_str("project_name");

  out.profiles = utils::split_flags(get_str("profiles"));

  // target
  out.target.triple.arch     = get_enum("target.arch", env::EArch, unknown);
  out.target.triple.platform = get_enum("target.platform", env::EPlatform, unknown);
  out.target.triple.vendor   = get_enum("target.vendor", env::EVendor, unknown);
  out.target.triple.abi      = get_enum("target.abi", env::EABI, unknown);
  out.target.cpu             = get_str("target.cpu");
  out.target.features        = get_flags("target.features", FCPUFeature, NONE);
  out.target.reloc_model     = get_enum("target.reloc_model", ERelocModel, PIC);
  out.target.code_model      = get_enum("target.code_model", ECodeModel, small);
  out.target.emits           = get_flags("target.emits", FEmit, bin);

  // profile
  out.profile.debug        = get_bool("profile.debug");
  out.profile.optimization = get_enum("profile.optimization", EOptimization, O0);

  // log
  out.log.logs  = get_flags("log.logs", FPass, NONE);
  out.log.level = get_enum("log.level", ELogLevel, DEFAULT);

  // warn
  out.warn.warns = get_flags("warning.warnings", FWarnMode, NONE);
  out.warn.level = get_enum("warning.level", EWarnLevel, W0);

  // diagnostic
  out.diagnostic.error_mode = get_enum("diagnostic.error.mode", EErrorMode, fail_fatal);
  out.diagnostic.out_format = get_enum("diagnostic.out.format", EDiagnosticFormat, userfriendly);

  // llvm
  out.llvm.verify_module = get_bool("llvm.verify_module");
  if (const toml::array* llvm_args = tbl.at_path("llvm.args").as_array()) {
    for (const auto& key : *llvm_args) out.llvm.args.emplace_back(key.value_or(""));
  }

  // directories
  out.dir.project  = resolve_path(get_str("directory.project"));
  out.dir.build    = resolve_path(get_str("directory.build"));
  out.dir.source   = resolve_path(get_str("directory.source"));
  out.dir.profile  = resolve_path(get_str("directory.profile"));
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

  if (const toml::array* clang_args = tbl.at_path("c_ffi.args").as_array()) {
    for (const auto& key : *clang_args) out.c_ffi.args.emplace_back(key.value_or(""));
  }

  return out;

#undef get_enum
#undef get_str
#undef get_bool
}

bool common::compiler::Options::write_config(std::string_view path, bool is_debug) noexcept
{
  fs::path p = path;

  auto btos = [](bool _in) -> std::string { return _in ? "true" : "false"; };

  std::string config_txt(common::OPTIONS_TEMPLATE);

  std::string _defines;
  for (auto& [key, val] : preprocessor.defines) std::format_to(std::back_inserter(_defines), "= \"{}\"\n", val);
  std::string _undefines;
  for (auto& val : preprocessor.undefines) std::format_to(std::back_inserter(_undefines), "\"{}\"\n", val);
  std::string _profiles;
  for (auto& elem : profiles) std::format_to(std::back_inserter(_profiles), "{}|", elem);
  _profiles = _profiles.substr(0, _profiles.size() - 1);

  std::string _llvm_args;
  for (auto& val : llvm.args) std::format_to(std::back_inserter(_llvm_args), "\"{}\",\n", val);
  std::string _clang_args;
  for (auto& val : c_ffi.args) std::format_to(std::back_inserter(_clang_args), "\"{}\",\n", val);

  const std::string _libc_version = std::format("{}.{}", c_ffi.libc_version.major, c_ffi.libc_version.minor);

  const auto _c_source = GET_FLAGS_NAME(c_ffi.c_source);

  std::map<std::string_view, std::string_view> config_params = {
      // project
      {"project_name",           project_name                                                                     },
      // target
      {"profiles",               _profiles                                                                        },
      {"target.arch",            GET_ENUM_NAME(target.triple.arch)                                                },
      {"target.platform",        GET_ENUM_NAME(target.triple.platform)                                            },
      {"target.vendor",          GET_ENUM_NAME(target.triple.vendor)                                              },
      {"target.abi",             GET_ENUM_NAME(target.triple.abi)                                                 },
      {"target.call_convention", GET_ENUM_NAME(target.call_convention)                                            },
      {"target.cpu",             target.cpu                                                                       },
      {"target.features",        GET_FLAGS_NAME(target.features)                                                  },
      {"target.code_model",      GET_ENUM_NAME(target.code_model)                                                 },
      {"target.reloc_model",     GET_ENUM_NAME(target.reloc_model)                                                },
      {"target.emits",           GET_FLAGS_NAME(target.emits)                                                     },
      // profile
      {"profile.debug",          is_debug ? "true" : btos(profile.debug)                                          },
      {"profile.optimization",   is_debug ? GET_ENUM_NAME(EOptimization::O3) : GET_ENUM_NAME(profile.optimization)},
      // logs
      {"log.logs",               is_debug ? GET_FLAGS_NAME(FPass::all) : GET_FLAGS_NAME(log.logs)                 },
      {"log.level",              is_debug ? GET_ENUM_NAME(ELogLevel::normal) : GET_ENUM_NAME(log.level)           },
      // warnings
      {"warn.warnings",          GET_FLAGS_NAME(warn.warns)                                                       },
      {"warn.level",             magic_enum::enum_name(warn.level).substr(1)                                      },
      {"diagnostic.error.mode",  GET_ENUM_NAME(diagnostic.error_mode)                                             },
      {"diagnostic.out.format",  GET_ENUM_NAME(diagnostic.out_format)                                             },
      // debug printer
      {"debug.debugs",           GET_FLAGS_NAME(debug.debugs)                                                     },
      // defines
      {"preprocessor.defines",   _defines                                                                         },
      // undefines
      {"preprocessor.undefines", _undefines                                                                       },
      // directories
      {"dir.project",            dir.project                                                                      },
      {"dir.build",              dir.build                                                                        },
      {"dir.source",             dir.source                                                                       },
      {"dir.profile",            dir.profile                                                                      },
      {"dir.vendor",             dir.vendor                                                                       },
      {"dir.ffi_json",           dir.ffi_json                                                                     },
      {"dir.binding",            dir.binding                                                                      },
      {"dir.compiler",           common::env::get_compilers_dir()                                                 },
      {"dir.stdlib",             common::env::get_stdlib_dir()                                                    },
      {"dir.packages",           common::env::get_packages_dir()                                                  },
      // llvm
      {"llvm.verify_module",     btos(llvm.verify_module)                                                         },
      {"llvm.args",              _llvm_args                                                                       },
      // c_ffi
      {"c_ffi.libc",             GET_ENUM_NAME(c_ffi.libc)                                                        },
      {"c_ffi.libc_version",     _libc_version                                                                    },
      {"c_ffi.std",              GET_ENUM_NAME(c_ffi.std)                                                         },
      {"c_ffi.c_source",         _c_source                                                                        },
      {"c_ffi.environment",      GET_ENUM_NAME(c_ffi.env)                                                         },
      {"c_ffi.disable_builtins", btos(c_ffi.disable_builtins)                                                     },
      {"c_ffi.strict_aliasing",  btos(c_ffi.strict_aliasing)                                                      },
      {"c_ffi.sysroot",          c_ffi.sysroot                                                                    },
      {"c_ffi.args",             _clang_args                                                                      },
  };

  common::utils::fmt_template(config_txt, config_params);

  try {
    std::ofstream f(p);
    f.clear();
    f << config_txt << std::flush;
    f.close();
  } catch (...) {
    return false;
  }

  return true;
}


std::string common::OPTIONS_TEMPLATE = R"(
# main tolza toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

# all fields will be stored as define element also


# ======================
# tolza-compiler section 
# ======================

# if empty, the workspace file name will be used
project_name                 = "%project_name"
# at ./profile/*.toml name to apply after this
# e.g. "debug", "release", "windows|debug", "linux|posix|release",
profiles                      = "%profiles"  
    
[target]
# enum: )" + env::EArch_names() +
                                       R"( ...
arch                         = "%target.arch"                
# enum: )" + env::EPlatform_names() +
                                       R"( ...
platform                     = "%target.platform"                  
# enum: )" + env::EVendor_names() +
                                       R"( ...
vendor                       = "%target.vendor"              
# enum: )" + env::EABI_names() +
                                       R"( ...
abi                          = "%target.abi"           
# text: overrides host cpu detection
cpu                          = "%target.cpu"                 
# flags: )" + compiler::FCPUFeature_names()
                                       +
                                       R"( ...
features                     = "%target.features"
# enum: )" + env::ECallConvention_names()
                                       +
                                       R"( ...
call_convention              = "%target.call_convention"
# enum: )" + compiler::ECodeModel_names()
                                       +
                                       R"( ...
code_model                   = "%target.code_model"          
# enum: )" + compiler::ERelocModel_names()
                                       +
                                       R"( ...
reloc_model                  = "%target.reloc_model"         
# flags: )" + compiler::FEmit_names() +
                                       R"( ...
emits                        = "%target.emits"               

[profile]
# the debug profile will override some options

# true = disable optimization, enable debug info
debug                        = %profile.debug
# enum: )" + compiler::EOptimization_names()
                                       +
                                       R"( ...
optimization                 = "%profile.optimization"       

[log]
# flags: )" + compiler::FPass_names() +
                                       R"( ...
logs                         = "%log.logs"        
# enum: )" + compiler::ELogLevel_names()
                                       + R"( ...
level                        = "%log.level"

[warning]
# flags: )" + compiler::FWarnMode_names()
                                       +
                                       R"( ...
warnings                     = "%warn.warnings"
# 1, 2, 3
level                        = %warn.level  

[debug]
# flags: ast
debugs                       = "%debug.debugs"

[diagnostic]
# enum: )" + compiler::EErrorMode_names()
                                       + R"( ...
error.mode = "%diagnostic.error.mode"
# enum: )" + compiler::EDiagnosticFormat_names()
                                       + R"( ...
out.format = "%diagnostic.out.format"

[preprocessor]
# text list : "my_undef", "my_other_undef", ...
undefines = [
  %preprocessor.undefines
]
[preprocessor.defines]
# map : key_name = "text_value"
%preprocessor.defines

[directory]
# all path
project                      = "%dir.project"
build                        = "%dir.build"  
source                       = "%dir.source"  
profile                      = "%dir.profile"  
vendor                       = "%dir.vendor"  
ffi_json                     = "%dir.ffi_json"  
binding                      = "%dir.binding"  
compiler                     = "%dir.compiler"  
stdlib                       = "%dir.stdlib"  
packages                     = "%dir.packages"

# ============
# LLVM section
# ============
# This configuration is strictly for LLVM passes and code generation.
# It does not affect your Tolza preprocessor.

[llvm]
# verify before AND after passes
verify_module                = %llvm.verify_module     
# custom raw flags passed to LLVM
# text list : "my_arg1", "my_arg2", ...
args = [                                      
  %llvm.args
]

# =============
# C ffi section
# =============
# This configuration is strictly for C ffi passes on C std lib

[c_ffi]
# enum: )" + env::ELibC_names() +
                                       R"( ...
libc                         = "%c_ffi.libc"
# major.minor format: e.g. 10.2
libc_version                 = "%c_ffi.libc_version"
# enum: )" + env::ECStandard_names() +
                                       R"( ...
std                          = "%c_ffi.std"
# flags: )" + env::FCSource_names() +
                                       R"( ...
c_source                     = "%c_ffi.c_source"
# enum: )" + env::EEnvironment_names() +
                                       R"( ...
environment                  = "%c_ffi.environment"
# true, false
disable_builtins             = %c_ffi.disable_builtins
# true, false
strict_aliasing              = %c_ffi.strict_aliasing
# path
sysroot                      = "%c_ffi.sysroot"
# custom raw flags passed to the c linker
# text list : "my_arg1", "my_arg2", ...
args = [
  %c_ffi.args
]
)";