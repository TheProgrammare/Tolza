#pragma once

#include <memory>

#include "ast_base.hpp"
#include "ast_declaration_local.hpp"
#include "ast_forward.hpp"

namespace ast
{

struct Evaluator final {
  enum class EKind { None, Pattern, Condition };
  EKind kind = EKind::None;

  std::unique_ptr<declaration::local::Pattern> pattern;
  std::unique_ptr<AExpression>                 condition;

  AExpression* node() const
  {
    if (kind == EKind::Pattern)
      return dynamic_cast<AExpression*>(pattern.get());
    else
      return condition.get();
  }

  Evaluator() = default;

  Evaluator(std::unique_ptr<declaration::local::Pattern> _pattern)
    : pattern(std::move(_pattern))
    , kind(EKind::Pattern)
  {
  }

  Evaluator(std::unique_ptr<AExpression> _condition)
    : condition(std::move(_condition))
    , kind(EKind::Condition)
  {
  }
};

} // namespace ast
  // AST