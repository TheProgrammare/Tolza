#pragma once

#include <memory>
#include <vector>

#include "Compiler/ScriptInfo.hpp"

bool pipeline_start_parser(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
