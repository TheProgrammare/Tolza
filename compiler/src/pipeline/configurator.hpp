#pragma once

#include "nexus/forward.hpp"

#include <string_view>

namespace configurator
{

void init_compiler_OPTIONS(std::string_view manifest_path, const common::compiler::Profile& command_args) noexcept;

}
