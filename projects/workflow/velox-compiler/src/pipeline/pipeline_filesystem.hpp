#pragma once

#include <vector>
#include <memory>

struct ScriptInfo;

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& target_file);
