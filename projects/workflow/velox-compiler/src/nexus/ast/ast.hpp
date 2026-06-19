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

#include <cassert>
#include <cstddef>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <iostream>

#include "nexus/forward.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ids.hpp"

namespace ast
{

#define AST_NODE(name) struct name final : NodeBase<ENodeKind::name>


#define SET_CALLABLE                                                                                                   \
  SET_VECTOR_NODE(parameters);                                                                                         \
                                                                                                                       \
  SET_TYPE(prototype);                                                                                                 \
  SET_NODE(codeblock);                                                                                                 \
                                                                                                                       \
  common::compiler::ECallingConv call_convention;                                                                      \
                                                                                                                       \
  bool is_pure              = false;                                                                                   \
  bool is_explicit_ret_type = false;

#define SET_NODE(name)   ast::ID name;
#define SET_SYMBOL(name) symbol::ID name;
#define SET_TYPE(name)   type::ID name;

#define SET_INFERRED_TYPE        type::ID inferred_type;
#define SET_INFERRED_TYPE_(name) type::ID name;

#define SET_VECTOR_NODE(name)   std::vector<ast::ID> name;
#define SET_VECTOR_SYMBOL(name) std::vector<symbol::ID> name;
#define SET_VECTOR_TYPE(name)   std::vector<type::ID> name;

using Path = std::vector<std::string>;


struct Node {
  ID        nodeid;
  scope::ID scpid;
  token::ID node_token_id;

  [[nodiscard]] ENodeKind kind() const noexcept
  {
    return _kind;
  }

  Node(const Node&)            = delete;
  Node& operator=(const Node&) = delete;

protected:
  const ENodeKind _kind = ENodeKind::Unknown;
  Node(ENodeKind k)
    : _kind(k)
  {
  }

public:
  virtual ~Node() = default;
};


template <typename T>
concept DerivedNode = requires {
  { T::static_kind } -> std::convertible_to<ENodeKind>;
} && std::is_base_of_v<Node, T> && !std::is_same_v<Node, T>;

template <ENodeKind K>
struct NodeBase : Node {
  static constexpr ENodeKind static_kind = K;
  NodeBase()
    : Node(K)
  {
  }
};

AST_NODE(Unknown){

};

AST_NODE(Root)
{
  SET_VECTOR_NODE(global_nodes);
};

std::vector<ID> get_parameters(ID nodeid) noexcept;


struct Arena final {
  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
    (void)add_get<ast::Root>();
  }

  bool freeze = false;

  cu::ID cuid;

  std::vector<std::unique_ptr<Node>> nodes;

  template <DerivedNode T>
  [[nodiscard]] T& add_get() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    auto obj = std::make_unique<T>();
    T*   raw = obj.get();

    auto new_id = get_next_id();
    obj->nodeid = new_id;
    nodes.emplace_back(std::move(obj));
    return *raw;
  }

  template <DerivedNode T>
  [[nodiscard]] ID add() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    auto obj = std::make_unique<T>();

    auto new_id = get_next_id();
    obj->nodeid = new_id;
    nodes.emplace_back(std::move(obj));
    return new_id;
  }

  [[nodiscard]] ID get_next_id() const noexcept;

  [[nodiscard]] Node& get(ID id) noexcept
  {
    assert(id.offset() < nodes.size());
    return *nodes[id.offset()];
  }

  [[nodiscard]] const Node& get(ID id) const noexcept
  {
    assert(id.offset() < nodes.size());
    return *nodes[id.offset()];
  }

  template <DerivedNode T>
  [[nodiscard]] T* as(ID id)
  {
    auto& n = get(id);

    if (n.kind() != T::static_kind) return nullptr;

    return static_cast<T*>(&n);
  }

  template <DerivedNode T>
  [[nodiscard]] const T* as(ID id) const
  {
    const auto& n = get(id);

    if (n.kind() != T::static_kind) return nullptr;

    return static_cast<T*>(&n);
  }

  [[nodiscard]] Root* get_file_root() noexcept
  {
    return as<Root>(ID::make(cuid, 0));
  }
};

[[nodiscard]] Node& get(ID id) noexcept;

template <DerivedNode T>
[[nodiscard]] T* as(ID id)
{
  auto& n = get(id);

  if (n.kind() != T::static_kind) return nullptr;

  return static_cast<T*>(&n);
}


[[nodiscard]] std::string get_decl_name(ID nodeid) noexcept;

[[nodiscard]] EVisibility get_decl_visibility(ID nodeid) noexcept;

[[nodiscard]] std::string get_mangled_id(ID id) noexcept;

[[nodiscard]] std::string get_debug_str(ID id) noexcept;

} // namespace ast