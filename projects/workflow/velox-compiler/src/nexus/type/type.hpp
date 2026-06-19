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
#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace type
{

// start of user type
constexpr size_t TYPEID_USER_START = 42;


enum class ETypeKind : uint8_t {
  NONE,
  Primitive,
  String,
  Tuple,
  StaticArray,
  Ptr,
  DynamicArray,
  Prototype,
  Facet,
  View,
  Form,
  Enum,
  Flag,
  Union,
  Identifier,
};

[[nodiscard]] inline bool ETypeKind_is_user_defined(ETypeKind type) noexcept
{
  return type >= ETypeKind::Facet && type <= ETypeKind::Identifier;
}


enum class EPrimitiveTypeKind : uint8_t {
  NONE,
  _u0,
  _bool,
  _cune,
  _rune,
  _ssize,
  _s8,
  _s16,
  _s32,
  _s64,
  _s128,
  _usize,
  _u8,
  _u16,
  _u32,
  _u64,
  _u128,
  _bsize,
  _b8,
  _b16,
  _b32,
  _b64,
  _b128,
  _ptrdiff,
  _fsize,
  _f16,
  _f32,
  _f64,
  _f80,
  _f128,
  _dsize,
  _d32,
  _d64,
  _d128,
  _udsize,
  _ud32,
  _ud64,
  _ud128,
  _ptr,
};

[[nodiscard]] inline bool EPrimitiveTypeKind_is_signed(EPrimitiveTypeKind type) noexcept
{
  return (type >= EPrimitiveTypeKind::_ssize && type <= EPrimitiveTypeKind::_s128)
         || (type >= EPrimitiveTypeKind::_fsize && type <= EPrimitiveTypeKind::_d128);
}
[[nodiscard]] inline bool EPrimitiveTypeKind_is_integral(EPrimitiveTypeKind type) noexcept
{
  return (type >= EPrimitiveTypeKind::_ssize && type <= EPrimitiveTypeKind::_u128);
}
[[nodiscard]] inline bool EPrimitiveTypeKind_is_byte(EPrimitiveTypeKind type) noexcept
{
  return (type >= EPrimitiveTypeKind::_bsize && type <= EPrimitiveTypeKind::_b128);
}
[[nodiscard]] inline bool EPrimitiveTypeKind_is_floating(EPrimitiveTypeKind type) noexcept
{
  return (type >= EPrimitiveTypeKind::_fsize && type <= EPrimitiveTypeKind::_f128);
}
[[nodiscard]] inline bool EPrimitiveTypeKind_is_fixed(EPrimitiveTypeKind type) noexcept
{
  return (type >= EPrimitiveTypeKind::_dsize && type <= EPrimitiveTypeKind::_ud128);
}
[[nodiscard]] inline bool EPrimitiveTypeKind_is_textual(EPrimitiveTypeKind type) noexcept
{
  return type == EPrimitiveTypeKind::_cune || type == EPrimitiveTypeKind::_rune;
}


[[nodiscard]] std::string_view EPrimitiveTypeKind_to_mangle(EPrimitiveTypeKind type) noexcept;

[[nodiscard]] EPrimitiveTypeKind ETokenKind_to_EPrimitiveTypeKind(token::ETokenKind tok) noexcept;


enum class EPtrType : uint8_t {
  NONE,
  raw_ptr,
  unique_ptr,
  shared_ptr,
};

[[nodiscard]] std::string_view EPtrType_to_str(EPtrType type) noexcept;

[[nodiscard]] std::string_view EPtrType_to_mangle(EPtrType type) noexcept;


[[nodiscard]] size_t EPrimitiveTypeKind_to_bits(EPrimitiveTypeKind type) noexcept;
[[nodiscard]] size_t EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type) noexcept;

[[nodiscard]] bool is_op_handled(EPrimitiveTypeKind src, ast::EBinOpType op) noexcept;
[[nodiscard]] bool is_cast_explicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept;
[[nodiscard]] bool is_cast_implicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept;


template <class T>
[[nodiscard]] inline size_t hash_val(T const& v) noexcept
{
  return std::hash<T>{}(v);
}

