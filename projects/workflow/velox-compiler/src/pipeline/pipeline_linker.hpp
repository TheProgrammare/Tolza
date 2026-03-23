#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;

bool pipeline_start_linker(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
