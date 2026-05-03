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

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace parser
{
struct Parser_Context;

struct Parser_Base final {
  Parser_Base() = delete;
  Parser_Base(Parser_Context& p_ctx);
  ~Parser_Base();


  [[nodiscard]] ast::_gnid parse_import();
  [[nodiscard]] ast::_gnid parse_export();
  [[nodiscard]] ast::_gnid parse_reexport();
  [[nodiscard]] ast::_gnid parse_extern();

  [[nodiscard]] ast::_gnid regex_path();
  [[nodiscard]] ast::_gnid identifier(bool p_no_qualified_id = false, bool p_keyword_allowed = false);
  [[nodiscard]] std::tuple<ast::_gnid, type::_id> identifier_typed();

  [[nodiscard]] ast::_gnid parse_instruction();

  Parser_Context& p;
};
} // namespace parser
