#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration_SFM final {
  Parser_Declaration_SFM(Parser_Context& ctx)
    : p(ctx)
  {
  }

  [[nodiscard]] ast::ID facet();
  [[nodiscard]] ast::ID view();
  [[nodiscard]] ast::ID form();
  void                  parse_form_declaration(ast::SFM_Form& form);
  [[nodiscard]] ast::ID rule();
  [[nodiscard]] ast::ID _rule_case();

  Parser_Context& p;
};
} // namespace parser