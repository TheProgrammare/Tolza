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

  [[nodiscard]] ast::_gnid try_literal(bool p_is_silent_error = false);
  [[nodiscard]] ast::_gnid literal_range(ast::_gnid p_start = ast::_gnid());


private:
  [[nodiscard]] ast::_gnid literal_boolean();
  [[nodiscard]] ast::_gnid literal_numeric();
  [[nodiscard]] ast::_gnid literal_decimal();
  [[nodiscard]] ast::_gnid literal_fixed_point(std::string_view p_val);
  [[nodiscard]] ast::_gnid literal_floating_point(std::string_view p_val);
  [[nodiscard]] ast::_gnid literal_integral(const token::Token& p_tok);
  [[nodiscard]] ast::_gnid literal_cune();
  [[nodiscard]] ast::_gnid format_specifier();
  [[nodiscard]] ast::_gnid literal_textual();
  [[nodiscard]] ast::_gnid literal_table();
  [[nodiscard]] ast::_gnid literal_table_population();
  [[nodiscard]] ast::_gnid literal_tuple();

public:
  [[nodiscard]] ast::_gnid literal_entity(ast::_gnid p_id = ast::_gnid());
  [[nodiscard]] ast::_gnid literal_component(ast::_gnid p_id = ast::_gnid());
  [[nodiscard]] ast::_gnid literal_field();


private:
  Parser_Context& p;
};
} // namespace parser