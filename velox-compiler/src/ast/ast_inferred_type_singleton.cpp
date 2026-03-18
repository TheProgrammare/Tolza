#include "ast_inferred_type_singleton.hpp"

#include "ast/ast_base.hpp"
#include "ast_type.hpp"


ast::AType* ast::type::get_bool_type()
{
  static ast::AType* prim = new Primitive(EPrimType::boolean);
  return prim;
}

ast::AType* ast::type::get_i8_type()
{
  static ast::AType* prim = new Primitive(EPrimType::i8);
  return prim;
}

ast::AType* ast::type::get_i16_type()
{
  static ast::AType* prim = new Primitive(EPrimType::i16);
  return prim;
}

ast::AType* ast::type::get_i32_type()
{
  static ast::AType* prim = new Primitive(EPrimType::i32);
  return prim;
}

ast::AType* ast::type::get_i64_type()
{
  static ast::AType* prim = new Primitive(EPrimType::i64);
  return prim;
}

ast::AType* ast::type::get_i128_type()
{
  static ast::AType* prim = new Primitive(EPrimType::i128);
  return prim;
}

ast::AType* ast::type::get_isize_type()
{
  static ast::AType* prim = new Primitive(EPrimType::iSize);
  return prim;
}

// unsigned integers
ast::AType* ast::type::get_u8_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u8);
  return prim;
}

ast::AType* ast::type::get_u16_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u16);
  return prim;
}

ast::AType* ast::type::get_u32_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u32);
  return prim;
}

ast::AType* ast::type::get_u64_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u64);
  return prim;
}

ast::AType* ast::type::get_u128_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u128);
  return prim;
}

ast::AType* ast::type::get_usize_type()
{
  static ast::AType* prim = new Primitive(EPrimType::uSize);
  return prim;
}

// booleans
ast::AType* ast::type::get_b8_type()
{
  static ast::AType* prim = new Primitive(EPrimType::b8);
  return prim;
}

ast::AType* ast::type::get_b16_type()
{
  static ast::AType* prim = new Primitive(EPrimType::b16);
  return prim;
}

ast::AType* ast::type::get_b32_type()
{
  static ast::AType* prim = new Primitive(EPrimType::b32);
  return prim;
}

ast::AType* ast::type::get_b64_type()
{
  static ast::AType* prim = new Primitive(EPrimType::b64);
  return prim;
}

ast::AType* ast::type::get_b128_type()
{
  static ast::AType* prim = new Primitive(EPrimType::b128);
  return prim;
}

ast::AType* ast::type::get_bsize_type()
{
  static ast::AType* prim = new Primitive(EPrimType::bSize);
  return prim;
}

ast::AType* ast::type::get_ptrdiff_type()
{
  static ast::AType* prim = new Primitive(EPrimType::ptrdiff);
  return prim;
}

// floats
ast::AType* ast::type::get_f32_type()
{
  static ast::AType* prim = new Primitive(EPrimType::f32);
  return prim;
}

ast::AType* ast::type::get_f64_type()
{
  static ast::AType* prim = new Primitive(EPrimType::f64);
  return prim;
}

ast::AType* ast::type::get_f128_type()
{
  static ast::AType* prim = new Primitive(EPrimType::f128);
  return prim;
}

ast::AType* ast::type::get_fsize_type()
{
  static ast::AType* prim = new Primitive(EPrimType::fSize);
  return prim;
}

// decimals
ast::AType* ast::type::get_deci_type()
{
  static ast::AType* prim = new Primitive(EPrimType::deci);
  return prim;
}

ast::AType* ast::type::get_udeci_type()
{
  static ast::AType* prim = new Primitive(EPrimType::udeci);
  return prim;
}

// other types
ast::AType* ast::type::get_void_type()
{
  static ast::AType* prim = new Primitive(EPrimType::u0);
  return prim;
}

ast::type::Tuple* ast::type::get_void_return_type()
{
  static ast::type::Tuple* tuple = new Tuple();
  return tuple;
}

ast::AType* ast::type::get_ascii_type()
{
  static ast::AType* prim = new Primitive(EPrimType::ASCII);
  return prim;
}

ast::AType* ast::type::get_utf32_type()
{
  static ast::AType* prim = new Primitive(EPrimType::UTF32);
  return prim;
}

ast::AType* ast::type::get_str_type()
{
  static ast::AType* prim = new Primitive(EPrimType::str);
  return prim;
}

ast::AType* ast::type::get_c_str_type()
{
  static ast::AType* prim = new Primitive(EPrimType::c_str);
  return prim;
}

ast::AType* ast::type::get_text_type()
{
  static ast::AType* prim = new Primitive(EPrimType::text);
  return prim;
}