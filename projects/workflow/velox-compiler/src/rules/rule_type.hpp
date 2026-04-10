#pragma once

#include "ast/ast_data.hpp"
#include "ast/ast_declaration_cop.hpp"
enum class EPrimType;

namespace rule
{
namespace type
{

bool op_on_primitive(EPrimType term, EBinOpType op);
bool op_on_entity(const ast::declaration::cop::Entity& term, EBinOpType op);
bool binary_op_primitive(EPrimType lhs, EPrimType rhs);
bool binary_op_entity(const ast::declaration::cop::Entity& lhs, const ast::declaration::cop::Entity& rhs);

bool cast_on_primitive_as_primitve(EPrimType term, EPrimType target_type);
bool cast_on_primitive_as_entity(EPrimType term, const ast::declaration::cop::Entity& target_type);
bool cast_on_entity_as_primtive(const ast::declaration::cop::Entity& term, EPrimType target_type);
bool cast_on_entity_as_entity(const ast::declaration::cop::Entity& term,
                              const ast::declaration::cop::Entity& target_type);

} // namespace type
} // namespace rule