#pragma once

#include "version.hpp.in"

#include <string>
#include <string_view>

namespace common
{

constexpr const char* VELOX_COMMON_VERSION = "2026.2.0b";

const std::string        SOFTWARE_VERSION = IN_SOFTWARE_VERSION;
extern const std::string SOFTWARE_NAME;

inline const std::string SOFTWARE_ABOUT = 
SOFTWARE_NAME + "\n"
"  Version: " + SOFTWARE_VERSION + "\n"
"  Build date: " __DATE__ " " __TIME__ "\n"
"  License: Apache License, Version 2.0\n"
"  Author: Foz Florian\n";


// unrecoverable error : program stopped
[[noreturn]] void FATAL_ERROR(std::string_view msg) noexcept;

} // namespace common
