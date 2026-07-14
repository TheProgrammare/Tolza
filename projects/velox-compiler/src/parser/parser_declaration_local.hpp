#pragma once

#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace parser
{
struct Parser_Context;

struct Parser_Declaration_Local final {
  Parser_Declaration_Local(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::ID parse_local(bool silent_error = false);


  [[nodiscard]] ast::ID parse_evaluator(ast::ID comparison_expr = ast::ID::invalid());

  [[nodiscard]] ast::ID              variable();
  [[nodiscard]] ast::ID              tuple_destructuring();
  [[nodiscard]] ast::ID              lambda();
  [[nodiscard]] ast::ID              capability();
  [[nodiscard]] ast::ID              parse_codeblock_instruction();
  [[nodiscard]] ast::ID              lambda_capture();
  [[nodiscard]] std::vector<ast::ID> parameters();

  [[nodiscard]] ast::ID parse_pattern(ast::ID comparison_expr);
  [[nodiscard]] ast::ID pattern_mapping(ast::ECapability p_parent_capa);
  [[nodiscard]] ast::ID facet_pattern(ast::ECapability p_capa, ast::ID p_facet_id, ast::ID p_comparison_ref);
  [[nodiscard]] ast::ID form_pattern(ast::ECapability p_capa, ast::ID p_form_id, ast::ID p_comparison_ref);
  [[nodiscard]] ast::ID tuple_pattern(ast::ECapability p_capa, ast::ID p_comparison_ref);
  [[nodiscard]] ast::ID enum_pattern(ast::ECapability p_capa, ast::ID p_enum_id, ast::ID p_comparison_ref);

  parser::Parser_Context& p;
};
} // namespace parser
