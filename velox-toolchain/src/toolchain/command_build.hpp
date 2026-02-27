#pragma once

#include <optional>
#include <filesystem>

#include "toolchain/compilation.hpp"

namespace fs = std::filesystem;

namespace command
{
namespace build
{

std::optional<CompCtx> init_compilation_context(const fs::path& path);
CompCtx                parse_compilation_context(const fs::path& config_path);
void                   parse_args_for_compilation_context(CompCtx& ctx, int start_arg, int argc, const char* argv[]);

} // namespace build
} // namespace command