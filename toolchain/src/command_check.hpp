#pragma once

#include <string_view>

namespace command::check
{

[[nodiscard]] bool check_tolza_config(std::string_view file, bool full_config, bool verbose = true) noexcept;
[[nodiscard]] bool check_workspace(std::string_view ws_path, bool verbose = true) noexcept;

} // namespace command::check