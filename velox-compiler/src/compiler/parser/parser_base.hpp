/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "compiler/ast/ast_codeblock_instruction.hpp"
#include "compiler/ast/ast_forward.hpp"

struct ScriptInfo;
struct ModuleImportation;

namespace parser
{
struct Parser_Context;

struct Parser_Base {
  Parser_Base(ScriptInfo& scr_info);
  ~Parser_Base();

  [[nodiscard]] std::vector<std::string> start_parsing();

  [[nodiscard]] ModuleImportation*                        parse_import();
  [[nodiscard]] std::shared_ptr<ast::declaration::Export> parse_export();

  [[nodiscard]] std::optional<ast::CodeBlock_instruction> parse_instruction();

  Parser_Context* ctx;
};
} // namespace parser
