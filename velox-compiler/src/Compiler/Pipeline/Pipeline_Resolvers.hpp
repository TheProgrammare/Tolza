#pragma once

#include "Compiler/ScriptInfo.hpp"
#include <memory>
#include <vector>

bool pipeline_start_resolvers(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
