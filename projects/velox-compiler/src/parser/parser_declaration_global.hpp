#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Declaration final {
  Parser_Declaration(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  [[nodiscard]] ast::ID parse_declaration();
  [[nodiscard]] ast::ID parse_codeblock_declaration(bool no_import = false, bool no_export = false);

  // declarations
  [[nodiscard]] ast::ID _module();
  [[nodiscard]] ast::ID type_alias();
  [[nodiscard]] ast::ID enumeration();
  [[nodiscard]] ast::ID _union();
  [[nodiscard]] ast::ID flag();
  [[nodiscard]] ast::ID global_variable();
  [[nodiscard]] ast::ID function();
  [[nodiscard]] ast::ID generic();
  [[nodiscard]] ast::ID goto_label_statement();

  parser::Parser_Context& p;
};
} // namespace parser
