#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration_COP final {
  Parser_Declaration_COP(Parser_Context& ctx)
    : p(ctx)
  {
  }

  [[nodiscard]] ast::_gnid component();
  [[nodiscard]] ast::_gnid role();
  [[nodiscard]] ast::_gnid entity();
  void                     parse_entity_declaration(ast::COP_Entity& entity);
  [[nodiscard]] ast::_gnid _entity_op();
  [[nodiscard]] ast::_gnid _entity_access_op();
  [[nodiscard]] ast::_gnid _entity_cast(ast::_gnid entity);
  [[nodiscard]] ast::_gnid system();
  [[nodiscard]] ast::_gnid _system_case();

  Parser_Context& p;
};
} // namespace parser