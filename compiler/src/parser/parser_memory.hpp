#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Memory final {
  Parser_Memory(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  // special memory expression
  [[nodiscard]] ast::ID del();
  [[nodiscard]] ast::ID align();
  [[nodiscard]] ast::ID drop();

  Parser_Context& p;
};
} // namespace parser