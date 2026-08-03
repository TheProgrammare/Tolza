#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

#include <Neargye/magic_enum.hpp>
#include <Neargye/magic_enum_flags.hpp>


namespace common::env
{


[[nodiscard]] std::vector<std::string> find_all_compilers(std::string_view dir_search) noexcept;
[[nodiscard]] std::string              find_latest_compiler(std::string_view dir_search) noexcept;
[[nodiscard]] std::string find_compiler_version(std::string_view dir_search, std::string_view version) noexcept;


enum class EArch : uint8_t {
  DEFAULT,
  x86_64,
  x86_32,

  aarch64,
  aarch32,

  riscv64,
  riscv32,

  ppc64,
  ppc32,

  mips64,
  mips32,

  wasm64,
  wasm32,

  sparc64,

  unknown,
  custom
};

constexpr bool is_32bit(EArch arch) noexcept
{
  switch (arch) {
  case EArch::x86_32:
  case EArch::aarch32:
  case EArch::riscv32:
  case EArch::ppc32:
  case EArch::mips32:
  case EArch::wasm32:  return true;

  default:             return false;
  }
}

enum class EPlatform : uint8_t {
  DEFAULT,
  linux, // yes linux is a kernel
  macos,
  windows,

  freebsd,
  openbsd,
  netbsd,

  dragonflybsd,

  android,
  ios,

  solaris,

  unknown,
  custom
};


[[nodiscard]] constexpr bool supports_posix(EPlatform platform) noexcept
{
  switch (platform) {
  case EPlatform::linux:
  case EPlatform::macos:
  case EPlatform::freebsd:
  case EPlatform::openbsd:
  case EPlatform::netbsd:
  case EPlatform::dragonflybsd:
  case EPlatform::android:
  case EPlatform::ios:
  case EPlatform::solaris:      return true;

  default:                      return false;
  }
}

[[nodiscard]] constexpr bool supports_gnu(EPlatform platform) noexcept
{
  switch (platform) {
  case EPlatform::linux: return true;

  default:               return false;
  }
}

[[nodiscard]] constexpr bool supports_bsd(EPlatform platform) noexcept
{
  switch (platform) {
  case EPlatform::freebsd:
  case EPlatform::openbsd:
  case EPlatform::netbsd:
  case EPlatform::dragonflybsd:

  case EPlatform::macos:
  case EPlatform::ios:          return true;

  default:                      return false;
  }
}

[[nodiscard]] constexpr bool supports_darwin(EPlatform platform) noexcept
{
  switch (platform) {
  case EPlatform::macos:
  case EPlatform::ios:   return true;

  default:               return false;
  }
}
enum class EABI : uint8_t {
  DEFAULT,
  sysv,  // System V ABI (x86_64 Linux/BSD default)
  win64, // Windows x86_64 ABI

  gnu, // GNU ABI (Linux GNU toolchains, musl/gnu distinction souvent implicite)

  aapcs,   // ARM 32-bit procedure call standard
  aapcs64, // aarch64 ABI (AArch64)

  darwin_arm64, // Apple arm/aarch64 ABI (macOS/iOS)

  msvc_x86, // MSVC 32-bit ABI
  msvc_x64, // MSVC 64-bit ABI

  riscv_ilp32, // RV32 (ILP32 ABI)
  riscv_lp64,  // RV64 (LP64 ABI)

  wasm32, // WebAssembly 32-bit ABI

  unknown,
  custom
};


enum class ECallConvention : uint8_t {
  DEFAULT,
  cdecl,
  stdcall,
  fastcall,
  thiscall,

  sysv,
  sysv_x86_64,
  sysv_x86_32,

  win64,

  riscv_lp64,
  riscv_ilp32,

  aapcs,
  aapcs64,
  aapcs_vfp,

  vectorcall,

  wasm32,

