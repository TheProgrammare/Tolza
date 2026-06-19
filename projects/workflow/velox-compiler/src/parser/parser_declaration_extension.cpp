#include "parser_declaration_extension.hpp"

#include "nexus/lexer/token.hpp"
#include "parser/parser_type.hpp"
#include "parser_context.hpp"

ast::ID parser::Parser_Declaration_Extension::parse_extension() noexcept
{
  (void)p.match(token::ETokenKind::EXTENSION);

  const auto tyid = p.p_type->parse_type();

  const auto tokkind = p.peek().kind;

  switch (tokkind) {
  case token::ETokenKind::FUNCTION: return extend_fn(tyid);
  case token::ETokenKind::AS:
  case token::ETokenKind::OP:
  }
}

ast::ID parser::Parser_Declaration_Extension::extend_fn(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_cast(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_op_bin(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_op_un(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_op_access(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_op_transfert(ast::ID target_type) noexcept
{
}
ast::ID parser::Parser_Declaration_Extension::extend_op_other(ast::ID target_type) noexcept
{
}