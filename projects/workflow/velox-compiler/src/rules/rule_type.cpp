#include "rule_type.hpp"

#include "nexus/ast/ast.hpp"
#include "nexus/type.hpp"

#include <cerrno>
#include <cstddef>
#include <initializer_list>


bool rule::type::can_op_primitive(::type::EPrimitiveTypeKind term, ast::EBinOpType op)
{
  auto op_handled = [&](const std::initializer_list<ast::EBinOpType>& collection) {
    return std::find(collection.begin(), collection.end(), op) != collection.end();
  };

  switch (term) {
  case ::type::EPrimitiveTypeKind::boolean: return ast::EBinOpType_is_boolean(op);
  case ::type::EPrimitiveTypeKind::cune:
  case ::type::EPrimitiveTypeKind::rune:    return ast::EBinOpType_is_textual(op);
  case ::type::EPrimitiveTypeKind::iSize:
  case ::type::EPrimitiveTypeKind::i8:
  case ::type::EPrimitiveTypeKind::i16:
  case ::type::EPrimitiveTypeKind::i32:
  case ::type::EPrimitiveTypeKind::i64:
  case ::type::EPrimitiveTypeKind::i128:
  case ::type::EPrimitiveTypeKind::ptrdiff:
  case ::type::EPrimitiveTypeKind::uSize:
  case ::type::EPrimitiveTypeKind::u8:
  case ::type::EPrimitiveTypeKind::u16:
  case ::type::EPrimitiveTypeKind::u32:
  case ::type::EPrimitiveTypeKind::u64:
  case ::type::EPrimitiveTypeKind::u128:    return ast::EBinOpType_is_integral(op);
  case ::type::EPrimitiveTypeKind::bSize:
  case ::type::EPrimitiveTypeKind::b8:
  case ::type::EPrimitiveTypeKind::b16:
  case ::type::EPrimitiveTypeKind::b32:
  case ::type::EPrimitiveTypeKind::b64:
  case ::type::EPrimitiveTypeKind::b128:    return ast::EBinOpType_is_bitwise(op);
  case ::type::EPrimitiveTypeKind::dSize:
  case ::type::EPrimitiveTypeKind::d32:
  case ::type::EPrimitiveTypeKind::d64:
  case ::type::EPrimitiveTypeKind::d128:
  case ::type::EPrimitiveTypeKind::udSize:
  case ::type::EPrimitiveTypeKind::ud32:
  case ::type::EPrimitiveTypeKind::ud64:
  case ::type::EPrimitiveTypeKind::ud128:
  case ::type::EPrimitiveTypeKind::fSize:
  case ::type::EPrimitiveTypeKind::f16:
  case ::type::EPrimitiveTypeKind::f32:
  case ::type::EPrimitiveTypeKind::f64:
  case ::type::EPrimitiveTypeKind::f80:
  case ::type::EPrimitiveTypeKind::f128:    return ast::EBinOpType_is_decimal(op);
  default:                                  return false;
  }
}


bool rule::type::can_binary_op_primitive(::type::EPrimitiveTypeKind lhs, ::type::EPrimitiveTypeKind rhs)
{
}

bool rule::type::can_binary_op_entity(const ast::COP_Entity& lhs, const ast::COP_Entity& rhs)
{
}


bool rule::type::can_cast_on_primitive_as_primitve(::type::EPrimitiveTypeKind term,
                                                   ::type::EPrimitiveTypeKind target_type)
{
  if (term == target_type) return true;

  static const bool table[static_cast<size_t>(::type::EPrimitiveTypeKind::COUNT)
                          + 1][static_cast<size_t>(::type::EPrimitiveTypeKind::COUNT) + 1] = {
      // clang-format off
	//              NONE	  u0      boolean	cune	  rune	  iSize	  i8	    i16	    i32	    i64	    i128	  uSize	  u8	    u16	    u32	    u64	    u128	  bSize	  b8	    b16	    b32	    b64	    b128	  ptrdiff	fSize	  f16     f32	    f64	    f80     f128	  dSize	  d32     d64     d128    udSize  ud32    ud64    ud128   COUNT
  /*NONE*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*u0*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,},
  /*boolean*/   { false,	false, 	true,	  false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*cune*/      { false,	false,	false, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*rune*/      { false,	false,	false,	false, 	true, 	true, 	false, 	false,	true,	  false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,},
  /*iSize*/     { false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*i8*/        { false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*i16*/       { false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*i32*/       { false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*i64*/       { false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*i128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*uSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*u8*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*u16*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*u32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*u64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*u128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*bSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*b8*/        { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*b16*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*b32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*b64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*b128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*ptrdiff*/   { false,	false,	false,	false,	false,	false, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*fSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*f16*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true,	  true,	  false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*f32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true, 	true, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*f64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*f80*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	false, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*f128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,  true,		false,	false,	false,	false,	false,	false,	false,  false,	false,},
  /*dSize*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	false,	false,	false,	false,	false,  false,	false,},
  /*d32*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true,	  true,	  false,	false,	false,	false,	false,  false,	false,},
  /*d64*/       { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true, 	false,	false,	false,	false,  false,	false,},
  /*d128*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true, 	true, 	false,	false,	false,  false,	false,},
  /*udSize*/    { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	false,  false,	false,},
  /*ud32*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  false,  false,	false,},
  /*ud64*/      { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true,   false,	false,},
  /*ud128*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true,   true,	  false,},
  /*COUNT*/     { false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false,},
      // clang-format on
  };

  return table[static_cast<size_t>(term)][static_cast<size_t>(target_type)];
}
bool rule::type::can_cast_on_primitive_as_entity(::type::EPrimitiveTypeKind term, const ast::COP_Entity& target_type)
{
}
bool rule::type::can_cast_on_entity_as_primtive(const ast::COP_Entity& term, ::type::EPrimitiveTypeKind target_type)
{
}
bool rule::type::can_cast_on_entity_as_entity(const ast::COP_Entity& term, const ast::COP_Entity& target_type)
{
}