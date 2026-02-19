#pragma once

#include <memory>
#include <optional>

#include "AST/AST_Base.hpp"
#include "AST/AST_Forward.hpp"

namespace PAR
{
struct Parser_Context;
struct Parser_Literal {
  Parser_Literal(Parser_Context &ctx) : ctx(ctx) {}

  [[nodiscard]] std::optional<std::unique_ptr<AST::ALiteral>> try_literal(bool is_silent_error = false);
  [[nodiscard]] std::unique_ptr<AST::Literal::Range>          literal_range(std::unique_ptr<AST::AExpression> start);

private:
  [[nodiscard]] std::unique_ptr<AST::Literal::Boolean>          literal_boolean();
  [[nodiscard]] std::unique_ptr<AST::Literal::Decimal>          literal_decimal();
  [[nodiscard]] std::unique_ptr<AST::Literal::Floating>         literal_floating_point();
  [[nodiscard]] std::unique_ptr<AST::Literal::Integral>         literal_integral();
  [[nodiscard]] std::unique_ptr<AST::Literal::ASCII>            literal_ascii();
  [[nodiscard]] std::unique_ptr<AST::Literal::Textual_Format>   literal_textual();
  [[nodiscard]] std::unique_ptr<AST::Literal::Format_Specifier> format_specifier();
  [[nodiscard]] std::unique_ptr<AST::ALiteral>                  literal_table();
  [[nodiscard]] std::unique_ptr<AST::Literal::Table_Population> literal_table_population();
  [[nodiscard]] std::unique_ptr<AST::Literal::Tuple>            literal_tuple();

public:
  [[nodiscard]] std::unique_ptr<AST::Literal::Entity>    literal_entity(std::unique_ptr<AST::AIdentifier> id);
  [[nodiscard]] std::unique_ptr<AST::Literal::Component> literal_component(std::unique_ptr<AST::AIdentifier> id);

private:
  Parser_Context &ctx;
};
} // namespace PAR