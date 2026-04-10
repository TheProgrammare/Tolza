#include "ast_literal.hpp"

#include <iomanip>
#include <memory>

#include "ast/ast_numeric_128_bits.hpp"
#include "ast_data.hpp"
#include "ast_type.hpp"
#include "ast_inferred_type_singleton.hpp"
#include "ast_expression.hpp"

#include "visitor/visitor_base.hpp"

#include "codegen/visitor_codegen.hpp"


void ast::literal::Boolean::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Integral::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Decimal::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Floating::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::CUNE::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::RUNE::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Text_Pure::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Format_Specifier::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Text_Interpolation::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Textual_Format::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Table_Population::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Table::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Map::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Enum::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Tuple::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Range::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Structured_Data::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Entity::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::literal::Iterator::accept(Visitor_Base& v)
{
  v.visit(*this);
}


llvm::Value* ast::literal::Boolean::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Integral::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Decimal::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Floating::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::CUNE::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::RUNE::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Text_Pure::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Text_Interpolation::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Textual_Format::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Table_Population::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Table::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Map::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Enum::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Tuple::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Range::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Structured_Data::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Entity::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::literal::Iterator::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


std::string ast::literal::Table::debug_str() const
{
  if (!element_type) return "table empty {}";
  std::string out;
  out = "literal table[" + std::to_string(resolved_size.size()) + ":";
  for (size_t i = 0; i < resolved_size.size(); i++) {
    out += std::to_string(resolved_size[i]) + "x";
  }
  out += " -&gt; " + element_type->mangle_type() + "]";
  return out;
}

ast::literal::Boolean::Boolean()
{
  inferred_type = type::get_bool_type();
}


ast::literal::Boolean::Boolean(bool value)
  : val(value)
{
  inferred_type = type::get_bool_type();
}

ast::literal::Integral::Integral()
{
  switch (type) {
  // signed integers
  case EPrimType::i8:    inferred_type = type::get_i8_type(); break;
  case EPrimType::i16:   inferred_type = type::get_i16_type(); break;
  case EPrimType::i32:   inferred_type = type::get_i32_type(); break;
  case EPrimType::i64:   inferred_type = type::get_i64_type(); break;
  case EPrimType::i128:  inferred_type = type::get_i128_type(); break;
  case EPrimType::iSize: inferred_type = type::get_isize_type(); break;

  // unsigned integers
  case EPrimType::u8:    inferred_type = type::get_u8_type(); break;
  case EPrimType::u16:   inferred_type = type::get_u16_type(); break;
  case EPrimType::u32:   inferred_type = type::get_u32_type(); break;
  case EPrimType::u64:   inferred_type = type::get_u64_type(); break;
  case EPrimType::u128:  inferred_type = type::get_u128_type(); break;
  case EPrimType::uSize: inferred_type = type::get_usize_type(); break;

  // booleans
  case EPrimType::b8:    inferred_type = type::get_b8_type(); break;
  case EPrimType::b16:   inferred_type = type::get_b16_type(); break;
  case EPrimType::b32:   inferred_type = type::get_b32_type(); break;
  case EPrimType::b64:   inferred_type = type::get_b64_type(); break;
  case EPrimType::b128:  inferred_type = type::get_b128_type(); break;
  case EPrimType::bSize: inferred_type = type::get_bsize_type(); break;
  default:               break;
  }
}

ast::literal::Integral::Integral(const Int128& value)
  : val(value)
  , type(EPrimType::iSize)
{
  inferred_type = type::get_isize_type();
}

bool ast::literal::Integral::is_signed() const
{
  switch (type) {
  case EPrimType::ptrdiff:
  case EPrimType::iSize:
  case EPrimType::i8:
  case EPrimType::i16:
  case EPrimType::i32:
  case EPrimType::i64:
  case EPrimType::i128:    return true;
  default:                 return false;
  }
}


ast::literal::Decimal::Decimal()
{
  switch (raw_type) {
  case EPrimType::d32:    inferred_type = type::get_d32_type(); return;
  case EPrimType::d64:    inferred_type = type::get_d64_type(); return;
  case EPrimType::d128:   inferred_type = type::get_d128_type(); return;
  case EPrimType::dSize:  inferred_type = type::get_dsize_type(); return;
  case EPrimType::ud32:   inferred_type = type::get_ud32_type(); return;
  case EPrimType::ud64:   inferred_type = type::get_ud64_type(); return;
  case EPrimType::ud128:  inferred_type = type::get_ud128_type(); return;
  case EPrimType::udSize: inferred_type = type::get_udsize_type(); return;
  default:                return;
  }
}

ast::literal::Decimal::Decimal(const Int128& value, size_t _scale, EPrimType _raw_type)
  : val(value)
  , scale(_scale)
  , raw_type(_raw_type)
{
}


ast::literal::Floating::Floating()
{
  switch (type) {
  // signed
  // integers
  case EPrimType::f16:   inferred_type = type::get_f16_type(); break;
  case EPrimType::f32:   inferred_type = type::get_f32_type(); break;
  case EPrimType::f64:   inferred_type = type::get_f64_type(); break;
  case EPrimType::f80:   inferred_type = type::get_f80_type(); break;
  case EPrimType::f128:  inferred_type = type::get_f128_type(); break;
  case EPrimType::fSize: inferred_type = type::get_fsize_type(); break;
  default:               break;
  }
}

ast::literal::Floating::Floating(const Float128& value)
  : val(value)
  , type(EPrimType::fSize)
{
}

ast::literal::CUNE::CUNE()
{
  inferred_type = type::get_cune_type();
}


ast::literal::CUNE::CUNE(char value)
{
  inferred_type = type::get_cune_type();
  val           = value;
}

ast::literal::RUNE::RUNE()
{
  inferred_type = type::get_rune_type();
}

ast::literal::RUNE::RUNE(std::string codePoints_value)
{
  codePoints    = codePoints_value;
  inferred_type = type::get_rune_type();
}


std::string ast::literal::Textual_Format::debug_str() const
{
  std::string out;
  for (auto& val : values) {
    if (val.kind == Textual_Element::Kind::Lerp) out += "{";
    out += val.val->debug_str();
    if (val.kind == Textual_Element::Kind::Lerp) out += "}";
  }
  return out;
}
