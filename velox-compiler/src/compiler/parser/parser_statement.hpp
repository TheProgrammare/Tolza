
#pragma once

#include <memory>

#include "compiler/ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Statement {
  Parser_Statement(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  // decl_possible means parse_instruction can failed without error to try to parse a declaration after
  [[nodiscard]] std::unique_ptr<ast::Node>                  parse_statement(bool is_silent_error = false);
  [[nodiscard]] std::unique_ptr<ast::statement::If>         if_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::For>        for_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::While>      while_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::Loop>       loop_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::Match>      match_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::GoTo>       goto_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::GoTo_Label> goto_label_statement();
  [[nodiscard]] std::unique_ptr<ast::statement::Return>     return_flow();

  Parser_Context& ctx;
};
} // namespace parser