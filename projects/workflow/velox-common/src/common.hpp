#pragma once

#include "version.hpp.in"
#include <string>
#include <vector>
#include <map>
#include <set>


namespace common
{

constexpr const char* VELOX_COMMON_VERSION = "2026.2.0b";

const std::string        SOFTWARE_VERSION = __SOFTWARE_VERSION__;
extern const std::string SOFTWARE_NAME;

inline const std::string SOFTWARE_ABOUT = 
SOFTWARE_NAME + "\n"
"  Version: " + SOFTWARE_VERSION + "\n"
"  Build date: " __DATE__ " " __TIME__ "\n"
"  License: Apache License, Version 2.0\n"
"  Author: Foz Florian";


std::string get_local_data_dir();
std::string get_cache_dir();
std::string get_templates_dir();

std::string get_compilers_dir();
std::string get_stdlib_dir();
std::string get_packages_dir();
std::string get_config_dir();

std::string get_exe_dir();

std::string resolve_path(const std::string& s, const std::string& relative = "");

inline constexpr char DETECTED_OS[] =
#if _WIN32
    "windows-msvc";
#elif __APPLE__ && __MACH__
    "darwin";
#elif __linux__ || __gnu_linux__
    "linux-gnu";
#else
    "unknown";
#endif

inline constexpr char DETECTED_ARCH[] =
#if __x86_64__ || _M_X64
    "x86_64";
#elif __i386 || _M_IX86
    "x86";
#elif __aarch64__ || _M_ARM64
    "aarch64";
#elif __arm__ || _M_ARM
    "arm";
#else
        "unknown";
#endif

inline constexpr char DETECTED_VENDOR[] =
#if __APPLE__
    "apple";
#elif __unix__
    "pc";
#elif _WIN32
    "pc";
#else
    "unknown";
#endif

inline constexpr char DETECTED_ABI[] =
#if __linux__ || __gnu_linux__
    "gnu";
#elif __APPLE__ && __MACH__
    "gnu";
#elif _WIN64
    "msvc";
#else
    "unknown";
#endif

inline constexpr size_t DETECTED_ARCH_SIZE = sizeof(void*) * 8;

namespace filesystem
{

inline const std::set<std::string> velox_extensions = {"vlx", "vlxbind", "vlxlib"};

bool                  is_velox_extension(const std::string& extension);
bool                  is_velox_file(const std::string& file_path);
std::set<std::string> find_velox_files(const std::string& target_dir, bool is_recursive);

} // namespace filesystem

std::vector<std::string> get_compiler_dirs();

void fmt_template(std::string& template_str, const std::initializer_list<std::string>& args);
void fmt_template(std::string& template_str, const std::map<std::string, std::string>& args);

} // namespace common