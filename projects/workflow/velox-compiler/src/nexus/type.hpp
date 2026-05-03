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
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace type
{

enum class ETypeKind : uint8_t {
  NONE,
  Primitive,
  String,
  Tuple,
  StaticArray,
  Ptr,
  DynamicArray,
  Prototype,
  Component,
  Role,
  Entity,
  Enum,
  Flag,
  Union,
  Identifier,
};

inline bool ETypeKind_is_user_defined(ETypeKind type)
{
  return type >= ETypeKind::Component && type <= ETypeKind::Identifier;
}


enum class EPrimitiveTypeKind {
  NONE,
  u0,
  boolean,
  cune,
  rune,
  iSize,
  i8,
  i16,
  i32,
  i64,
  i128,
  uSize,
  u8,
  u16,
  u32,
  u64,
  u128,
  bSize,
  b8,
  b16,
  b32,
  b64,
  b128,
  ptrdiff,
  fSize,
  f16,
  f32,
  f64,
  f80,
  f128,
  dSize,
  d32,
  d64,
  d128,
  udSize,
  ud32,
  ud64,
  ud128,
  COUNT,
};

inline bool EPrimitiveTypeKind_is_signed(EPrimitiveTypeKind type)
{
  return (type >= EPrimitiveTypeKind::iSize && type <= EPrimitiveTypeKind::i128)
         || (type >= EPrimitiveTypeKind::fSize && type <= EPrimitiveTypeKind::d128);
}
inline bool EPrimitiveTypeKind_is_integral(EPrimitiveTypeKind type)
{
  return (type >= EPrimitiveTypeKind::iSize && type <= EPrimitiveTypeKind::u128);
}
inline bool EPrimitiveTypeKind_is_byte(EPrimitiveTypeKind type)
{
  return (type >= EPrimitiveTypeKind::bSize && type <= EPrimitiveTypeKind::b128);
}
inline bool EPrimitiveTypeKind_is_floating(EPrimitiveTypeKind type)
{
  return (type >= EPrimitiveTypeKind::fSize && type <= EPrimitiveTypeKind::f128);
}
inline bool EPrimitiveTypeKind_is_fixed(EPrimitiveTypeKind type)
{
  return (type >= EPrimitiveTypeKind::dSize && type <= EPrimitiveTypeKind::ud128);
}
inline bool EPrimitiveTypeKind_is_textual(EPrimitiveTypeKind type)
{
  return type == EPrimitiveTypeKind::cune || type == EPrimitiveTypeKind::rune;
}


[[nodiscard]] std::string_view EPrimitiveTypeKind_to_str(EPrimitiveTypeKind type);

[[nodiscard]] std::string_view EPrimitiveTypeKind_to_mangle(EPrimitiveTypeKind type);

[[nodiscard]] EPrimitiveTypeKind ETokenKind_to_EPrimitiveTypeKind(token::ETokenKind tok);


enum class EPtrType {
  NONE,
  raw_ptr,
  unique_ptr,
  shared_ptr,
};

[[nodiscard]] std::string_view EPtrType_to_str(EPtrType type);

[[nodiscard]] std::string_view EPtrType_to_mangle(EPtrType type);


[[nodiscard]] size_t EPrimitiveTypeKind_to_bits(EPrimitiveTypeKind type);
[[nodiscard]] size_t EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type);

[[nodiscard]] bool is_op_handled(EPrimitiveTypeKind src, ast::EBinOpType op);
[[nodiscard]] bool is_cast_explicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target);
[[nodiscard]] bool is_cast_implicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target);


template <class T>
inline size_t hash_val(T const& v)
{
  return std::hash<T>{}(v);
}

