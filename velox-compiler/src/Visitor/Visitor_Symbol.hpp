#pragma once

#include <memory>

#include "AST/AST_Base.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Declaration_COP.hpp"
#include "AST/AST_Expression.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Statement.hpp"
#include "Visitor_Default.hpp"

struct Visitor_Symbol : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  virtual ~Visitor_Symbol();

  AST::Declaration::COP::Entity* current_entity;

  std::shared_ptr<AST::Declaration::COP::Component> current_component;
  std::shared_ptr<AST::Declaration::Function>       current_function;
  std::shared_ptr<AST::Declaration::COP::System>    current_system;
  std::shared_ptr<AST::Declaration::Local::Lambda>  current_lambda;

  bool resolve_sym(AST::AIdentifier& expr, std::weak_ptr<Symbol_Data>& target_resolution, bool silentError = false);
};

inline const std::string SYM_HINT =
    "\n  - Did you write correctly the identifier?\n  - Did you access correctly to the path?\n  - Did you import the "
    "concerned module?\n  - Did you define correctly the type?";
