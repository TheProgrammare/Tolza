#pragma once

#include <memory>
#include <vector>

#include "script_info.hpp"

bool pipeline_start_preprocessor(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
