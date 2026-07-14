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

  [[nodiscard]] ast::ID parse_expression();
  [[nodiscard]] ast::ID parse_expression_term();
  [[nodiscard]] ast::ID base_expression();
  [[nodiscard]] ast::ID suffix_expression(ast::ID p_base_expr);
  [[nodiscard]] ast::ID cast_as(ast::ID p_expr);
  [[nodiscard]] ast::ID assign(ast::ID left);


  [[nodiscard]] ast::ID              member_access(ast::ID p_left);
  [[nodiscard]] ast::ID              table_access(ast::ID p_target);
  [[nodiscard]] ast::ID              function_call(ast::ID p_callee);
  [[nodiscard]] ast::ID              rule_call(ast::ID p_target_form);
  [[nodiscard]] std::vector<ast::ID> call_arguments();
  [[nodiscard]] ast::ID              if_ternary();


  // special memory expression
  [[nodiscard]] ast::ID new_ptr();
  [[nodiscard]] ast::ID ptr_val();
  [[nodiscard]] ast::ID ref_of();
  [[nodiscard]] ast::ID mut_of();
  [[nodiscard]] ast::ID copy_of();
  [[nodiscard]] ast::ID addr_of();
  [[nodiscard]] ast::ID size_of();
  [[nodiscard]] ast::ID move();
  [[nodiscard]] ast::ID getbits(ast::ID p_expr);

  Parser_Context& p;
};
} // namespace parser