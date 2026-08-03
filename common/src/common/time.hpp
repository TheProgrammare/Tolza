#pragma once

#include <string>


namespace common::time
{

[[nodiscard]] std::string now_datetime() noexcept;
[[nodiscard]] std::string now_date() noexcept;
[[nodiscard]] std::string now_time() noexcept;

} // namespace common::time