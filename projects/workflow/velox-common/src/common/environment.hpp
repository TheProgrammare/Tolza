#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>


namespace common::env
{


[[nodiscard]] std::vector<std::string> find_all_compilers(std::string_view dir_search) noexcept;
[[nodiscard]] std::string              find_latest_compiler(std::string_view dir_search) noexcept;
[[nodiscard]] std::string find_compiler_version(std::string_view dir_search, std::string_view version) noexcept;


enum class EArch : uint8_t {
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


enum class EVendor : uint8_t { apple, pc, w64, unknown };


namespace PlatformDetection
{
inline constexpr EPlatform PLATFORM =
#if _WIN32
    EPlatform::windows;
#elif __APPLE__ && __MACH__
    EPlatform::macos;
#elif __linux__ || __gnu_linux__
    EPlatform::linux;
#elif __FreeBSD__
    EPlatform::freebsd;
#elif __OpenBSD__
    EPlatform::openbsd;
#elif __NetBSD__
    EPlatform::netbsd;
#elif __DragonFly__
    EPlatform::dragonflybsd;
#elif __ANDROID__
    EPlatform::android;
#elif __APPLE__ && defined(TARGET_OS_IPHONE)
    EPlatform::ios;
#elif __sun
    EPlatform::solaris;
#else
    EPlatform::unknown;
#endif

inline constexpr EArch ARCH =
#if __x86_64__ || _M_X64
    EArch::x86_64;
#elif __i386__ || _M_IX86
    EArch::x86_32;
#elif __aarch64__ || _M_aarch64
    EArch::aarch64;
#elif __arm__ || _M_ARM
    EArch::aarch32;
#elif __riscv && __riscv_xlen == 64
    EArch::riscv64;
#elif __riscv && __riscv_xlen == 32
    EArch::riscv32;
#elif defined(__powerpc64__)
    EArch::ppc64;
#elif defined(__powerpc__)
    EArch::ppc32;
#elif defined(__mips64)
    EArch::mips64;
#elif defined(__mips__)
    EArch::mips32;
#elif defined(__wasm64__)
    EArch::wasm64;
#elif defined(__wasm__)
        EArch::wasm32;
#elif defined(__sparc__) && defined(__arch64__)
        EArch::sparc64;
#else
        EArch::unknown;
#endif

inline constexpr EVendor VENDOR =
#if __APPLE__
    EVendor::apple;
#elif _WIN32
    EVendor::w64;
#elif __unix__
    EVendor::pc;
#else
    EVendor::unknown;
#endif

inline constexpr EABI ABI =
#if _WIN64
    EABI::msvc_x64;
#elif _WIN32
    EABI::msvc_x86;
#elif __APPLE__ && __aarch64__
    EABI::darwin_aarch64;
#elif __APPLE__
    EABI::sysv;
#elif __linux__ && __aarch64__
        EABI::aapcs64;
#elif __linux__ && __arm__
        EABI::aapcs;
#elif __linux__ && __riscv && __riscv_xlen == 64
        EABI::riscv_lp64;
#elif __linux__ && __riscv && __riscv_xlen == 32
        EABI::riscv_ilp32;
#elif __wasm__
        EABI::wasm32;
#elif __linux__ || __gnu_linux__
        EABI::gnu;
#else
        EABI::unknown;
#endif

inline constexpr size_t ARCH_SIZE = sizeof(void*) * 8;
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