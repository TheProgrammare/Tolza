#pragma once

#include "ScriptInfo.hpp"
#include <string>
#include <vector>
#include <memory>


std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string &target_file);
