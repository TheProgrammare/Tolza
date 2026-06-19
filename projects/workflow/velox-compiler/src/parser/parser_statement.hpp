
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
  [[nodiscard]] ast::ID parse_statement(bool p_is_silent_error = false);
  [[nodiscard]] ast::ID if_statement();
  [[nodiscard]] ast::ID for_statement();
  [[nodiscard]] ast::ID while_statement();
  [[nodiscard]] ast::ID loop_statement();
  [[nodiscard]] ast::ID match_statement();
  [[nodiscard]] ast::ID goto_statement();
  [[nodiscard]] ast::ID goto_label_statement();
  [[nodiscard]] ast::ID return_flow();

  Parser_Context& p;
};
} // namespace parser