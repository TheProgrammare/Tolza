#pragma once

#include <memory>
#include <vector>

#include "compiler/script_info.hpp"

bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);