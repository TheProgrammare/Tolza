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
#include "pool/stable_storage.hpp"

#include <cassert>
#include <cstddef>
#include <vector>

namespace ast
{

constexpr size_t NODEID_USER_START = 3;


struct NodeEntry final {
  void*     data;
  ID        id;
  ENodeKind kind;

  explicit NodeEntry(void* p_data, ID p_id, ENodeKind p_kind)
    : data(p_data)
    , id(p_id)
    , kind(p_kind)
  {
  }
};

struct Arena final {
  Arena(cu::ID _cuid);

  bool freeze = false;

  const cu::ID cuid;

  std::vector<NodeEntry> entries;

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

    const ID id   = ID::make(cuid, entries.size());
    T*       node = storage.create_get<T>();

    node->header.nodeid = id;
    entries.emplace_back(NodeEntry(node, id, T::static_kind));

    return id;
  }

  [[nodiscard]] NodeHeader& get(ID id) noexcept
  {
    assert(id.cu() == cuid);
    assert(id.index() < entries.size());

    return *storage.get<NodeHeader>(id.index());
  }

  [[nodiscard]] const NodeHeader& get(ID id) const noexcept
  {
    return const_cast<Arena*>(this)->get(id);
  }


  template <Generic T>
  [[nodiscard]] T* as(ID id)
  {
    assert(id.cu() == cuid);
    assert(id.index() < entries.size());

    auto& entry = entries[id.index()];
    if (entry.kind != T::static_kind) return nullptr;

    return static_cast<T*>(entry.data);
  }

  template <Generic T>
  [[nodiscard]] const T* as(ID id) const
  {
    return const_cast<Arena*>(this)->as<T>(id);
  }


  [[nodiscard]] ast::Root* get_file_root() noexcept;

private:
  StableStorage storage;
};


constexpr ID NODEID_BUILTIN_Member_Access_Data = ID::make(cu::ID::main(), 0);
constexpr ID NODEID_BUILTIN_Member_Access_Len  = ID::make(cu::ID::main(), 1);
constexpr ID NODEID_BUILTIN_Member_Access_Capa = ID::make(cu::ID::main(), 2);


} // namespace ast