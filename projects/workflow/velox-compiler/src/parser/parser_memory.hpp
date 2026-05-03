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
  [[nodiscard]] ast::_gnid del();
  [[nodiscard]] ast::_gnid align();
  [[nodiscard]] ast::_gnid drop();

  Parser_Context& p;
};
} // namespace parser