  custom,
  unknown
};


[[nodiscard]] constexpr ECallConvention call_convention(EPlatform platform, EArch arch, EABI abi) noexcept
{
  // ABI explicite
  switch (abi) {
  case EABI::win64:
  case EABI::msvc_x64:     return ECallConvention::win64;

  case EABI::aapcs:        return ECallConvention::aapcs;

  case EABI::aapcs64:
  case EABI::darwin_arm64: return ECallConvention::aapcs64;

  case EABI::riscv_ilp32:  return ECallConvention::riscv_ilp32;

  case EABI::riscv_lp64:   return ECallConvention::riscv_lp64;

  case EABI::wasm32:       return ECallConvention::wasm32;

  case EABI::msvc_x86:
    switch (arch) {
    case EArch::x86_32:
      // MSVC x86 n'a pas une convention unique.
      // cdecl est le comportement C par défaut.
      return ECallConvention::cdecl;

    default: return ECallConvention::unknown;
    }

  case EABI::sysv:
    switch (arch) {
    case EArch::x86_64: return ECallConvention::sysv_x86_64;
    case EArch::x86_32: return ECallConvention::sysv_x86_32;
    default:            return ECallConvention::sysv;
    }

  case EABI::gnu:
    switch (arch) {
    case EArch::x86_64:  return ECallConvention::sysv_x86_64;
    case EArch::x86_32:  return ECallConvention::sysv_x86_32;
    case EArch::aarch32: return ECallConvention::aapcs;
    case EArch::aarch64: return ECallConvention::aapcs64;
    case EArch::riscv32: return ECallConvention::riscv_ilp32;
    case EArch::riscv64: return ECallConvention::riscv_lp64;
    default:             return ECallConvention::unknown;
    }

  case EABI::unknown:
  case EABI::custom:
  case EABI::DEFAULT: break;
  }


  // Fallback plateforme + architecture
  switch (platform) {

  case EPlatform::windows:
    switch (arch) {
    case EArch::x86_64: return ECallConvention::win64;
    case EArch::x86_32: return ECallConvention::cdecl;
    default:            return ECallConvention::unknown;
    }


  case EPlatform::linux:
  case EPlatform::android:
  case EPlatform::freebsd:
  case EPlatform::openbsd:
  case EPlatform::netbsd:
  case EPlatform::dragonflybsd:
  case EPlatform::macos:
  case EPlatform::ios:
  case EPlatform::solaris:
    switch (arch) {

    case EArch::x86_64:  return ECallConvention::sysv_x86_64;
    case EArch::x86_32:  return ECallConvention::sysv_x86_32;
    case EArch::aarch32: return ECallConvention::aapcs;
    case EArch::aarch64: return ECallConvention::aapcs64;
    case EArch::riscv32: return ECallConvention::riscv_ilp32;
    case EArch::riscv64: return ECallConvention::riscv_lp64;
    case EArch::wasm32:  return ECallConvention::wasm32;
    default:             return ECallConvention::unknown;
    }


  case EPlatform::unknown:
  case EPlatform::custom:
  default:                 return ECallConvention::unknown;
  }
}


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


[[nodiscard]] constexpr FCSource c_source()
{
  uint16_t flags = 0;

#ifdef _GNU_SOURCE
  flags |= static_cast<uint16_t>(FCSource::gnu);
#endif

#ifdef _BSD_SOURCE
  flags |= static_cast<uint16_t>(FCSource::bsd);
#endif

#ifdef __APPLE__
  flags |= static_cast<uint16_t>(FCSource::darwin);
#endif

#ifdef _POSIX_C_SOURCE
  flags |= static_cast<uint16_t>(FCSource::posix);
#endif

#ifdef _XOPEN_SOURCE
  flags |= static_cast<uint16_t>(FCSource::xopen);
#endif

#ifdef __linux__
  flags |= static_cast<uint16_t>(FCSource::linux);
#endif

#ifdef __ANDROID__
  flags |= static_cast<uint16_t>(FCSource::android);
#endif

#ifdef _CRT_SECURE_NO_WARNINGS
  flags |= static_cast<uint16_t>(FCSource::crt_secure_no_warnings);
#endif

  return static_cast<FCSource>(flags);
}


enum class EVendor : uint8_t { DEFAULT, apple, pc, w64, unknown };


enum class ECStandard : uint8_t { DEFAULT, c89, c99, c11, c17, c23, gnu89, gnu99, gnu11, gnu17, gnu23, unknown };

enum class EEnvironment : uint8_t { DEFAULT, gnu, musl, msvc, gnuabi, mingw, darwin, baremetal, wasi, custom, unknown };

enum class ELibC : uint8_t {
  DEFAULT,
  glibc,
  musl,
  libsystem,
  ucrt,
  msvcrt,
  mingw_libc,
  bionic,
  bsd_libc,
  custom,
  unknown
};

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

namespace PlatformDetection
{
inline constexpr EPlatform PLATFORM =
#if defined(__ANDROID__)
    EPlatform::android;
#elif defined(__APPLE__) && defined(__MACH__)
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    EPlatform::ios;
#else
    EPlatform::macos;
#endif
#elif defined(_WIN32)
    EPlatform::windows;
#elif defined(__linux__) || defined(__gnu_linux__)
    EPlatform::linux;
#elif defined(__FreeBSD__)
    EPlatform::freebsd;
#elif defined(__OpenBSD__)
    EPlatform::openbsd;
#elif defined(__NetBSD__)
    EPlatform::netbsd;
#elif defined(__DragonFly__)
    EPlatform::dragonflybsd;
#elif defined(__sun)
    EPlatform::solaris;
#else
    EPlatform::unknown;
#endif

inline constexpr EArch ARCH =
#if defined(__x86_64__) || defined(_M_X64)
    EArch::x86_64;
#elif defined(__i386__) || defined(_M_IX86)
    EArch::x86_32;
#elif defined(__aarch64__) || defined(_M_ARM64)
    EArch::aarch64;
#elif defined(__arm__) || defined(_M_ARM)
    EArch::aarch32;
#elif defined(__riscv)
#if __riscv_xlen == 64
    EArch::riscv64;
#else
    EArch::riscv32;
#endif
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(_M_PPC)
    EArch::ppc64;
#elif defined(__powerpc__) || defined(__ppc__)
    EArch::ppc32;
#elif defined(__mips64)
    EArch::mips64;
#elif defined(__mips__)
    EArch::mips32;
#elif defined(__wasm64__)
    EArch::wasm64;
#elif defined(__wasm32__) || defined(__wasm__)
        EArch::wasm32;
#elif defined(__sparc__) && defined(__arch64__)
        EArch::sparc64;
#elif defined(__sparc__)
        EArch::sparc32;
#else
        EArch::unknown;
#endif

inline constexpr EVendor VENDOR =
#if defined(__APPLE__)
    EVendor::apple;
#elif defined(_WIN32)
    EVendor::w64;
#elif defined(__sun)
    EVendor::sun;
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)                        \
    || defined(__DragonFly__)
    EVendor::pc;
#else
    EVendor::unknown;
#endif

inline constexpr EABI ABI =
#if defined(__wasm64__)
    EABI::wasm64;
#elif defined(__wasm32__) || defined(__wasm__)
    EABI::wasm32;
#elif defined(_WIN64)
    EABI::msvc_x64;
#elif defined(_WIN32)
    EABI::msvc_x86;
#elif defined(__APPLE__) && defined(__aarch64__)
    EABI::darwin_aarch64;
#elif defined(__APPLE__)
        EABI::sysv;
#elif defined(__riscv)
#if __riscv_xlen == 64
        EABI::riscv_lp64;
#else
        EABI::riscv_ilp32;
#endif
#elif defined(__aarch64__)
        EABI::aapcs64;
#elif defined(__arm__)
        EABI::aapcs;
#elif defined(__linux__) || defined(__gnu_linux__) || defined(__FreeBSD__) || defined(__OpenBSD__)                     \
    || defined(__NetBSD__) || defined(__DragonFly__)
        EABI::gnu;
#else
        EABI::unknown;
#endif

inline constexpr size_t ARCH_SIZE = sizeof(void*) * 8;


inline constexpr ELibC LIBC =
#if defined(__BIONIC__)
    ELibC::bionic;
#elif defined(__GLIBC__)
    ELibC::glibc;
#elif defined(__MUSL__)
    ELibC::musl;
#elif defined(__APPLE__)
    ELibC::libsystem;
#elif defined(__MINGW32__) || defined(__MINGW64__)
    ELibC::mingw_libc;
#elif defined(_UCRT)
    ELibC::ucrt;
#elif defined(_MSC_VER)
    ELibC::msvcrt;
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
    ELibC::bsd_libc;
#else
    ELibC::unknown;
#endif

inline constexpr EEnvironment ENVIRONMENT =
#if defined(__wasi__)
    EEnvironment::wasi;
#elif defined(__MINGW32__) || defined(__MINGW64__)
    EEnvironment::mingw;
#elif defined(_MSC_VER)
    EEnvironment::msvc;
#elif defined(__APPLE__)
    EEnvironment::darwin;
#elif defined(__MUSL__)
    EEnvironment::musl;
#elif defined(__GLIBC__)
    EEnvironment::gnu;
#elif defined(__ARM_EABI__) || defined(__ARM_PCS)
    EEnvironment::gnuabi;
#elif !defined(__STDC_HOSTED__)
    EEnvironment::baremetal;
#elif (__STDC_HOSTED__ == 0)
    EEnvironment::baremetal;
#else
        EEnvironment::unknown;
#endif


inline constexpr ECStandard C_STANDARD =
#if defined(__STDC_VERSION__)
#if defined(__STRICT_ANSI__)
#if __STDC_VERSION__ >= 202311L
    ECStandard::c23;
#elif __STDC_VERSION__ >= 201710L
    ECStandard::c17;
#elif __STDC_VERSION__ >= 201112L
    ECStandard::c11;
#elif __STDC_VERSION__ >= 199901L
    ECStandard::c99;
#else
    ECStandard::c89;
#endif
#else
#if __STDC_VERSION__ >= 202311L
    ECStandard::gnu23;
#elif __STDC_VERSION__ >= 201710L
    ECStandard::gnu17;
#elif __STDC_VERSION__ >= 201112L
    ECStandard::gnu11;
#elif __STDC_VERSION__ >= 199901L
    ECStandard::gnu99;
#else
    ECStandard::gnu89;
#endif
#endif
#else
    ECStandard::unknown;
#endif

inline constexpr ECallConvention CALL_CONVENTION = call_convention(PLATFORM, ARCH, ABI);

inline constexpr FCSource C_SOURCE = c_source();

}; // namespace PlatformDetection