inline size_t hash_combine(size_t seed, size_t value)
{
  // 0x9e3779b97f4a7c15 = golden ratio constant (good avalanche)
  return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

enum class ETextType { NONE, str, c_str, text, cune, rune };


struct Type final {
  ETypeKind kind = ETypeKind::NONE;

  Decorator decorator;

  struct PrimitiveData final {
    EPrimitiveTypeKind primitive;
  };

  struct PtrData final {
    _id inner;
  };

  struct StringData final {
    ETextType kind = ETextType::str;
  };

  struct StaticArrayData final {
    _id    inner;
    size_t size;
  };

  struct DynamicArrayData final {
    _id inner;
  };

  struct TupleData final {
    std::vector<_id> elems;
  };

  struct PrototypeData final {
    std::vector<_id> params;
    _id              ret;
    bool             is_variadic = false;
  };

  struct EnumData final {
    std::vector<std::vector<_id>> variants;
    symbol::_id                   sym;
  };

  struct FlagData final {
    size_t      size = 1;
    symbol::_id sym;
  };

  struct UnionData final {
    std::vector<_id> variants;
    symbol::_id      sym;
  };

  struct ComponentData final {
    std::vector<_id> fields;
    symbol::_id      sym;
  };

  struct RoleData final {
    std::vector<_id> components;
    symbol::_id      sym;
  };

  struct EntityData final {
    std::vector<_id> components;
    symbol::_id      sym;
  };

  struct IdentifierData final {
    ast::_gnid  gnid;
    symbol::_id sym;

    bool is_resolved() const
    {
      return bool(sym);
    }
    _id get_type() const;
  };

  using DataVariant =
      std::variant<PrimitiveData, PtrData, TupleData, StaticArrayData, DynamicArrayData, PrototypeData, EnumData,
                   StringData, FlagData, UnionData, ComponentData, RoleData, EntityData, IdentifierData>;

  DataVariant data;

  symbol::_id get_sym_id() const;
};

template <typename T, typename Variant>
struct is_in_variant;

template <typename T, typename... Ts>
struct is_in_variant<T, std::variant<Ts...>> : std::bool_constant<(std::is_same_v<T, Ts> || ...)> {
};

template <typename T>
concept IsDataType = is_in_variant<std::remove_cvref_t<T>, Type::DataVariant>::value;


struct TypeEq final {
  bool operator()(Type const& a, Type const& b) const noexcept;
};


struct Arena final {
  Arena();

  struct Factory final {
    Arena& arena;

    _id make_primitive(EPrimitiveTypeKind p, Decorator decorator = Decorator());
    _id make_ptr(_id inner, Decorator decorator = Decorator());
    _id make_static_array(_id inner, size_t size, Decorator decorator = Decorator());
    _id make_dynamic_array(_id inner, Decorator decorator = Decorator());
    _id make_tuple(std::vector<_id> elems, Decorator decorator = Decorator());
    _id make_prototype(std::vector<_id> params, _id ret, bool is_variadic, symbol::_id sym,
                       Decorator decorator = Decorator());
    _id make_enum(std::vector<std::vector<_id>> variants, symbol::_id sym, Decorator decorator = Decorator());
    _id make_flag(size_t size, symbol::_id sym, Decorator decorator = Decorator());
    _id make_union(std::vector<_id> variants, symbol::_id sym, Decorator decorator = Decorator());
    _id make_component(std::vector<_id> fields, symbol::_id sym, Decorator decorator = Decorator());
    _id make_role(std::vector<_id> components, symbol::_id sym, Decorator decorator = Decorator());
    _id make_entity(std::vector<_id> components, symbol::_id sym, Decorator decorator = Decorator());
    _id make_identifier(ast::_gnid gnid, symbol::_id sym, Decorator decorator = Decorator());
    _id make_string(ETextType txt_ty, Decorator decorator = Decorator());
  };

  Factory factory{*this};

  // arena storage
  std::vector<Type> types;

  // safe interning
  std::unordered_map<size_t, std::vector<_id>> buckets;

  const Type& get(_id id) const
  {
    assert(id < types.size());
    return types[id.value()];
  }

  Type& get_mut(_id id)
  {
    assert(id < types.size());
    return types[id.value()];
  }

  template <IsDataType T>
  T* get_as(_id id)
  {
    auto& ty = get_mut(id);
    return std::get_if<T>(&ty.data);
  }

  _id intern(Type t)
  {
    size_t h = hash_type(t);

    auto& bucket = buckets[h];

    // is type already exists
    for (_id id : bucket) {
      if (TypeEq{}(types[id.value()], t)) return id;
    }

    return add(std::move(t), bucket);
  }

private:
  _id add(Type t, std::vector<_id>& bucket)
  {
    _id id = static_cast<_id>(types.size());
    types.push_back(std::move(t));
    bucket.push_back(id);
    return id;
  }
  size_t hash_type(const Type& t) const;
};

struct UnresolvedArena final {
  std::unordered_set<_id, _id_hash> unresolved;

  void add_unresolved(const _id& r)
  {
    unresolved.insert(r);
  }

  bool is_unresolved(_id gnid)
  {
    return unresolved.contains(gnid);
  }

  void resolved(_id gnid)
  {
    unresolved.erase(gnid);
  }
};

constexpr _id BAD_TYPE_ID = _id(0);

constexpr _id TYPEID_u0      = _id(1);
constexpr _id TYPEID_boolean = _id(2);
constexpr _id TYPEID_i8      = _id(3);
constexpr _id TYPEID_i16     = _id(4);
constexpr _id TYPEID_i32     = _id(5);
constexpr _id TYPEID_i64     = _id(6);
constexpr _id TYPEID_i128    = _id(7);
constexpr _id TYPEID_iSize   = _id(8);
constexpr _id TYPEID_u8      = _id(9);
constexpr _id TYPEID_u16     = _id(10);
constexpr _id TYPEID_u32     = _id(11);
constexpr _id TYPEID_u64     = _id(12);
constexpr _id TYPEID_u128    = _id(13);
constexpr _id TYPEID_uSize   = _id(14);
constexpr _id TYPEID_b8      = _id(15);
constexpr _id TYPEID_b16     = _id(16);
constexpr _id TYPEID_b32     = _id(17);
constexpr _id TYPEID_b64     = _id(18);
constexpr _id TYPEID_b128    = _id(19);
constexpr _id TYPEID_bSize   = _id(20);
constexpr _id TYPEID_f16     = _id(21);
constexpr _id TYPEID_f32     = _id(22);
constexpr _id TYPEID_f64     = _id(23);
constexpr _id TYPEID_f80     = _id(24);
constexpr _id TYPEID_f128    = _id(25);
constexpr _id TYPEID_fSize   = _id(26);
constexpr _id TYPEID_d32     = _id(27);
constexpr _id TYPEID_d64     = _id(28);
constexpr _id TYPEID_d128    = _id(29);
constexpr _id TYPEID_dSize   = _id(30);
constexpr _id TYPEID_ud32    = _id(31);
constexpr _id TYPEID_ud64    = _id(32);
constexpr _id TYPEID_ud128   = _id(33);
constexpr _id TYPEID_udSize  = _id(34);
constexpr _id TYPEID_ptrdiff = _id(35);
constexpr _id TYPEID_cune    = _id(36);
constexpr _id TYPEID_rune    = _id(37);
constexpr _id TYPEID_c_str   = _id(38);
constexpr _id TYPEID_str     = _id(39);
constexpr _id TYPEID_text    = _id(40);

} // namespace type