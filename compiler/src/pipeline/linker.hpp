#pragma once

namespace linker
{
bool link_modules() noexcept;
bool link_executable() noexcept;
bool emit() noexcept;
} // namespace linker