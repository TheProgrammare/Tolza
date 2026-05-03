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

  [[nodiscard]] ast::_gnid parse_local(bool silent_error = false);


  [[nodiscard]] ast::_gnid parse_evaluator(ast::_gnid p_comparison_ref = ast::_gnid());

  [[nodiscard]] ast::_gnid              variable();
  [[nodiscard]] ast::_gnid              tuple_destructuring();
  [[nodiscard]] ast::_gnid              lambda();
  [[nodiscard]] ast::_gnid              capability();
  [[nodiscard]] ast::_gnid              parse_codeblock();
  [[nodiscard]] ast::_gnid              lambda_capture();
  [[nodiscard]] std::vector<ast::_gnid> parameters();

  [[nodiscard]] ast::_gnid parse_pattern(ast::_gnid p_comparison_ref);
  [[nodiscard]] ast::_gnid pattern_mapping(ast::ECapability p_parent_capa);
  [[nodiscard]] ast::_gnid component_pattern(ast::ECapability p_capa, ast::_gnid p_comp_id,
                                             ast::_gnid p_comparison_ref);
  [[nodiscard]] ast::_gnid entity_pattern(ast::ECapability p_capa, ast::_gnid p_entity_id, ast::_gnid p_comparison_ref);
  [[nodiscard]] ast::_gnid tuple_pattern(ast::ECapability p_capa, ast::_gnid p_comparison_ref);
  [[nodiscard]] ast::_gnid enum_pattern(ast::ECapability p_capa, ast::_gnid p_enum_id, ast::_gnid p_comparison_ref);

  parser::Parser_Context& p;
};
} // namespace parser
