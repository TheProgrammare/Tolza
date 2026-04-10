#include "ast_inferred_type_singleton.hpp"

#include "ast/ast_base.hpp"
#include "ast_type.hpp"


std::shared_ptr<ast::AType> ast::type::get_bool_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::boolean);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_i8_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::i8);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_i16_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::i16);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_i32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::i32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_i64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::i64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_i128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::i128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_isize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::iSize);
  return prim;
}

// unsigned integers
std::shared_ptr<ast::AType> ast::type::get_u8_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u8);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_u16_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u16);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_u32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_u64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_u128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_usize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::uSize);
  return prim;
}

// booleans
std::shared_ptr<ast::AType> ast::type::get_b8_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::b8);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_b16_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::b16);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_b32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::b32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_b64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::b64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_b128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::b128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_bsize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::bSize);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_ptrdiff_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::ptrdiff);
  return prim;
}

// floats
std::shared_ptr<ast::AType> ast::type::get_f16_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::f16);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_f32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::f32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_f64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::f64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_f80_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::f80);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_f128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::f128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_fsize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::fSize);
  return prim;
}

// decimals

std::shared_ptr<ast::AType> ast::type::get_d32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::d32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_d64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::d64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_d128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::d128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_dsize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::dSize);
  return prim;
}
std::shared_ptr<ast::AType> ast::type::get_ud32_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::ud32);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_ud64_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::ud64);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_ud128_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::ud128);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_udsize_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::udSize);
  return prim;
}

// other types
std::shared_ptr<ast::AType> ast::type::get_void_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::u0);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_cune_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::cune);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_rune_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::rune);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_str_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::str);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_c_str_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::c_str);
  return prim;
}

std::shared_ptr<ast::AType> ast::type::get_text_type()
{
  static std::shared_ptr<ast::AType> prim = std::make_shared<Primitive>(EPrimType::text);
  return prim;
}