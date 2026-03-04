#pragma once

#include "script_info.hpp"
#include <vector>
#include <memory>

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const fs::path& target_file);
