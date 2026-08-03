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
#include <cstdint>
#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ids.hpp"
#include "nexus/type/forward.hpp"
#include "nexus/ast/definition.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"

namespace ast
{

struct NodeEntry {
private:
  uint32_t  _offset;
  ENodeKind _kind;

public:
  NodeEntry(uint32_t p_offset, ENodeKind p_kind)
    : _offset(p_offset)
    , _kind(p_kind)
  {
  }
  [[nodiscard]] ENodeKind kind() const noexcept
  {
    return _kind;
  }
  [[nodiscard]] uint32_t offset() const noexcept
  {
    return _offset;
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

  std::vector<std::byte> storage;
  std::vector<NodeEntry> nodes;

  template <typename T>
  [[nodiscard]] T& add_get() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    uint32_t mem_offset = allocate(sizeof(T), alignof(T));

    T* node = new (&storage[mem_offset]) T();

    ID id = get_next_id();

    node->header.nodeid = id;

    nodes.emplace_back(NodeEntry(mem_offset, T::static_kind));

    return *as<T>(id);
  }

  template <typename T>
  [[nodiscard]] ID add() noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    uint32_t mem_offset = allocate(sizeof(T), alignof(T));

    T* node = new (&storage[mem_offset]) T();

    ID id = get_next_id();

    node->header.nodeid = id;

    nodes.emplace_back(NodeEntry(mem_offset, T::static_kind));

    return id;
  }

  [[nodiscard]] ID get_next_id() const noexcept;

  [[nodiscard]] NodeHeader& get(ID id) noexcept
  {
    auto& entry = nodes[id.index()];

    return *reinterpret_cast<NodeHeader*>(&storage[entry.offset()]);
  }

  [[nodiscard]] const NodeHeader& get(ID id) const noexcept
  {
    const auto& entry = nodes[id.index()];

    return *reinterpret_cast<const NodeHeader*>(&storage[entry.offset()]);
  }


  template <typename T>
  [[nodiscard]] T* as(ID id)
  {
    assert(id.index() < nodes.size());
    auto& entry = nodes[id.index()];

    if (entry.kind() != T::static_kind) return nullptr;

    return reinterpret_cast<T*>(&storage[entry.offset()]);
  }

  template <typename T>
  [[nodiscard]] const T* as(ID id) const
  {
    const auto& entry = nodes[id.index()];

    if (entry.kind() != T::static_kind) return nullptr;

    return reinterpret_cast<const T*>(&storage[entry.offset()]);
  }


  [[nodiscard]] Root* get_file_root() noexcept
  {
    return as<Root>(ID::make(cuid, 0));
  }

private:
  [[nodiscard]] uint32_t allocate(size_t size, size_t alignment)
  {
    const auto current = reinterpret_cast<uintptr_t>(storage.data() + storage.size());

    const auto aligned = (current + alignment - 1) & ~(alignment - 1);

    const auto padding = aligned - current;

    storage.resize(storage.size() + padding + size);

    return static_cast<uint32_t>(storage.size() - size);
  }
};


[[nodiscard]] std::string            get_decl_name(ID nodeid) noexcept;
[[nodiscard]] EVisibility            get_decl_visibility(ID nodeid) noexcept;
[[nodiscard]] std::string            get_mangled_id(ID id) noexcept;
[[nodiscard]] std::string            debug_node_on_line(ID id) noexcept;
[[nodiscard]] std::string            dump_debug(ID id) noexcept;
[[nodiscard]] std::string            dump(ID id) noexcept;
[[nodiscard]] const type::Prototype* get_prototype(ID id) noexcept;
[[nodiscard]] std::vector<ast::ID>   get_parameters(ast::ID nodeid) noexcept;


} // namespace ast