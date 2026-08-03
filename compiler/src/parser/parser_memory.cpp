#include "parser_memory.hpp"

#include "nexus/lexer/token.hpp"
#include "compiler/compilation_unit.hpp"

#include "parser_context.hpp"

#include "parser_expression.hpp"
#include "parser_operation.hpp"
#include "parser_type.hpp"

#include "ast/ast_memory.hpp"

ast::ID parser::Parser_Memory::align()
{
  constexpr std::string_view hint = "define value memory alignment like: `align(a)`";
  auto&                      node = p.add_get_node<ast::Memory_Align>(p.peek(-1).tokid);

  (void)p.expect(106, token::ETokenKind::L_PAREN, "Expected start arg '('.", hint);
  node.target = p.p_expr->parse_expression();
  (void)p.expect(107, token::ETokenKind::R_PAREN, "Expected end arg ')'.", hint);

  return node.nodeid();
}

ast::ID parser::Parser_Memory::del()
{
  auto& node = p.add_get_node<ast::Memory_Del>(p.peek().tokid);
  (void)p.match(token::ETokenKind::DEL);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}

ast::ID parser::Parser_Memory::drop()
{
  auto& node = p.add_get_node<ast::Memory_Drop>(p.peek().tokid);
  (void)p.match(token::ETokenKind::DROP);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}
