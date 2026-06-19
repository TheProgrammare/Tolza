#pragma once

#include <string>

namespace command::package
{

[[nodiscard]] bool install(std::string_view pkg_name) noexcept;
[[nodiscard]] bool remove(std::string_view pkg_name) noexcept;
[[nodiscard]] bool info(std::string_view pkg_name) noexcept;
[[nodiscard]] bool purge(std::string_view pkg_name) noexcept;
[[nodiscard]] bool check(std::string_view pkg_name) noexcept;
[[nodiscard]] bool list(std::string_view regex, bool only_installed, bool only_upgradable) noexcept;
[[nodiscard]] bool update() noexcept;
[[nodiscard]] bool upgrade() noexcept;
[[nodiscard]] bool clean() noexcept;

} // namespace command::package