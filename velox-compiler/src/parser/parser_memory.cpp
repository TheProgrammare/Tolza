#include "parser_memory.hpp"

#include "parser_context.hpp"

#include "parser_expression.hpp"
#include "parser_operation.hpp"
#include "parser_type.hpp"

#include "ast/ast_memory.hpp"

std::unique_ptr<ast::memory::Align> parser::Parser_Memory::align()
{
  static const std::string hint = "define value memory alignment like: `align(a)`";
  auto                     node = ctx.Create_Node<ast::memory::Align>(ctx.tok_v.peek(-1));

  ctx.tok_v.expect(106, TokTy::OPEN_PAREN, "Expected start arg '('.", hint);
  node->target = ctx.p_expr->parse_expression();
  ctx.tok_v.expect(107, TokTy::CLOSE_PAREN, "Expected end arg ')'.", hint);

  return node;
}

std::unique_ptr<ast::memory::Del> parser::Parser_Memory::del()
{
  auto node = ctx.Create_Node<ast::memory::Del>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::DEL);
  node->target = ctx.p_expr->parse_expression();

  return node;
}

std::unique_ptr<ast::memory::Drop> parser::Parser_Memory::drop()
{
  auto node = ctx.Create_Node<ast::memory::Drop>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::DROP);
  node->target = ctx.p_expr->parse_expression();

  return node;
}
