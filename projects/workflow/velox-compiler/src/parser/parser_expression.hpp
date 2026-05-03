#pragma once

#include <vector>

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Expression final {
  Parser_Expression(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::_gnid parse_expression();
  [[nodiscard]] ast::_gnid parse_expression_term();
  [[nodiscard]] ast::_gnid base_expression();
  [[nodiscard]] ast::_gnid suffix_expression(ast::_gnid& p_base_expr);
  [[nodiscard]] ast::_gnid cast_as(ast::_gnid& p_expr);


  [[nodiscard]] ast::_gnid              member_access(ast::_gnid& p_left);
  [[nodiscard]] ast::_gnid              table_access(ast::_gnid& p_target);
  [[nodiscard]] ast::_gnid              function_call(ast::_gnid& p_callee);
  [[nodiscard]] ast::_gnid              system_call(ast::_gnid& p_target_entity);
  [[nodiscard]] std::vector<ast::_gnid> call_arguments();
  [[nodiscard]] ast::_gnid              if_ternary();

  // special memory expression
  [[nodiscard]] ast::_gnid new_ptr();
  [[nodiscard]] ast::_gnid ptr_val();
  [[nodiscard]] ast::_gnid ref_of();
  [[nodiscard]] ast::_gnid mut_of();
  [[nodiscard]] ast::_gnid copy_of();
  [[nodiscard]] ast::_gnid addr_of();
  [[nodiscard]] ast::_gnid size_of();
  [[nodiscard]] ast::_gnid move();
  [[nodiscard]] ast::_gnid getbits(ast::_gnid& p_expr);

  Parser_Context& p;
};
} // namespace parser