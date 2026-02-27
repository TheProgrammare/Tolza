#pragma once

#include <memory>
#include <vector>

#include "Compiler/ScriptInfo.hpp"

bool pipeline_start_LLVM_IR(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
