#include "rule_type.hpp"

#include <Neargye/magic_enum.hpp>
#include "nexus/ast/ast.hpp"
#include "nexus/type/type.hpp"

#include <cerrno>
#include <cstddef>


bool rule::type::can_op_primitive(::type::EPrimitiveTypeKind term, ast::EBinOpType op)
{
  switch (term) {
  case ::type::EPrimitiveTypeKind::_bool:    return ast::EBinOpType_is_boolean(op);
  case ::type::EPrimitiveTypeKind::_cune:
  case ::type::EPrimitiveTypeKind::_rune:    return ast::EBinOpType_is_textual(op);
  case ::type::EPrimitiveTypeKind::_ssize:
  case ::type::EPrimitiveTypeKind::_s8:
  case ::type::EPrimitiveTypeKind::_s16:
  case ::type::EPrimitiveTypeKind::_s32:
  case ::type::EPrimitiveTypeKind::_s64:
  case ::type::EPrimitiveTypeKind::_s128:
  case ::type::EPrimitiveTypeKind::_ptrdiff:
  case ::type::EPrimitiveTypeKind::_usize:
  case ::type::EPrimitiveTypeKind::_u8:
  case ::type::EPrimitiveTypeKind::_u16:
  case ::type::EPrimitiveTypeKind::_u32:
  case ::type::EPrimitiveTypeKind::_u64:
  case ::type::EPrimitiveTypeKind::_u128:    return ast::EBinOpType_is_integral(op);
  case ::type::EPrimitiveTypeKind::_bsize:
  case ::type::EPrimitiveTypeKind::_b8:
  case ::type::EPrimitiveTypeKind::_b16:
  case ::type::EPrimitiveTypeKind::_b32:
  case ::type::EPrimitiveTypeKind::_b64:
  case ::type::EPrimitiveTypeKind::_b128:    return ast::EBinOpType_is_bitwise(op);
  case ::type::EPrimitiveTypeKind::_dsize:
  case ::type::EPrimitiveTypeKind::_d32:
  case ::type::EPrimitiveTypeKind::_d64:
  case ::type::EPrimitiveTypeKind::_d128:
  case ::type::EPrimitiveTypeKind::_udsize:
  case ::type::EPrimitiveTypeKind::_ud32:
  case ::type::EPrimitiveTypeKind::_ud64:
  case ::type::EPrimitiveTypeKind::_ud128:
  case ::type::EPrimitiveTypeKind::_fsize:
  case ::type::EPrimitiveTypeKind::_f16:
  case ::type::EPrimitiveTypeKind::_f32:
  case ::type::EPrimitiveTypeKind::_f64:
  case ::type::EPrimitiveTypeKind::_f80:
  case ::type::EPrimitiveTypeKind::_f128:    return ast::EBinOpType_is_decimal(op);
  case ::type::EPrimitiveTypeKind::_ptr:     return ast::EBinOpType_is_memory(op);
  case ::type::EPrimitiveTypeKind::_u0:
  case ::type::EPrimitiveTypeKind::NONE:     return false;
  }
}


bool rule::type::can_binary_op_primitive(::type::EPrimitiveTypeKind lhs, ::type::EPrimitiveTypeKind rhs)
{
}

bool rule::type::can_binary_op_form(const ast::SFM_Form& lhs, const ast::SFM_Form& rhs)
{
}


bool rule::type::can_cast_on_primitive_as_primitve(::type::EPrimitiveTypeKind term,
                                                   ::type::EPrimitiveTypeKind target_type)
{
  if (term == target_type) return true;

  static const bool table[static_cast<size_t>(magic_enum::enum_count<::type::EPrimitiveTypeKind>())
                          + 1][static_cast<size_t>(magic_enum::enum_count<::type::EPrimitiveTypeKind>()) + 1] = {
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
bool rule::type::can_cast_on_primitive_as_form(::type::EPrimitiveTypeKind term, const ast::SFM_Form& target_type)
{
}
bool rule::type::can_cast_on_form_as_primtive(const ast::SFM_Form& term, ::type::EPrimitiveTypeKind target_type)
{
}
bool rule::type::can_cast_on_form_as_form(const ast::SFM_Form& term, const ast::SFM_Form& target_type)
{
}