#pragma once

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_cop.hpp"

namespace rule
{
namespace type
{

bool can_op_primitive(::type::EPrimitiveTypeKind term, ast::EBinOpType op);
bool can_op_entity(const ast::COP_Entity& term, ast::EBinOpType op);
bool can_binary_op_primitive(::type::EPrimitiveTypeKind lhs, ::type::EPrimitiveTypeKind rhs);
bool can_binary_op_entity(const ast::COP_Entity& lhs, const ast::COP_Entity& rhs);

// !!! not reliable !!!
bool can_cast_on_primitive_as_primitve(::type::EPrimitiveTypeKind term, ::type::EPrimitiveTypeKind target_type);
bool can_cast_on_primitive_as_entity(::type::EPrimitiveTypeKind term, const ast::COP_Entity& target_type);
bool can_cast_on_entity_as_primtive(const ast::COP_Entity& term, ::type::EPrimitiveTypeKind target_type);
bool can_cast_on_entity_as_entity(const ast::COP_Entity& term, const ast::COP_Entity& target_type);

} // namespace type
} // namespace rule