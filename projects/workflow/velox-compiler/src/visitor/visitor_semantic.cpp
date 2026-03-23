
#include "visitor_semantic.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"


void Visitor_Semantic::visit(ast::literal::Textual_Format& n)
{
  n.is_pure_literal_text = n.values.size() == 1 && n.values[0].kind == ast::literal::Textual_Element::Kind::Text;
}