[[nodiscard]] std::string_view get_local_data_dir() noexcept;
[[nodiscard]] std::string_view get_cache_dir() noexcept;
[[nodiscard]] std::string_view get_templates_dir() noexcept;

[[nodiscard]] std::string_view get_compilers_dir() noexcept;
[[nodiscard]] std::string_view get_stdlib_dir() noexcept;
[[nodiscard]] std::string_view get_packages_dir() noexcept;
[[nodiscard]] std::string_view get_config_dir() noexcept;

[[nodiscard]] std::string_view get_exe_dir() noexcept;

[[nodiscard]] std::vector<std::string> get_compiler_dirs() noexcept;

} // namespace common::env


template <>
struct magic_enum::customize::enum_range<common::env::FCSource> {
  static constexpr bool is_flags = true;
};

namespace common::env
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


constexpr std::string& EArch_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::EArch);
  return s;
}
constexpr std::string& EPlatform_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::EPlatform);
  return s;
}
constexpr std::string& EABI_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::EABI);
  return s;
}
constexpr std::string& EVendor_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::EVendor);
  return s;
}
constexpr std::string& ECallConvention_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::ECallConvention);
  return s;
}

constexpr std::string& ECStandard_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::ECStandard);
  return s;
}
constexpr std::string& FCSource_names()
{
  static auto s = GET_FLAGS_NAMES_TO_STRING(common::env::FCSource);
  return s;
}
constexpr std::string& EEnvironment_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::EEnvironment);
  return s;
}
constexpr std::string& ELibC_names()
{
  static auto s = GET_ENUM_NAMES_TO_STRING(common::env::ELibC);
  return s;
}

#undef GET_ENUM_NAMES_TO_STRING
#undef GET_FLAGS_NAMES_TO_STRING

} // namespace common::env