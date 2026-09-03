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

#include "ast/definition.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/forward.hpp"

#include <cstdint>
#include <vector>

namespace cu
{
enum class EFileSource : uint8_t;
}


namespace ast
{


struct CodeBlock final {
  NODE_HEADER(CodeBlock);

  SET_VECTOR_NODE(elements);
};

struct Symbol_Id final {
  NODE_HEADER(Symbol_Id);

  std::string name;
};

struct Symbol_Qualified final {
  NODE_HEADER(Symbol_Qualified);

  std::string              name;
  std::vector<std::string> path;

  EPathAnchor anchor = EPathAnchor::relative_self;
};

// for every node who need a type resolution
struct Symbol_Type final {
  NODE_HEADER(Symbol_Type);

  SET_NODE(name);
  SET_VECTOR_TYPE(generic_args)
};

struct Path_Regex final {
  NODE_HEADER(Path_Regex);
  // a::b::c
  std::vector<std::string> path;

  // src::, std::, pkg::, bind::, vendor::, self::
  cu::EFileSource source = static_cast<cu::EFileSource>(0);

  // my_mod::{ a, b, c }
  std::vector<std::string> elements;

  // my_mod::*
  bool all_elements = false;
};

struct Import final {
  NODE_HEADER(Import);

  SET_NODE(regex);

  // one or multiple if multiple elements
  std::string alias;
};


} // namespace ast
  // AST
