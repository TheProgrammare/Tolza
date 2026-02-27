#pragma once

#include <memory>
#include <vector>

#include "compiler/script_info.hpp"

void generate_AST_View(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
