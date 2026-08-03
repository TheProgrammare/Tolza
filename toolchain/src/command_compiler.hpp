#pragma once

#include <string>

namespace command::compiler
{

void apply_compiler(std::string_view file) noexcept;
void cogito_compiler(std::string_view file) noexcept;

} // namespace command::compiler
