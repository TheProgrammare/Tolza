#pragma once

#include <string_view>

namespace common
{

#ifndef TOLZA_VERSION
inline constexpr std::string_view TOLZA_VERSION = "2026-08b";
#endif

// unrecoverable error : program stopped
[[noreturn]] void FATAL_ERROR(std::string_view msg) noexcept;

} // namespace common
