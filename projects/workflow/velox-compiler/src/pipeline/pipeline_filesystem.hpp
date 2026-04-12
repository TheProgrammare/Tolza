#pragma once

#include <vector>
#include <set>
#include <memory>

struct ScriptInfo;

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& p_target_dir);
std::vector<std::shared_ptr<ScriptInfo>>
pipeline_start_filesystem_on_files(const std::set<std::string>& p_target_files);
