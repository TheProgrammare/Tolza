#pragma once

#include <memory>
#include <vector>

struct ScriptInfo;

bool pipeline_start_lexer(const std::vector<std::shared_ptr<ScriptInfo>>& fs_out);
