
#pragma once

#include "ast/ast_declaration_global.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_operation.hpp"
#include "resolver_base.hpp"

namespace resolver
{


constexpr std::string_view ERR_TY_NOT_FOUND = "Type not found!";
constexpr std::string_view HINT_NOT_FOUND =
    R"(  - Did you use a correct identifier ?
  - Did you use a correct type ?
  - Did you use a correct path ?
  - Did you are in correct scope ?)";

struct Semantic : Base {
  // keep parent constructor
  using Base::Base;

  [[nodiscard]] bool start_resolver();

  void resolve_Global_Function(ast::Global_Function& n);
  void resolve_Operation_Binary(ast::Operation_Binary& n);
  void resolve_Operation_Cast_As(ast::Operation_Cast_As& n);
  void resolve_Expression_Call(ast::Expression_Call& n);
};

} // namespace resolver
