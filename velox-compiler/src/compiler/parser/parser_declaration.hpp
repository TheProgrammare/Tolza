#pragma once

#include <memory>

#include "compiler/ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;

struct Parser_Declaration {
  Parser_Declaration(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::ADeclaration>            parse_declaration();
  // declarations
  [[nodiscard]] std::shared_ptr<ast::ADeclaration>            module();
  [[nodiscard]] std::shared_ptr<ast::declaration::Type_Alias> type_alias();
  [[nodiscard]] std::shared_ptr<ast::declaration::Enum>       enumeration();
  [[nodiscard]] std::shared_ptr<ast::declaration::Global>     global_variable();
  [[nodiscard]] std::shared_ptr<ast::declaration::Function>   function();
  [[nodiscard]] std::shared_ptr<ast::declaration::Generic>    generic();
  [[nodiscard]] std::shared_ptr<ast::statement::GoTo_Label>   goto_label_statement();

  parser::Parser_Context& ctx;
};
} // namespace parser
