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

#include "id/typeid.hpp"
#include "nexus/forward.hpp"
#include "pool/stable_storage.hpp"
#include "type/tool.hpp"
#include "type/type.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace type
{

// start of user type
constexpr uint64_t TYPEID_USER_START = 42;
constexpr uint64_t TYPEID_FFI_START  = UINT64_MAX / 2;

struct TypeEntry final {
  void*     data;
  ID        id;
  ETypeKind kind;

  explicit TypeEntry(void* p_data, ID p_id, ETypeKind p_kind)
    : data(p_data)
    , id(p_id)
    , kind(p_kind)

  {
  }
};

void initialization() noexcept;

inline std::unordered_map<ID, TypeEntry*, ID::Hash> primitives;


struct Factory final {
  Arena& arena;

  [[nodiscard]] ID make_primitive(EPrimitiveTypeKind p, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_ptr(ID inner, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_static_array(ID inner, size_t size, ast::ID size_expr, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_dynamic_array(ID inner, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_slice(ID inner, Qualifier qualifier = Qualifier(), bool is_c_table = false);
  [[nodiscard]] ID make_tuple(const std::vector<ID>& elems, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_prototype(const std::vector<type::Prototype_Param>& params, ID ret, bool is_variadic,
                                  Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_enum(const std::vector<ID>& variants, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_flag(size_t size, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_union(const std::vector<ID>& variants, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_facet(const std::vector<ID>& fields, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_view(const std::vector<ID>& facets, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_form(const std::vector<ID>& facets, definition::ID sym, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_forward_identifier(std::string_view name, Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_identifier(std::string_view name, ast::ID nodeid, definition::ID sym,
                                   Qualifier qualifier = Qualifier());
  [[nodiscard]] ID make_string(ETextType txt_ty, Qualifier qualifier = Qualifier());
};

struct Arena final {
  Arena(cu::ID _cuid);

  Factory factory{*this};

  bool freeze = false;

  const cu::ID cuid;

  // arena storage
  std::vector<TypeEntry*>                                entries;
  std::unordered_map<std::string, type::ID>              resolved_identifiers;
  std::unordered_map<type::ID, type::ID, type::ID::Hash> canon_identifier;

  // safe interning
  std::unordered_map<size_t, std::vector<ID>> buckets;

  // returns the canonical type
  [[nodiscard]] TypeHeader&       get(ID tyid) noexcept;
  // returns the canonical type
  [[nodiscard]] const TypeHeader& get(ID tyid) const noexcept;

  template <type::Generic T>
  [[nodiscard]] T* as(ID tyid) noexcept
  {
    assert(tyid.cu() == cuid);

    size_t index = tyid.index();

    if (index < TYPEID_USER_START) {
      const auto* entry = type::primitives.at(type::ID::make(cu::ID::main(), index));

      if (entry->kind != T::static_kind) return nullptr;

      return static_cast<T*>(entry->data);
    }

    index -= TYPEID_USER_START;

    // canonical
    if (auto it = canon_identifier.find(tyid); it != canon_identifier.end()) return as<T>(it->second);

    assert(index < entries.size());
    auto& entry = *entries[index];

    if (entry.kind != T::static_kind) return nullptr;

    return static_cast<T*>(entry.data);
  }

  template <type::Generic T>
  [[nodiscard]] const T* as(ID tyid) const noexcept
  {
    return const_cast<Arena*>(this)->as<T>(tyid);
  }

  [[nodiscard]] size_t size() const noexcept
  {
    return entries.size();
  }

  [[nodiscard]] bool is_canonical(std::string_view str) const noexcept
  {
    return !resolved_identifiers.contains(std::string(str));
  }

  void add_canon(std::string_view str, ID resolved) noexcept
  {
    resolved_identifiers.insert_or_assign(std::string(str), resolved);
  }

  void add_identifier_canon(type::ID tyid, type::ID canon) noexcept
  {
    canon_identifier.insert_or_assign(tyid, canon);
  }

  template <type::Generic T>
  [[nodiscard]] ID intern(T&& t) noexcept
  {
    const size_t h = hash_type(t);

    auto& bucket = buckets[h];

    // is type already exists
    for (ID id : bucket) {
      if (is_same_type(t, id)) {
        return id;
      }
    }

    const auto id = add(std::forward<T>(t), bucket);
    return id;
  }


private:
  template <type::Generic T>
  [[nodiscard]] ID add(T&& t, std::vector<ID>& bucket) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    using U = std::remove_cvref_t<T>;

    ID new_id = ID::make(cuid, entries.size() + TYPEID_USER_START);

    auto* entry = storage.create_get<TypeEntry>(TypeEntry(new U(std::forward<T>(t)), new_id, U::static_kind));

    static_cast<TypeHeader*>(entry->data)->tyid = new_id;

    entries.emplace_back(entry);
    bucket.emplace_back(new_id);

    return new_id;
  }

  StableStorage storage;

  size_t hash_type(type::Primitive const& d) noexcept;
  size_t hash_type(type::Ptr const& d) noexcept;
  size_t hash_type(type::Array const& d) noexcept;
  size_t hash_type(type::Buffer const& d) noexcept;
  size_t hash_type(type::Slice const& d) noexcept;
  size_t hash_type(type::Tuple const& d) noexcept;
  size_t hash_type(type::Prototype const& d) noexcept;
  size_t hash_type(type::Enum const& d) noexcept;
  size_t hash_type(type::Flag const& d) noexcept;
  size_t hash_type(type::Union const& d) noexcept;
  size_t hash_type(type::Facet const& d) noexcept;
  size_t hash_type(type::Form const& d) noexcept;
  size_t hash_type(type::View const& d) noexcept;
  size_t hash_type(type::Identifier const& d) noexcept;
  size_t hash_type(type::String const& d) noexcept;
};

constexpr ID BAD_TYPE_ID = ID::invalid();

constexpr ID TYPEID_u0      = ID::make_primitive(EPrimitiveTypeKind::_u0);
constexpr ID TYPEID_bool    = ID::make_primitive(EPrimitiveTypeKind::_bool);
constexpr ID TYPEID_cune    = ID::make_primitive(EPrimitiveTypeKind::_cune);
constexpr ID TYPEID_rune    = ID::make_primitive(EPrimitiveTypeKind::_rune);
constexpr ID TYPEID_ssize   = ID::make_primitive(EPrimitiveTypeKind::_ssize);
constexpr ID TYPEID_s8      = ID::make_primitive(EPrimitiveTypeKind::_s8);
constexpr ID TYPEID_s16     = ID::make_primitive(EPrimitiveTypeKind::_s16);
constexpr ID TYPEID_s32     = ID::make_primitive(EPrimitiveTypeKind::_s32);
constexpr ID TYPEID_s64     = ID::make_primitive(EPrimitiveTypeKind::_s64);
constexpr ID TYPEID_s128    = ID::make_primitive(EPrimitiveTypeKind::_s128);
constexpr ID TYPEID_usize   = ID::make_primitive(EPrimitiveTypeKind::_usize);
constexpr ID TYPEID_u8      = ID::make_primitive(EPrimitiveTypeKind::_u8);
constexpr ID TYPEID_u16     = ID::make_primitive(EPrimitiveTypeKind::_u16);
constexpr ID TYPEID_u32     = ID::make_primitive(EPrimitiveTypeKind::_u32);
constexpr ID TYPEID_u64     = ID::make_primitive(EPrimitiveTypeKind::_u64);
constexpr ID TYPEID_u128    = ID::make_primitive(EPrimitiveTypeKind::_u128);
constexpr ID TYPEID_bsize   = ID::make_primitive(EPrimitiveTypeKind::_bsize);
constexpr ID TYPEID_b8      = ID::make_primitive(EPrimitiveTypeKind::_b8);
constexpr ID TYPEID_b16     = ID::make_primitive(EPrimitiveTypeKind::_b16);
constexpr ID TYPEID_b32     = ID::make_primitive(EPrimitiveTypeKind::_b32);
constexpr ID TYPEID_b64     = ID::make_primitive(EPrimitiveTypeKind::_b64);
constexpr ID TYPEID_b128    = ID::make_primitive(EPrimitiveTypeKind::_b128);
constexpr ID TYPEID_ptrdiff = ID::make_primitive(EPrimitiveTypeKind::_ptrdiff);
constexpr ID TYPEID_fsize   = ID::make_primitive(EPrimitiveTypeKind::_fsize);
constexpr ID TYPEID_f16     = ID::make_primitive(EPrimitiveTypeKind::_f16);
constexpr ID TYPEID_f32     = ID::make_primitive(EPrimitiveTypeKind::_f32);
constexpr ID TYPEID_f64     = ID::make_primitive(EPrimitiveTypeKind::_f64);
constexpr ID TYPEID_f80     = ID::make_primitive(EPrimitiveTypeKind::_f80);
constexpr ID TYPEID_f128    = ID::make_primitive(EPrimitiveTypeKind::_f128);
constexpr ID TYPEID_dsize   = ID::make_primitive(EPrimitiveTypeKind::_dsize);
constexpr ID TYPEID_d32     = ID::make_primitive(EPrimitiveTypeKind::_d32);
constexpr ID TYPEID_d64     = ID::make_primitive(EPrimitiveTypeKind::_d64);
constexpr ID TYPEID_d128    = ID::make_primitive(EPrimitiveTypeKind::_d128);
constexpr ID TYPEID_udsize  = ID::make_primitive(EPrimitiveTypeKind::_udsize);
constexpr ID TYPEID_ud32    = ID::make_primitive(EPrimitiveTypeKind::_ud32);
constexpr ID TYPEID_ud64    = ID::make_primitive(EPrimitiveTypeKind::_ud64);
constexpr ID TYPEID_ud128   = ID::make_primitive(EPrimitiveTypeKind::_ud128);
constexpr ID TYPEID_opaque  = ID::make_primitive(EPrimitiveTypeKind::_opaque);
constexpr ID TYPEID_cstr    = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_opaque) + 1);
constexpr ID TYPEID_str     = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_opaque) + 2);
constexpr ID TYPEID_text    = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_opaque) + 3);


} // namespace type