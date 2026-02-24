#pragma once

#include <memory>
#include <vector>

#include "ScriptInfo.hpp"

void generate_AST_View(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
