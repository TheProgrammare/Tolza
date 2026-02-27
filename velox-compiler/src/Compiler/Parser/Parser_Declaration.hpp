#pragma once

#include <memory>

#include "Compiler/AST/AST_Forward.hpp"

namespace PAR
{
struct Parser_Context;

struct Parser_Declaration {
  Parser_Declaration(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<AST::ADeclaration>            parse_declaration();
  // declarations
  [[nodiscard]] std::shared_ptr<AST::ADeclaration>            module();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Type_Alias> type_alias();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Enum>       enumeration();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Global>     global_variable();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Function>   function();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Generic>    generic();
  [[nodiscard]] std::shared_ptr<AST::Statement::GoTo_Label>   goto_label_statement();

  PAR::Parser_Context& ctx;
};
} // namespace PAR
