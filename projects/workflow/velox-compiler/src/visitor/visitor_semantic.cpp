
#include "visitor_semantic.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"

#include "rules/rule_type.hpp"


void Visitor_Semantic::visit(ast::declaration::Function& n)
{
  if (n.declaration_name == "main") {
    if (!n._scope.empty())
      add_error(215, n, "Illegal function reserved name 'main'. Or your main function musn't be scoped.", "");
    if (n.declaration_is_exported)
      add_error(216, n, "Illegal function reserved name 'main'. Or your main function musn't be exported.", "");
    if (n.declaration_is_external)
      add_error(217, n, "Illegal function reserved name 'main'. Or your main function musn't be external.", "");
  }
}


void Visitor_Semantic::visit(ast::operation::Binary& n)
{
  if (!n.left->expression_inferred_type->is_same(*n.right->expression_inferred_type)) {
    add_error(205, n,
              "Invalid binary operation on two differents types:\n  - left type: \""
                  + n.left->expression_inferred_type->debug_str() + "\"\n  - right type: \""
                  + n.right->expression_inferred_type->debug_str() + "\"",
              "");
    return;
  }

  if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(n.left->expression_inferred_type)) {
    if (!rule::type::can_op_primitive(ptr->type, n.op_ty)) {
      add_error(206, n, "Invalid operation \"" + EBinOpType_to_str(n.op_ty) + "\" on " + ptr->debug_str() + " type.",
                "");
      return;
    }
  }
}

void Visitor_Semantic::visit(ast::operation::Cast_As& n)
{
}


void Visitor_Semantic::visit(ast::expression::Call_Argument& n)
{
  if (n.variadic_arg) return;

  if (!n.expression_inferred_type->is_same(*n.fn_param_type->type)) {
    add_error_two_nodes(214, n, *n.fn_param_type, "Invalid argument type", "");
  }
}
