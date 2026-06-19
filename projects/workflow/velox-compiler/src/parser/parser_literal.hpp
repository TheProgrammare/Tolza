#pragma once

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace parser
{
struct Parser_Literal final {
  Parser_Literal(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::ID try_literal(bool p_is_silent_error = false);
  [[nodiscard]] ast::ID literal_range(ast::ID p_start = ast::ID::invalid());


private:
  [[nodiscard]] ast::ID literal_boolean();
  [[nodiscard]] ast::ID literal_numeric();
  [[nodiscard]] ast::ID literal_decimal();
  [[nodiscard]] ast::ID literal_fixed_point(std::string_view p_val);
  [[nodiscard]] ast::ID literal_floating_point(std::string_view p_val);
  [[nodiscard]] ast::ID literal_integral(const token::Token& p_tok);
  [[nodiscard]] ast::ID literal_cune();
  [[nodiscard]] ast::ID format_specifier();
  [[nodiscard]] ast::ID literal_textual();
  [[nodiscard]] ast::ID literal_table();
  [[nodiscard]] ast::ID literal_table_population();
  [[nodiscard]] ast::ID literal_tuple();

public:
  [[nodiscard]] ast::ID literal_form(ast::ID nodeid = ast::ID::invalid());
  [[nodiscard]] ast::ID literal_facet(ast::ID nodeid = ast::ID::invalid());
  [[nodiscard]] ast::ID literal_field();


private:
  Parser_Context& p;
};
} // namespace parser