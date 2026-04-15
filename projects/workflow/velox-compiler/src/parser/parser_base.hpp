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
#include <string>
#include <vector>

#include "ast/ast_forward.hpp"
#include "ast/ast_codeblock_instruction.hpp"
#include "misc/module_manager.hpp"

struct ScriptInfo;

namespace parser
{
struct Parser_Context;

struct Parser_Base {
  Parser_Base() = delete;
  Parser_Base(Parser_Context& p_ctx);
  ~Parser_Base();


  [[nodiscard]] std::shared_ptr<ast::declaration::Import>   parse_import();
  [[nodiscard]] std::shared_ptr<ast::declaration::Export>   parse_export();
  [[nodiscard]] std::shared_ptr<ast::declaration::ReExport> parse_reexport();
  [[nodiscard]] std::shared_ptr<ast::declaration::Extern>   parse_extern();

  [[nodiscard]] ast::CodeBlock_instruction parse_instruction();

  Parser_Context& ctx;
};
} // namespace parser
