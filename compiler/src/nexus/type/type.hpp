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

#include <cassert>
#include <cstdint>
#include <memory>
#include <print>
#include <memory_resource>
#include <string_view>
#include <unordered_map>


#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

#include "nexus/type/definition.hpp"

namespace type
{

// start of user type
constexpr size_t TYPEID_USER_START = 42;
constexpr size_t TYPEID_FFI_START  = std::numeric_limits<size_t>::max() / 2;

struct TypeEntry final {
  ID        id;
  ETypeKind kind;
  Variant   data;

  explicit TypeEntry(ID p_id, ETypeKind p_kind, Variant p_data)
    : id(p_id)
    , kind(p_kind)
    , data(std::move(p_data))
  {
  }
};


[[nodiscard]] bool is_same_type(type::Primitive const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Ptr const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Array const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Buffer const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Slice const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Tuple const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Prototype const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Enum const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Flag const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Union const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Facet const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Form const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::View const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Identifier const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::String const& t, ID tyid) noexcept;

ID   get_prototype(ast::ID nodeid) noexcept;
ID   get_inner(type::ID tyid) noexcept;
// ID   get_key(ast::ID nodeid) noexcept;
// ID   get_val(ast::ID nodeid) noexcept;
void initialization() noexcept;

inline std::unordered_map<ID, TypeEntry, ID::Hash> primitives;

[[nodiscard]] std::string dump(ID tyid) noexcept;

[[nodiscard]] std::vector<Prototype_Param> to_proto_params(const std::vector<ast::ID>& params) noexcept;

struct Arena final {
  Arena(cu::ID _cuid);

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

  Factory factory{*this};

  bool freeze = false;

  cu::ID cuid;

  // arena storage
  std::vector<TypeEntry>                                 types;
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
      auto& data = type::primitives.at(type::ID::make(cu::ID::main(), index)).data;
      return std::get_if<T>(&data);
    }

    index -= TYPEID_USER_START;

    // canonical
    auto it = canon_identifier.find(tyid);
    if (it != canon_identifier.end()) return as<T>(it->second);

    assert(index < types.size());
    auto& entry = types[index];

    return std::get_if<T>(&entry.data);
  }

  template <type::Generic T>
  [[nodiscard]] const T* as(ID tyid) const noexcept
  {
    assert(tyid.cu() == cuid);
    size_t index = tyid.index();
    if (index < TYPEID_USER_START) {
      const auto& data = type::primitives.at(type::ID::make(cu::ID::main(), index)).data;
      return std::get_if<const T>(&data);
    }

    index -= TYPEID_USER_START;

    // canonical
    auto it = canon_identifier.find(tyid);
    if (it != canon_identifier.end()) return as<T>(it->second);

    assert(index < types.size());
    const auto& entry = types[index];

    T* out = nullptr;
    if (entry.kind == T::static_kind) out = std::get_if<const T>(&entry.data);

    return out;
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

    ID new_id = ID::make(cuid, types.size() + TYPEID_USER_START);

    types.emplace_back(TypeEntry(new_id, U::static_kind, Variant(std::forward<T>(t))));

    std::visit([&](auto& obj) { obj.header.tyid = new_id; }, types.back().data);

    bucket.emplace_back(new_id);

    return new_id;
  }

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