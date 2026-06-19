#pragma once

#include <string>


namespace cli
{

[[nodiscard]] bool        is_valid_filename(std::string_view name) noexcept;
void                      sanitize_filename(std::string& name) noexcept;
[[nodiscard]] bool        yes_no_question(std::string_view msg) noexcept;
[[nodiscard]] std::string get_input(std::string_view msg) noexcept;
[[nodiscard]] std::string ask_filename() noexcept;

} // namespace cli