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

#include "nexus/ast/ast.hpp"
#include <set>
#include <string_view>
#include <vector>


namespace ast
{

AST_NODE(ID)
{
  std::string_view name;
};

AST_NODE(ID_Qualified)
{
  std::string_view              name;
  std::vector<std::string_view> path;

  EPathSource src = EPathSource::NONE;
};

// for every node who need a type resolution
AST_NODE(ID_Typed)
{
  SET_NODE(name);
  SET_VECTOR_TYPE(generic_args)
};

AST_NODE(Path_Regex)
{ // a::b::c
  std::vector<std::string_view> path;

  // src::, std::, pkg::, bind::, vendor::, relative (my_mod::)
  script::EFileSource source;

  // my_mod::{ a, b, c }
  std::vector<std::string_view> elements;

  // my_mod::*
  bool all_elements = false;
};

AST_NODE(Import)
{
  SET_NODE(regex);

  // one or multiple if multiple elements
  std::vector<std::string_view> aliases;
};

AST_NODE(Root)
{
  SET_VECTOR_NODE(global_nodes);
};


} // namespace ast
  // AST
