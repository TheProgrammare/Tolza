#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;

bool pipeline_start_llvm_opti(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos);
