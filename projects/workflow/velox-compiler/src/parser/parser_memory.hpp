#pragma once

#include <memory>

#include "ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Memory {
  Parser_Memory(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  // special memory expression
  [[nodiscard]] std::unique_ptr<ast::memory::Del>   del();
  [[nodiscard]] std::unique_ptr<ast::memory::Align> align();
  [[nodiscard]] std::unique_ptr<ast::memory::Drop>  drop();

  Parser_Context& ctx;
};
} // namespace parser