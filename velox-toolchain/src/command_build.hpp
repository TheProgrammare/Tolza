#pragma once

#include <optional>

#include "toolchain.hpp"

std::string remove_quotes(const std::string& str);

namespace command
{
namespace build
{

void err(const std::string& msg);

std::optional<toolchain::CompCtx> init_compilation_context(const std::string& path);
toolchain::CompCtx                parse_compilation_context(const std::string& config_path);
void parse_args_for_compilation_context(toolchain::CompCtx& ctx, int start_arg, int argc, const char* argv[]);
bool generate_ffi_json(const std::string& compiler_file, const std::string& target_dir, const std::string& dest_dir);
bool start_compilation(const toolchain::CompCtx& ctx);

} // namespace build
} // namespace command