#pragma once

#include <memory>
#include <vector>

#include "script_info.hpp"

bool pipeline_start_lexer(const std::vector<std::shared_ptr<ScriptInfo>>& fs_out);
