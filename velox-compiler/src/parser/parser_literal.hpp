#pragma once

#include <memory>

#include "ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Literal {
  Parser_Literal(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::unique_ptr<ast::ALiteral>       try_literal(bool is_silent_error = false);
  [[nodiscard]] std::unique_ptr<ast::literal::Range> literal_range(std::unique_ptr<ast::AExpression> start);


private:
  [[nodiscard]] std::unique_ptr<ast::literal::Boolean>          literal_boolean();
  [[nodiscard]] std::unique_ptr<ast::literal::Decimal>          literal_decimal();
  [[nodiscard]] std::unique_ptr<ast::literal::Floating>         literal_floating_point();
  [[nodiscard]] std::unique_ptr<ast::literal::Integral>         literal_integral();
  [[nodiscard]] std::unique_ptr<ast::literal::ASCII>            literal_ascii();
  [[nodiscard]] std::unique_ptr<ast::literal::Format_Specifier> format_specifier();
  [[nodiscard]] std::unique_ptr<ast::literal::Textual_Format>   literal_textual();
  [[nodiscard]] std::unique_ptr<ast::ALiteral>                  literal_table();
  [[nodiscard]] std::unique_ptr<ast::literal::Table_Population> literal_table_population();
  [[nodiscard]] std::unique_ptr<ast::literal::Tuple>            literal_tuple();

public:
  [[nodiscard]] std::unique_ptr<ast::literal::Entity>           literal_entity(std::unique_ptr<ast::AIdentifier> id);
  [[nodiscard]] std::unique_ptr<ast::literal::Structured_Data>  literal_component(std::unique_ptr<ast::AIdentifier> id);
  [[nodiscard]] std::unique_ptr<ast::expression::Call_Argument> literal_field();


private:
  Parser_Context& ctx;
};
} // namespace parser