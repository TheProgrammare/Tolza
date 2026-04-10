#include "rule_type.hpp"
#include "ast/ast_data.hpp"
#include <cerrno>
#include <cstddef>
#include <initializer_list>


bool rule::type::op_on_primitive(EPrimType term, EBinOpType op)
{
  auto op_handled = [&](const std::initializer_list<EBinOpType>& collection) {
    return std::find(collection.begin(), collection.end(), op) != collection.end();
  };

  switch (term) {
  case EPrimType::boolean: return op_handled(k_boolean_op);
  case EPrimType::cune:
  case EPrimType::rune:
  case EPrimType::c_str:
  case EPrimType::str:
  case EPrimType::text:    return op_handled(k_textual_op);
  case EPrimType::iSize:
  case EPrimType::i8:
  case EPrimType::i16:
  case EPrimType::i32:
  case EPrimType::i64:
  case EPrimType::i128:
  case EPrimType::ptrdiff:
  case EPrimType::uSize:
  case EPrimType::u8:
  case EPrimType::u16:
  case EPrimType::u32:
  case EPrimType::u64:
  case EPrimType::u128:    return op_handled(k_integral_op);
  case EPrimType::bSize:
  case EPrimType::b8:
  case EPrimType::b16:
  case EPrimType::b32:
  case EPrimType::b64:
  case EPrimType::b128:    return op_handled(k_bitwise_op);
  case EPrimType::dSize:
  case EPrimType::d32:
  case EPrimType::d64:
  case EPrimType::d128:
  case EPrimType::udSize:
  case EPrimType::ud32:
  case EPrimType::ud64:
  case EPrimType::ud128:
  case EPrimType::fSize:
  case EPrimType::f16:
  case EPrimType::f32:
  case EPrimType::f64:
  case EPrimType::f80:
  case EPrimType::f128:    return op_handled(k_decimal_op);
  case EPrimType::Enum:
  case EPrimType::Flag:    return op_handled(k_bitwise_op);
  default:                 return false;
  }
}


bool rule::type::binary_op_primitive(EPrimType lhs, EPrimType rhs)
{
}

bool rule::type::binary_op_entity(const ast::declaration::cop::Entity& lhs, const ast::declaration::cop::Entity& rhs)
{
}


bool rule::type::cast_on_primitive_as_primitve(EPrimType term, EPrimType target_type)
{
  if (term == target_type) return true;

  static const bool table[static_cast<size_t>(EPrimType::COUNT) + 1][static_cast<size_t>(EPrimType::COUNT) + 1] = {
      // clang-format off
	//                NONE	  boolean	  cune	  rune	  c_str	  str	  text	  iSize	  i8	  i16	  i32	  i64	  i128	  uSize	  u8	  u16	  u32	  u64	  u128	  bSize	  b8	  b16	  b32	  b64	  b128	  ptrdiff	  fSize	  f32	  f64	  f128	  u0	  deci	  udeci	  Enum	  Flag	  Component	  Role	  Entity	  Generic	  Function	  Fn_Proto	  tuple	  Array	  map	  Range	  Iterator	  Slice	  COUNT
  /*NONE*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,          },
  /*boolean*/   { false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*cune*/      { false,	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*rune*/      { false,	false,	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*c_str*/     { false,	false,	true, 	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*str*/       { false,	false,	true, 	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*text*/      { false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*iSize*/     { false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*i8*/        { false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*i16*/       { false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*i32*/       { false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*i64*/       { false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*i128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*uSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u8*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u16*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*bSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*b8*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*b16*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*b32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*b64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*b128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*ptrdiff*/   { false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*fSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*f32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*f64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*f128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u0*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*uSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*d32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*d64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*d128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*udSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*ud32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*ud64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*ud128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Enum*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Flag*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Component*/ { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Role*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Entity*/    { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Generic*/   { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Function*/  { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,},
  /*Fn_Proto*/  { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,},
  /*tuple*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,},
  /*Array*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,},
  /*map*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,},
  /*Range*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,},
  /*Iterator*/  { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,},
  /*Slice*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,},
  /*COUNT*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
      // clang-format on
  };

  return table[static_cast<size_t>(term)][static_cast<size_t>(target_type)];
}
bool rule::type::cast_on_primitive_as_entity(EPrimType term, const ast::declaration::cop::Entity& target_type)
{
}
bool rule::type::cast_on_entity_as_primtive(const ast::declaration::cop::Entity& term, EPrimType target_type)
{
}
bool rule::type::cast_on_entity_as_entity(const ast::declaration::cop::Entity& term,
                                          const ast::declaration::cop::Entity& target_type)
{
}