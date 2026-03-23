#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;


bool pipeline_start_preprocessor(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
