#pragma once

#include <vector>
#include <set>
#include <memory>

struct ScriptInfo;

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& target_file);
std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem_on_files(const std::set<std::string>& target_files);
