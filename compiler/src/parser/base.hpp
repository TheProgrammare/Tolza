/*
 *	The Tolza programming language - Apache License, Version 2.0
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

#include "ast/forward.hpp"
#include "id/nodeid.hpp"
#include "nexus/forward.hpp"

#include <cstddef>
#include <string_view>
#include <tuple>



namespace parser
{
struct Parser_Context;

struct Parser_Base final {
  Parser_Base() = delete;
  Parser_Base(Parser_Context& p_ctx);
  ~Parser_Base();


  [[nodiscard]] ast::ID parse_import();
  [[nodiscard]] ast::ID parse_export();
  [[nodiscard]] ast::ID parse_reexport();
  [[nodiscard]] ast::ID parse_extern();

  [[nodiscard]] ast::ID regex_path();
  [[nodiscard]] ast::ID identifier(bool p_no_qualified_id = false, bool p_keyword_allowed = false);
  [[nodiscard]] std::tuple<ast::ID, type::ID> identifier_typed();

  [[nodiscard]] ast::ID parse_instruction();

  [[nodiscard]] ast::Local_Parameter&  inject_parameter(ast::ID parent_callable, size_t pos, std::string_view name,
                                                        ast::EPassMode passmode, type::ID tyid);
  [[nodiscard]] ast::Local_Variable&   inject_variable(std::string_view name, ast::EVariableKind kind, type::ID tyid,
                                                       ast::ID expr);
  [[nodiscard]] ast::Local_Capability& inject_capability(std::string_view name, ast::ECapability capa, type::ID tyid,
                                                         ast::ID expr);

  Parser_Context& p;
};
} // namespace parser
