#pragma once

#include <string_view>

namespace notification
{
void notify(std::string_view title, std::string_view msg, bool success) noexcept;
} // namespace notification