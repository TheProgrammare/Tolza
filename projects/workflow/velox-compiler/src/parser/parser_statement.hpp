
#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Statement final {
  Parser_Statement(Parser_Context& ctx)
    : p(ctx)
  {
  }

  // decl_possible means parse_instruction can failed without error to try to parse a declaration after
  [[nodiscard]] ast::_gnid parse_statement(bool p_is_silent_error = false);
  [[nodiscard]] ast::_gnid if_statement();
  [[nodiscard]] ast::_gnid for_statement();
  [[nodiscard]] ast::_gnid while_statement();
  [[nodiscard]] ast::_gnid loop_statement();
  [[nodiscard]] ast::_gnid match_statement();
  [[nodiscard]] ast::_gnid goto_statement();
  [[nodiscard]] ast::_gnid goto_label_statement();
  [[nodiscard]] ast::_gnid return_flow();

  Parser_Context& p;
};
} // namespace parser