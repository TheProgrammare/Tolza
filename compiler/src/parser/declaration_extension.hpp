#pragma once

#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration_Extension final {
  Parser_Declaration_Extension(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  bool    in_extend = false;
  ast::ID current_self;
  ast::ID current_other;

  [[nodiscard]] ast::ID parse_extension();

  // extensions
  [[nodiscard]] ast::ID extend_fn(type::ID extended_type);
  [[nodiscard]] ast::ID extend_cast(type::ID extended_type);
  [[nodiscard]] ast::ID extend_op_bin(type::ID extended_type);
  [[nodiscard]] ast::ID extend_op_un(type::ID extended_type);
  [[nodiscard]] ast::ID extend_op_subscript(type::ID extended_type);
  [[nodiscard]] ast::ID extend_op_transfert(type::ID extended_type);
  [[nodiscard]] ast::ID extend_op_other(type::ID extended_type);

  parser::Parser_Context& p;
};
} // namespace parser
