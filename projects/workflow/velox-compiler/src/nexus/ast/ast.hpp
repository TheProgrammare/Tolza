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
#include <unordered_map>
#include <vector>

#include "compiler/compiler.hpp"
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
  bool is_const             = false;                                                                                   \
  bool is_pure              = false;                                                                                   \
  bool is_explicit_ret_type = false;

#define SET_NODE(name)   ast::_gnid name;
#define SET_SYMBOL(name) symbol::_id name;
#define SET_TYPE(name)   type::_id name;

#define SET_INFERRED_TYPE        type::_id inferred_type;
#define SET_INFERRED_TYPE_(name) type::_id name;

#define SET_VECTOR_NODE(name)   std::vector<ast::_gnid> name;
#define SET_VECTOR_SYMBOL(name) std::vector<symbol::_id> name;
#define SET_VECTOR_TYPE(name)   std::vector<type::_id> name;

using Path = std::vector<std::string>;


struct Node {
  _gnid      node_id;
  scope::_id scope_id;
  token::_id node_token_id;

private:
  ENodeKind node_kind = ENodeKind::Unknown;

  Node(const Node&)            = delete;
  Node& operator=(const Node&) = delete;

protected:
  explicit Node(ENodeKind kind)
    : node_kind(kind)
  {
  }

public:
  ENodeKind kind() const
  {
    return node_kind;
  }

  virtual ~Node() = default;
};

template <ENodeKind K>
struct NodeBase : Node {
  static constexpr ENodeKind static_kind = K;

  NodeBase()
    : Node(K)
  {
  }
};

struct IDN final : NodeBase<ENodeKind::Unknown> {
};

AST_NODE(Unknown){

};

template <typename T>
concept DerivedNode = requires {
  { T::static_kind } -> std::convertible_to<ENodeKind>;
} && std::is_base_of_v<Node, T> && !std::is_same_v<Node, T>;


struct GNID_Factory final {
  static _gnid make_gnid(script::_id scr, _id n)
  {
    return _gnid((size_t(scr.value()) << 32) | n.value());
  }

  static std::tuple<script::_id, _id> to_literal(_gnid gnid)
  {
    script::_id scr(gnid.value() >> 32);
    _id         n(gnid.value() & 0xFFFFFFFF);

    return {scr, n};
  }
};


struct Arena final {
  struct Tools final {
    Arena& arena;

    [[nodiscard]] std::string get_declaration_mangle_name(_gnid gnid) const;
  };
  Tools tools{*this};

  Node& get(script::_id scr, _id n)
  {
    auto id = GNID_Factory::make_gnid(scr, n);
    return get(id);
  }

  Node& get(_gnid gnid);

  template <typename T>
  T* get_as(_gnid gnid)
  {
    auto& n = get(gnid);

    if (n.kind() != T::static_kind) return nullptr;

    return static_cast<T*>(&n);
  }
};


struct ScriptArena final {
  struct Tools final {
    ScriptArena& arena;

    [[nodiscard]] std::string_view get_node_declaration_name(_id node_id) const;
  };

  Tools tools{*this};

  script::ScriptInfo& scr;

  std::vector<std::unique_ptr<Node>> nodes;

  template <DerivedNode T>
  _id add()
  {
    auto obj = std::make_unique<T>();

    auto new_id  = get_next_gnid();
    obj->node_id = new_id;
    nodes.push_back(std::move(obj));
    return new_id.get_node_id();
  }

  _gnid get_next_gnid() const;

  Node& get(_id id)
  {
    assert(id < nodes.size());
    return *nodes[id.value()];
  }

  template <DerivedNode T>
  T* get_as(_id id)
  {
    auto& n = get(id);

    if (n.kind() != T::static_kind) return nullptr;

    return static_cast<T*>(&n);
  }
};

} // namespace ast