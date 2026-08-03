#pragma once


#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace type
{


enum class ETypeKind : uint8_t {
  NONE,
  Primitive,
  String,
  Tuple,
  Array,
  Buffer,
  Slice,
  Ptr,
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

[[nodiscard]] inline bool ETypeKind_is_iterable(ETypeKind type) noexcept
{
  return type >= ETypeKind::Array && type <= ETypeKind::Slice;
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
  _opaque,
};

enum class EPrimitiveFamily : uint8_t {
  _void,
  _boolean,
  _textual,
  _signed,
  _unsigned,
  _byte,
  _ptrdiff,
  _floating,
  _s_fixed,
  _u_fixed,
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

[[nodiscard]] bool is_op_handled(EPrimitiveTypeKind src, ast::EOp_Bin op) noexcept;
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

enum class ETextType : uint8_t { NONE, _str, _cstr, _text, _cune, _rune };

[[nodiscard]] ETextType ETokenKind_to_ETextType(token::ETokenKind tok) noexcept;

} // namespace type