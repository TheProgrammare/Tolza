#pragma once


#include "nexus/forward.hpp"

#include <common/enum_lite.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>

namespace type
{


DEFINE_ENUM(ETypeKind, uint8_t, //
            Primitive, 1,       //
            String, 2,          //
            Tuple, 3,           //
            Array, 4,           //
            Buffer, 5,          //
            Slice, 6,           //
            Ptr, 7,             //
            Prototype, 8,       //
            Facet, 9,           //
            View, 10,           //
            Form, 11,           //
            Enum, 12,           //
            Flag, 13,           //
            Union, 14,          //
            Identifier, 15,     //
            Range, 16,          //
)

[[nodiscard]] inline bool ETypeKind_is_user_defined(ETypeKind type) noexcept
{
  return type >= ETypeKind::Facet && type <= ETypeKind::Identifier;
}

[[nodiscard]] inline bool ETypeKind_is_iterable(ETypeKind type) noexcept
{
  return type >= ETypeKind::Array && type <= ETypeKind::Slice;
}


DEFINE_ENUM(EPrimitiveTypeKind, uint8_t, //
            _u0, 1,                      //
            _bool, 2,                    //
            _cune, 3,                    //
            _rune, 4,                    //
            _ssize, 5,                   //
            _s8, 6,                      //
            _s16, 7,                     //
            _s32, 8,                     //
            _s64, 9,                     //
            _s128, 10,                   //
            _usize, 11,                  //
            _u8, 12,                     //
            _u16, 13,                    //
            _u32, 14,                    //
            _u64, 15,                    //
            _u128, 16,                   //
            _bsize, 17,                  //
            _b8, 18,                     //
            _b16, 19,                    //
            _b32, 20,                    //
            _b64, 21,                    //
            _b128, 22,                   //
            _ptrdiff, 23,                //
            _fsize, 24,                  //
            _f16, 25,                    //
            _f32, 26,                    //
            _f64, 27,                    //
            _f80, 28,                    //
            _f128, 29,                   //
            _dsize, 30,                  //
            _d32, 31,                    //
            _d64, 32,                    //
            _d128, 33,                   //
            _udsize, 34,                 //
            _ud32, 35,                   //
            _ud64, 36,                   //
            _ud128, 37,                  //
            _opaque, 38,                 //
)

DEFINE_ENUM(EPrimitiveFamily, uint8_t, //
            _void, 1,                  //
            _boolean, 2,               //
            _textual, 3,               //
            _signed, 4,                //
            _unsigned, 5,              //
            _byte, 6,                  //
            _ptrdiff, 7,               //
            _floating, 8,              //
            _s_fixed, 9,               //
            _u_fixed, 10,              //
            _ptr, 11,                  //
)

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


DEFINE_ENUM(EPtrType, uint8_t, //
            raw_ptr, 1,        //
            unique_ptr, 2,     //
            shared_ptr, 3,     //
)

[[nodiscard]] std::string_view EPtrType_to_mangle(EPtrType type) noexcept;


[[nodiscard]] size_t EPrimitiveTypeKind_to_bits(EPrimitiveTypeKind type) noexcept;
[[nodiscard]] size_t EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type) noexcept;

[[nodiscard]] bool is_op_handled(EPrimitiveTypeKind src, ast::EOp_Bin op) noexcept;
[[nodiscard]] bool is_cast_explicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept;
[[nodiscard]] bool is_cast_implicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept;


template <typename T>
[[nodiscard]] inline size_t hash_val(T const& v) noexcept
{
  return std::hash<T>{}(v);
}

[[nodiscard]] inline size_t hash_combine(size_t seed, size_t value) noexcept
{
  // 0x9e3779b97f4a7c15 = golden ratio constant (good avalanche)
  return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

DEFINE_ENUM(ETextType, uint8_t, //
            _str, 1,            //
            _cstr, 2,           //
            _text, 3,           //
            _cune, 4,           //
            _rune, 5,           //
)

[[nodiscard]] ETextType ETokenKind_to_ETextType(token::ETokenKind tok) noexcept;

} // namespace type