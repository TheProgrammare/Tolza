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

#include "ast/data.hpp"
#include "ast/node/base.hpp"
#include "compiler/compilation_unit.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "id/tokid.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

namespace cu
{
enum class EFileSource : uint8_t;
}


namespace ast
{

#define NODE_HEADER(name)                                                                                              \
  NodeHeader                 header      = NodeHeader(ENodeKind::name);                                                \
  static constexpr ENodeKind static_kind = ENodeKind::name;                                                            \
  [[nodiscard]] ID           nodeid() const noexcept                                                                   \
  {                                                                                                                    \
    return header.nodeid;                                                                                              \
  }


#define SET_CALLABLE                                                                                                   \
  SET_VECTOR_NODE(parameters);                                                                                         \
  SET_NODE(contract);                                                                                                  \
  SET_TYPE(prototype);                                                                                                 \
  SET_NODE(codeblock);                                                                                                 \
                                                                                                                       \
  common::env::ECallConvention call_convention;                                                                        \
                                                                                                                       \
  bool is_pure         = false;                                                                                        \
  bool is_explicit_ret = false;

#define SET_NODE(name)   ast::ID name;
#define SET_SYMBOL(name) definition::ID name;
#define SET_TYPE(name)   type::ID name;

#define SET_INFERRED_TYPE        type::ID inferred_type;
#define SET_INFERRED_TYPE_(name) type::ID name;

#define SET_VECTOR_NODE(name)   std::vector<ast::ID> name;
#define SET_VECTOR_SYMBOL(name) std::vector<definition::ID> name;
#define SET_VECTOR_TYPE(name)   std::vector<type::ID> name;

using Path = std::vector<std::string>;


struct NodeHeader final {
  ID        nodeid;
  scope::ID scpid;
  token::ID start_tokid;
  token::ID end_tokid;
  ENodeKind kind = ENodeKind::NONE;

  explicit NodeHeader(ENodeKind k)
    : kind(k)
  {
  }
};


struct Unknown final {
  NODE_HEADER(NONE);

  Unknown()
  {
    assert(false);
  }
};

struct Root final {
  NODE_HEADER(Root);

  SET_VECTOR_NODE(global_nodes);
};


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

  ast::EPathAnchor anchor = ast::EPathAnchor::relative_self;
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