[[nodiscard]] inline size_t hash_combine(size_t seed, size_t value) noexcept
{
  // 0x9e3779b97f4a7c15 = golden ratio constant (good avalanche)
  return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

enum class ETextType : uint8_t { NONE, _str, _c_str, _text, _cune, _rune };

[[nodiscard]] ETextType ETokenKind_to_ETextType(token::ETokenKind tok) noexcept;


struct Type {
  ID tyid;

  Qualifier qualifier;

  [[nodiscard]] symbol::ID  get_sym_id() const noexcept;
  [[nodiscard]] bool        set_sym_id(symbol::ID symid) noexcept;
  [[nodiscard]] std::string velox_codegen() const noexcept;

  [[nodiscard]] ETypeKind kind() const noexcept
  {
    return _kind;
  }

  Type(const Type&)            = delete;
  Type& operator=(const Type&) = delete;

protected:
  const ETypeKind _kind = ETypeKind::NONE;
  Type(ETypeKind k)
    : _kind(k)
  {
  }

public:
  virtual ~Type() = default;
};

template <ETypeKind K>
struct TypeBase : Type {
  static constexpr ETypeKind static_kind = K;
  TypeBase()
    : Type(K)
  {
  }
};

#define DEF_TYPE(name) struct name final : TypeBase<ETypeKind::name>


DEF_TYPE(Primitive)
{
  EPrimitiveTypeKind primitive;
};

DEF_TYPE(Ptr)
{
  ID inner;
};

DEF_TYPE(String)
{
  ETextType kind = ETextType::_str;
};

DEF_TYPE(StaticArray)
{
  ID     inner;
  size_t size;
};

DEF_TYPE(DynamicArray)
{
  ID inner;
};

DEF_TYPE(Tuple)
{
  std::vector<ID> elems;
};

DEF_TYPE(Prototype)
{
  struct Param {
    ast::EPassMode passmode;
    ID             type;
    bool           is_restrict = false;

    auto operator<=>(const Param&) const = default;
  };

  std::vector<Param> params;
  ID                 ret;
  bool               is_variadic = false;
};

DEF_TYPE(Enum)
{
  std::vector<ID> variants;
  symbol::ID      sym;
};

DEF_TYPE(Flag)
{
  size_t     size = 1;
  symbol::ID sym;
};

DEF_TYPE(Union)
{
  std::vector<ID> variants;
  symbol::ID      sym;
};

DEF_TYPE(Facet)
{
  std::vector<ID> fields;
  symbol::ID      sym;
};

DEF_TYPE(View)
{
  std::vector<ID> facets;
  symbol::ID      sym;
};

DEF_TYPE(Form)
{
  std::vector<ID> facets;
  symbol::ID      sym;
};

DEF_TYPE(Identifier)
{
  ast::ID     nodeid;
  symbol::ID  sym;
  std::string forward_name;

  [[nodiscard]] bool is_resolved() const noexcept
  {
    return bool(sym);
  }
  [[nodiscard]] ID get_type() const noexcept;
};


ID get_prototype(ast::ID nodeid) noexcept;


template <typename T>
concept IsDataType = std::is_base_of_v<Type, T>;


struct TypeEq final {
  bool operator()(Type const& a, Type const& b) const noexcept;
};

struct Factory final {
  template <IsDataType T, typename... Args>
  [[nodiscard]] T make_type(Args&&... args) const
  {
    return T(args...);
  }
};

void initialization() noexcept;

inline std::unordered_map<ID, std::unique_ptr<Type>, ID::Hash> primitives;

[[nodiscard]] Type& get(ID id) noexcept;

template <IsDataType T>
[[nodiscard]] T* as(ID id) noexcept;

struct Arena final {
  Arena(cu::ID _cuid);

  struct Factory final {
    Arena& arena;

    [[nodiscard]] ID make_primitive(EPrimitiveTypeKind p, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_ptr(ID inner, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_static_array(ID inner, size_t size, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_dynamic_array(ID inner, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_tuple(const std::vector<ID>& elems, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_prototype(const std::vector<type::Prototype::Param>& params, ID ret, bool is_variadic,
                                    Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_enum(const std::vector<ID>& variants, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_flag(size_t size, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_union(const std::vector<ID>& variants, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_facet(const std::vector<ID>& fields, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_view(const std::vector<ID>& facets, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_form(const std::vector<ID>& facets, symbol::ID sym, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_forward_identifier(std::string_view name, Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_identifier(std::string_view name, ast::ID nodeid, symbol::ID sym,
                                     Qualifier qualifier = Qualifier());
    [[nodiscard]] ID make_string(ETextType txt_ty, Qualifier qualifier = Qualifier());
  };

  Factory factory{*this};

  bool freeze = false;

  cu::ID cuid;

  // arena storage
  std::vector<std::unique_ptr<Type>> types;

  // safe interning
  std::unordered_map<size_t, std::vector<ID>> buckets;


  [[nodiscard]] Type&       get(ID id) noexcept;
  [[nodiscard]] const Type& get(ID id) const noexcept;

  template <IsDataType T>
  [[nodiscard]] T* as(ID id) noexcept
  {
    auto* ty = &get(id);
    assert(ty->kind() == T::static_kind && "Must derivate to Type");
    return static_cast<T*>(ty);
  }

  [[nodiscard]] ID intern(std::unique_ptr<Type> t) noexcept
  {
    const size_t h = hash_type(*t);

    auto& bucket = buckets[h];

    // is type already exists
    for (ID id : bucket) {
      const auto offset = id.offset() - TYPEID_USER_START;
      assert(offset >= 0 && offset < types.size() && "offset is out of bounds");
      const auto* ty = types[offset].get();
      assert(ty && "type not found");
      if (TypeEq{}(*ty, *t)) return id;
    }

    return add(std::move(t), bucket);
  }

private:
  [[nodiscard]] ID add(std::unique_ptr<Type> t, std::vector<ID>& bucket) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    ID new_id = ID::make(cuid, types.size() + TYPEID_USER_START);
    t->tyid   = new_id;
    types.emplace_back(std::move(t));
    bucket.emplace_back(new_id);
    return new_id;
  }
  [[nodiscard]] static size_t hash_type(const Type& t) noexcept;
};

constexpr ID BAD_TYPE_ID = ID::invalid();

constexpr ID TYPEID_u0      = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u0);
constexpr ID TYPEID_bool    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_bool);
constexpr ID TYPEID_cune    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_cune);
constexpr ID TYPEID_rune    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_rune);
constexpr ID TYPEID_ssize   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ssize);
constexpr ID TYPEID_s8      = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_s8);
constexpr ID TYPEID_s16     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_s16);
constexpr ID TYPEID_s32     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_s32);
constexpr ID TYPEID_s64     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_s64);
constexpr ID TYPEID_s128    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_s128);
constexpr ID TYPEID_usize   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_usize);
constexpr ID TYPEID_u8      = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u8);
constexpr ID TYPEID_u16     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u16);
constexpr ID TYPEID_u32     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u32);
constexpr ID TYPEID_u64     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u64);
constexpr ID TYPEID_u128    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_u128);
constexpr ID TYPEID_bsize   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_bsize);
constexpr ID TYPEID_b8      = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_b8);
constexpr ID TYPEID_b16     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_b16);
constexpr ID TYPEID_b32     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_b32);
constexpr ID TYPEID_b64     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_b64);
constexpr ID TYPEID_b128    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_b128);
constexpr ID TYPEID_ptrdiff = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ptrdiff);
constexpr ID TYPEID_fsize   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_fsize);
constexpr ID TYPEID_f16     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_f16);
constexpr ID TYPEID_f32     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_f32);
constexpr ID TYPEID_f64     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_f64);
constexpr ID TYPEID_f80     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_f80);
constexpr ID TYPEID_f128    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_f128);
constexpr ID TYPEID_dsize   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_dsize);
constexpr ID TYPEID_d32     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_d32);
constexpr ID TYPEID_d64     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_d64);
constexpr ID TYPEID_d128    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_d128);
constexpr ID TYPEID_udsize  = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_udsize);
constexpr ID TYPEID_ud32    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ud32);
constexpr ID TYPEID_ud64    = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ud64);
constexpr ID TYPEID_ud128   = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ud128);
constexpr ID TYPEID_ptr     = ID::make_primitive(cu::ID::main(), EPrimitiveTypeKind::_ptr);
constexpr ID TYPEID_c_str   = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_ptr) + 1);
constexpr ID TYPEID_str     = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_ptr) + 2);
constexpr ID TYPEID_text    = ID::make(cu::ID::main(), static_cast<uint8_t>(EPrimitiveTypeKind::_ptr) + 3);


} // namespace type