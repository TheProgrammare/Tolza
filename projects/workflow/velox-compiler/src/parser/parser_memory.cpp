#include "parser_memory.hpp"

#include "nexus/lexer/token.hpp"
#include "nexus/script.hpp"

#include "parser_context.hpp"

#include "parser_expression.hpp"
#include "parser_operation.hpp"
#include "parser_type.hpp"

#include "ast/ast_memory.hpp"

ast::_gnid parser::Parser_Memory::align()
{
  constexpr std::string_view hint = "define value memory alignment like: `align(a)`";
  parser_add_node(node, Memory_Align, p.peek(-1).id);

  p.expect(106, token::ETokenKind::OPEN_PAREN, "Expected start arg '('.", hint);
  node->target = p.p_expr->parse_expression();
  p.expect(107, token::ETokenKind::CLOSE_PAREN, "Expected end arg ')'.", hint);

  return node->node_id;
}

ast::_gnid parser::Parser_Memory::del()
{
  parser_add_node(node, Memory_Del, p.peek().id);
  p.match(token::ETokenKind::DEL);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}

ast::_gnid parser::Parser_Memory::drop()
{
  parser_add_node(node, Memory_Drop, p.peek().id);
  p.match(token::ETokenKind::DROP);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}
