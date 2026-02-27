#pragma once

#include <memory>
#include <vector>

#include "Compiler/ScriptInfo.hpp"

bool pipeline_start_linker(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
