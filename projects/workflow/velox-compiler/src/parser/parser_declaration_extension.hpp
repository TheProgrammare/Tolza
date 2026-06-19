#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration_Extension final {
  Parser_Declaration_Extension(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::ID parse_extension() noexcept;

  // extensions
  [[nodiscard]] ast::ID extend_fn(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_cast(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_op_bin(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_op_un(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_op_access(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_op_transfert(ast::ID target_type) noexcept;
  [[nodiscard]] ast::ID extend_op_other(ast::ID target_type) noexcept;

  parser::Parser_Context& p;
};
} // namespace parser
