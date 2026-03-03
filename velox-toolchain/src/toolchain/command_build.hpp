#pragma once

#include <optional>
#include <filesystem>

#include "compilation.hpp"

namespace fs = std::filesystem;

std::string remove_quotes(const std::string& str);

namespace command
{
namespace build
{

std::optional<CompCtx> init_compilation_context(const fs::path& path);
CompCtx                parse_compilation_context(const fs::path& config_path);
void                   parse_args_for_compilation_context(CompCtx& ctx, int start_arg, int argc, const char* argv[]);

} // namespace build
} // namespace command