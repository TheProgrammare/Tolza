#pragma once

#include "ast/data.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "nexus/forward.hpp"
#include "type/forward.hpp"

#include <string_view>
#include <vector>

namespace ast
{


[[nodiscard]] inline bool ENodeKind_is_symbol(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::Symbol_Id && kind <= ENodeKind::Symbol_Type;
}

[[nodiscard]] inline bool ENodeKind_is_literal(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Literal_Record;
}

[[nodiscard]] inline bool ENodeKind_is_callable(ENodeKind kind) noexcept
{
  return kind == ENodeKind::Expression_Invocation || kind == ENodeKind::Global_Function
         || kind == ENodeKind::Global_Extend_Fn || kind == ENodeKind::Local_Lambda || kind == ENodeKind::SFM_Rule
         || kind == ENodeKind::Expression_Invocation_Rule;
}

[[nodiscard]] inline bool ENodeKind_is_local(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::CodeBlock && kind <= ENodeKind::Local_Capability;
}

[[nodiscard]] inline bool ENodeKind_is_global(ENodeKind kind) noexcept
{
  return (kind >= ENodeKind::Global_Variable && kind <= ENodeKind::Global_Generic)
         || (kind >= ENodeKind::SFM_Facet && kind <= ENodeKind::SFM_Rule) || kind == ENodeKind::Import;
}

[[nodiscard]] inline bool ENodeKind_is_declaration(ENodeKind kind) noexcept
{
  return ENodeKind_is_local(kind) || ENodeKind_is_global(kind);
}

[[nodiscard]] inline bool ENodeKind_is_statement(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::Statement_If && kind <= ENodeKind::Statement_Match_Case;
}

[[nodiscard]] inline bool ENodeKind_is_operation(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::Operation_Cast_As && kind <= ENodeKind::Operation_Interval;
}

[[nodiscard]] inline bool ENodeKind_is_expression(ENodeKind kind) noexcept
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Expression_New_Ptr;
}

[[nodiscard]] std::string_view ENodeKind_to_sym(ENodeKind kind) noexcept;


[[nodiscard]] EOp_Unary ETokenKind_to_EOp_Unary(token::ETokenKind tok) noexcept;

[[nodiscard]] std::string_view EOp_Unary_to_sym(EOp_Unary opTy) noexcept;

[[nodiscard]] std::string_view EOp_Subscript_to_sym(EOp_Subscript opTy) noexcept;


[[nodiscard]] EOp_Bin ETokenKind_to_EOp_Bin(token::ETokenKind tok) noexcept;


[[nodiscard]] std::string_view EOp_Bin_to_sym(EOp_Bin op) noexcept;

[[nodiscard]] inline bool EOp_Bin_is_logical(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_and && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_memory(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_mem_add && op <= EOp_Bin::_mem_dist;
}

[[nodiscard]] inline bool EOp_Bin_is_comparison(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_ordering && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_bitwise(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_b_and && op <= EOp_Bin::_b_ror;
}

[[nodiscard]] inline bool EOp_Bin_is_boolean(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_ordering && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_textual(EOp_Bin op) noexcept
{
  return (op >= EOp_Bin::_eq && op <= EOp_Bin::_neqs) || op == EOp_Bin::_add;
}

[[nodiscard]] inline bool EOp_Bin_is_decimal(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_add && op <= EOp_Bin::_neqs;
}

[[nodiscard]] inline bool EOp_Bin_is_integral(EOp_Bin op) noexcept
{
  return op >= EOp_Bin::_add && op <= EOp_Bin::_neqs;
}

[[nodiscard]] EOp_Other ETokenStr_to_EOp_Other(std::string_view tok_str) noexcept;


[[nodiscard]] ECapability ETokenKind_to_ECapability(token::ETokenKind tok) noexcept;
[[nodiscard]] ECapability deduce_type_ECapability(bool is_complex, bool is_mut) noexcept;

[[nodiscard]] EPassMode ETokenKind_to_EPassMode(token::ETokenKind tok) noexcept;
[[nodiscard]] bool      EPassMode_Can_Default(EPassMode passMode) noexcept;

[[nodiscard]] EExprPassMode ETokenKind_to_EExprPassMode(token::ETokenKind tok) noexcept;

[[nodiscard]] EVariableKind ETokenKind_to_EVariableKind(token::ETokenKind tok) noexcept;

[[nodiscard]] ETransfertType ETokenKind_to_ETransfertType(token::ETokenKind tok) noexcept;

[[nodiscard]] EOp_Mem ETokenKind_to_EOp_Mem(token::ETokenKind tok) noexcept;

[[nodiscard]] ast::EVisibility       get_decl_visibility(ID nodeid) noexcept;
[[nodiscard]] const type::Prototype* get_prototype(ID id) noexcept;
[[nodiscard]] std::vector<ast::ID>   get_parameters(ID nodeid) noexcept;
[[nodiscard]] type::ID               get_decl_type(ID nodeid) noexcept;
[[nodiscard]] ast::ID                get_value(ID nodeid) noexcept;
[[nodiscard]] bool                   is_table_population(ID nodeid) noexcept;
[[nodiscard]] bool                   is_map(ID nodeid) noexcept;
[[nodiscard]] bool                   is_const(ID nodeid) noexcept;

} // namespace ast