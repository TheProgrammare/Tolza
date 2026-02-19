#include "AST_Literal.hpp"

#include "AST/AST_Data.hpp"
#include "AST/AST_Type.hpp"
#include "AST_Inferred_Type_Singleton.hpp"

#include "Visitor/Symbol_Manager.hpp"

std::string AST::Literal::Table::debug_str() const
{
  if (element_definition.expired()) return "";
  std::string out;
  out = "table[" + std::to_string(resolved_size.size()) + ":";
  for (size_t i = 0; i < resolved_size.size(); i++) {
    out += std::to_string(resolved_size[i]) + "x";
  }
  if (auto locked = element_definition.lock()) out += " -> " + locked->symbol->name + "]";
  return out;
}

AST::Literal::Boolean::Boolean() { inferred_type = Type::get_bool_type(); }

AST::Literal::Integral::Integral()
{
  switch (type) {
  // signed integers
  case EPrimType::i8:    inferred_type = Type::get_i8_type(); break;
  case EPrimType::i16:   inferred_type = Type::get_i16_type(); break;
  case EPrimType::i32:   inferred_type = Type::get_i32_type(); break;
  case EPrimType::i64:   inferred_type = Type::get_i64_type(); break;
  case EPrimType::i128:  inferred_type = Type::get_i128_type(); break;
  case EPrimType::iSize: inferred_type = Type::get_isize_type(); break;

  // unsigned integers
  case EPrimType::u8:    inferred_type = Type::get_u8_type(); break;
  case EPrimType::u16:   inferred_type = Type::get_u16_type(); break;
  case EPrimType::u32:   inferred_type = Type::get_u32_type(); break;
  case EPrimType::u64:   inferred_type = Type::get_u64_type(); break;
  case EPrimType::u128:  inferred_type = Type::get_u128_type(); break;
  case EPrimType::uSize: inferred_type = Type::get_usize_type(); break;

  // booleans
  case EPrimType::b8:    inferred_type = Type::get_b8_type(); break;
  case EPrimType::b16:   inferred_type = Type::get_b16_type(); break;
  case EPrimType::b32:   inferred_type = Type::get_b32_type(); break;
  case EPrimType::b64:   inferred_type = Type::get_b64_type(); break;
  case EPrimType::b128:  inferred_type = Type::get_b128_type(); break;
  case EPrimType::bSize: inferred_type = Type::get_bsize_type(); break;
  default:               break;
  }
}

AST::Literal::Decimal::Decimal()
{
  if (is_unsigned)
    inferred_type = Type::get_udeci_type();
  else
    inferred_type = Type::get_deci_type();
}

AST::Literal::Floating::Floating()
{
  switch (type) {
  // signed integers
  case EPrimType::f32:   inferred_type = Type::get_f32_type(); break;
  case EPrimType::f64:   inferred_type = Type::get_f64_type(); break;
  case EPrimType::f128:  inferred_type = Type::get_f128_type(); break;
  case EPrimType::fSize: inferred_type = Type::get_fsize_type(); break;
  default:               break;
  }
}

AST::Literal::ASCII::ASCII() { inferred_type = Type::get_ascii_type(); }

AST::Literal::UTF32::UTF32() { inferred_type = Type::get_utf32_type(); }

AST::Literal::Text::Text()
{
  if (is_ascii)
    inferred_type = Type::get_str_type();
  else
    inferred_type = Type::get_text_type();
}
