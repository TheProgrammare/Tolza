#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;


bool pipeline_start_LLVM_IR(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
