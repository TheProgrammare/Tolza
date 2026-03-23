#pragma once

#include <string>

#include <compiler_context.hpp>

std::string remove_quotes(const std::string& str);

namespace command
{
namespace build
{

void err(const std::string& msg);

common::CompCtx init_compilation_context(const std::string& path);
common::CompCtx config_to_compilation_context(const std::string& config_path);
bool generate_ffi_json(const std::string& compiler_file, const std::string& target_dir, const std::string& dest_dir);
bool start_compilation(const common::CompCtx& ctx);

} // namespace build
} // namespace command