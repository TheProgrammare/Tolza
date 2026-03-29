#pragma once

#include <string>

namespace common
{
struct CompCtx;
}

namespace command
{
namespace workspace
{

void err(const std::string& msg);
void log(const std::string& msg);

std::string generate_velox_workspace(const std::string& project_name, const std::string& path, bool force = false);
std::string write_file(const std::string& path, const std::string& text, bool verbose = true);
std::string write_config_file(const std::string& path, const std::string& name, bool file_debug_mode);
void        ask_new_workspace(const std::string& ws_path, const std::string& name);
std::string new_velox_workspace();
std::string compiler_context_to_config(const common::CompCtx& ctx);

} // namespace workspace
} // namespace command