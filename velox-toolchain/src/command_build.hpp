#pragma once

#include <optional>
#include <filesystem>

#include "toolchain.hpp"

namespace fs = std::filesystem;

std::string remove_quotes(const std::string& str);

namespace command
{
namespace build
{

void err(const std::string& msg);

std::optional<toolchain::CompCtx> init_compilation_context(const fs::path& path);
toolchain::CompCtx                parse_compilation_context(const fs::path& config_path);
void parse_args_for_compilation_context(toolchain::CompCtx& ctx, int start_arg, int argc, const char* argv[]);
bool generate_ffi_json(const fs::path& compiler_file, const fs::path& target_dir, const fs::path& dest_dir);
bool start_compilation(const toolchain::CompCtx& ctx);

} // namespace build
} // namespace command