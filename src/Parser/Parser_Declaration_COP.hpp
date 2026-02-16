#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

struct Symbol_Data;
using SYM_DEFINITION = std::weak_ptr<Symbol_Data>;

namespace PAR
{
struct Parser_Context;

struct Parser_Declaration_COP {
  Parser_Declaration_COP(Parser_Context &ctx) : ctx(ctx) {}

  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::Component> component();
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::Role>      role();
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::Entity>    entity();
  void parse_entity_declaration(SYM_DEFINITION inEntity, std::shared_ptr<AST::Declaration::COP::Entity> n_entity);
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::Entity_Op>   _entity_op(SYM_DEFINITION inEntity);
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::Entity_Cast> _entity_cast(SYM_DEFINITION inEntity);
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::System>      system();
  [[nodiscard]] std::shared_ptr<AST::Declaration::COP::System_Case> _system_case();

  Parser_Context &ctx;
};
} // namespace PAR