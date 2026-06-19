#pragma once

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_sfm.hpp"

namespace rule::type
{

[[nodiscard]] bool can_op_primitive(::type::EPrimitiveTypeKind term, ast::EBinOpType op);
[[nodiscard]] bool can_op_form(const ast::SFM_Form& term, ast::EBinOpType op);
[[nodiscard]] bool can_binary_op_primitive(::type::EPrimitiveTypeKind lhs, ::type::EPrimitiveTypeKind rhs);
[[nodiscard]] bool can_binary_op_form(const ast::SFM_Form& lhs, const ast::SFM_Form& rhs);

// !!! not reliable !!!
[[nodiscard]] bool can_cast_on_primitive_as_primitve(::type::EPrimitiveTypeKind term,
                                                     ::type::EPrimitiveTypeKind target_type);
[[nodiscard]] bool can_cast_on_primitive_as_form(::type::EPrimitiveTypeKind term, const ast::SFM_Form& target_type);
[[nodiscard]] bool can_cast_on_form_as_primtive(const ast::SFM_Form& term, ::type::EPrimitiveTypeKind target_type);
[[nodiscard]] bool can_cast_on_form_as_form(const ast::SFM_Form& term, const ast::SFM_Form& target_type);

} // namespace rule::type