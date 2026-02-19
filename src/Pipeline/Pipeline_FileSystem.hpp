#pragma once

#include "ScriptInfo.hpp"
#include <memory>
#include <string>
#include <vector>

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& target_file);
