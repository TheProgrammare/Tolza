#pragma once

#include "ast/ast_data.hpp"
#include "ast/ast_declaration_cop.hpp"
enum class EPrimType;

namespace rule
{
namespace type
{

bool can_op_primitive(EPrimType term, EBinOpType op);
bool can_op_entity(const ast::declaration::cop::Entity& term, EBinOpType op);
bool can_binary_op_primitive(EPrimType lhs, EPrimType rhs);
bool can_binary_op_entity(const ast::declaration::cop::Entity& lhs, const ast::declaration::cop::Entity& rhs);

// !!! not reliable !!!
bool can_cast_on_primitive_as_primitve(EPrimType term, EPrimType target_type);
bool can_cast_on_primitive_as_entity(EPrimType term, const ast::declaration::cop::Entity& target_type);
bool can_cast_on_entity_as_primtive(const ast::declaration::cop::Entity& term, EPrimType target_type);
bool can_cast_on_entity_as_entity(const ast::declaration::cop::Entity& term,
                                  const ast::declaration::cop::Entity& target_type);

} // namespace type
} // namespace rule