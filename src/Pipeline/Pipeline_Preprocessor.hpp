#pragma once

#include <vector>
#include <memory>

#include "ScriptInfo.hpp"

bool pipeline_start_preprocessor(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos);
