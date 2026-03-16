#pragma once

#include <memory>

#include "ast/ast_base.hpp"
#include "ast/ast_forward.hpp"
#include "visitor_default.hpp"

struct Symbol_Data;

struct Visitor_Symbol : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  virtual ~Visitor_Symbol();

  ast::declaration::cop::Entity* current_entity;

  std::shared_ptr<ast::declaration::cop::Component> current_component;
  std::shared_ptr<ast::declaration::Function>       current_function;
  std::shared_ptr<ast::declaration::cop::System>    current_system;
  std::shared_ptr<ast::declaration::local::Lambda>  current_lambda;

  bool resolve_sym(ast::AIdentifier& expr, Symbol_Data*& target_resolution, bool silentError = false);

  void visit(ast::Expr_ID& n) override;
  void visit(ast::Expr_ID_Qualified& n) override;
  void visit(ast::Expr_ID_Type& n) override;
  void visit(ast::expression::Call& n) override;
};

inline const std::string SYM_HINT =
    "\n  - Did you write correctly the identifier?\n  - Did you access correctly to the path?\n  - Did you import "
    "the "
    "concerned module?\n  - Did you define correctly the type?";
