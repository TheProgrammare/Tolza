#pragma once

#include <memory>

#include "compiler/ast/ast_forward.hpp"

struct Symbol_Data;

namespace parser
{
struct Parser_Context;

struct Parser_Declaration_COP {
  Parser_Declaration_COP(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::declaration::cop::Component> component();
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::Role>      role();
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::Entity>    entity();
  void parse_entity_declaration(std::shared_ptr<ast::declaration::cop::Entity> n_entity);
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::Entity_Op>
  _entity_op(std::shared_ptr<ast::declaration::cop::Entity> inEntity);
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::Entity_Cast>
  _entity_cast(std::shared_ptr<ast::declaration::cop::Entity> inEntity);
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::System>      system();
  [[nodiscard]] std::shared_ptr<ast::declaration::cop::System_Case> _system_case();

  Parser_Context& ctx;
};
} // namespace parser