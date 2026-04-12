#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;


bool pipeline_start_codegen(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos);
