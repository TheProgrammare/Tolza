#pragma once

#include <vector>
#include <memory>

#include "ScriptInfo.hpp"

void generate_AST_View(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos);
