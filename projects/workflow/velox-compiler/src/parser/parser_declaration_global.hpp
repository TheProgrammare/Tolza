#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration final {
  Parser_Declaration(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::_gnid parse_declaration();
  // declarations
  [[nodiscard]] ast::_gnid _module();
  [[nodiscard]] ast::_gnid type_alias();
  [[nodiscard]] ast::_gnid enumeration();
  [[nodiscard]] ast::_gnid _union();
  [[nodiscard]] ast::_gnid flag();
  [[nodiscard]] ast::_gnid global_variable();
  [[nodiscard]] ast::_gnid function();
  [[nodiscard]] ast::_gnid generic();
  [[nodiscard]] ast::_gnid goto_label_statement();

  std::string extern_abi;

  parser::Parser_Context& p;
};
} // namespace parser
