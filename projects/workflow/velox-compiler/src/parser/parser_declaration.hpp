#pragma once

#include <memory>

#include "ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;

struct Parser_Declaration {
  Parser_Declaration(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::ADeclaration>            parse_declaration();
  // declarations
  [[nodiscard]] std::shared_ptr<ast::ADeclaration>            _module();
  [[nodiscard]] std::shared_ptr<ast::declaration::Type_Alias> type_alias();
  [[nodiscard]] std::shared_ptr<ast::declaration::Enum>       enumeration();
  [[nodiscard]] std::shared_ptr<ast::declaration::Union>      _union();
  [[nodiscard]] std::shared_ptr<ast::declaration::Flag>       flag();
  [[nodiscard]] std::shared_ptr<ast::declaration::Global>     global_variable();
  [[nodiscard]] std::shared_ptr<ast::declaration::Function>   function();
  [[nodiscard]] std::shared_ptr<ast::declaration::Generic>    generic();
  [[nodiscard]] std::shared_ptr<ast::statement::GoTo_Label>   goto_label_statement();

  parser::Parser_Context& ctx;
};
} // namespace parser
