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
#include "ast/definition.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "type/forward.hpp"

#include <cassert>
#include <memory_resource>
#include <vector>

namespace ast
{

constexpr size_t NODEID_USER_START = 3;


struct NodeEntry final {
  void*     ptr;
  ID        id;
  ENodeKind kind;

  explicit NodeEntry(void* p_ptr, ID p_id, ENodeKind p_kind)
    : ptr(p_ptr)
    , id(p_id)
    , kind(p_kind)
  {
  }
};

struct Arena final {
  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
    (void)add_get<ast::Root>();
  }

  bool freeze = false;

  cu::ID cuid;

  std::vector<NodeEntry> nodes;

  template <Generic T>
  [[nodiscard]] T& add_get() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    ID id = add<T>();

    return *as<T>(id);
  }

  template <Generic T>
  [[nodiscard]] ID add() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    void* mem = resource.allocate(sizeof(T), alignof(T));

    T* node = new (mem) T();

    ID id = get_next_id();

    node->header.nodeid = id;

    nodes.emplace_back(NodeEntry(mem, id, T::static_kind));

    return id;
  }

  [[nodiscard]] ID get_next_id() const noexcept;

  [[nodiscard]] NodeHeader& get(ID id) noexcept
  {
    auto& entry = nodes[id.index()];

    return *static_cast<NodeHeader*>(entry.ptr);
  }

  [[nodiscard]] const NodeHeader& get(ID id) const noexcept
  {
    assert(id.index() < nodes.size());
    const auto& entry = nodes[id.index()];

    return *static_cast<const NodeHeader*>(entry.ptr);
  }


  template <Generic T>
  [[nodiscard]] T* as(ID id)
  {
    assert(id.index() < nodes.size());
    auto& entry = nodes[id.index()];
    if (entry.kind == T::static_kind) return static_cast<T*>(entry.ptr);

    return nullptr;
  }

  template <Generic T>
  [[nodiscard]] const T* as(ID id) const
  {
    assert(id.index() < nodes.size());
    const auto& entry = nodes[id.index()];
    if (entry.kind == T::static_kind) return static_cast<T*>(entry.ptr);

    return nullptr;
  }


  [[nodiscard]] Root* get_file_root() noexcept
  {
    return as<Root>(ID::make(cuid, 0));
  }

private:
  std::pmr::monotonic_buffer_resource resource;
};


[[nodiscard]] std::string            get_decl_name(ID nodeid) noexcept;
[[nodiscard]] EVisibility            get_decl_visibility(ID nodeid) noexcept;
[[nodiscard]] std::string            get_mangled_id(ID id) noexcept;
[[nodiscard]] std::string            debug_node_on_line(ID id) noexcept;
[[nodiscard]] std::string            dump_debug(ID id) noexcept;
[[nodiscard]] std::string            dump(ID id) noexcept;
[[nodiscard]] const type::Prototype* get_prototype(ID id) noexcept;
[[nodiscard]] std::vector<ast::ID>   get_parameters(ast::ID nodeid) noexcept;
[[nodiscard]] type::ID               get_decl_type(ast::ID nodeid) noexcept;


constexpr ID NODEID_BUILTIN_Member_Access_Data = ID::make(cu::ID::main(), 0);
constexpr ID NODEID_BUILTIN_Member_Access_Len  = ID::make(cu::ID::main(), 1);
constexpr ID NODEID_BUILTIN_Member_Access_Capa = ID::make(cu::ID::main(), 2);


} // namespace ast