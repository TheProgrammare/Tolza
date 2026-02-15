#pragma once

#include <vector>
#include <memory>

#include "ScriptInfo.hpp"

bool pipeline_start_LLVM_IR(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos);
