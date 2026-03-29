#pragma once

#include <string>
#include <map>

namespace command
{
namespace check
{
void err(const std::string& msg);
void log(const std::string& ms, bool sub_log = false);

bool check_velox_config(const std::string& file, bool full_config, bool verbose = true);
bool check_workspace(const std::string& ws_path, bool verbose = true);

} // namespace check
} // namespace command