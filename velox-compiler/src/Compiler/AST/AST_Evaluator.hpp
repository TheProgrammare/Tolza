#pragma once

#include <memory>

#include "AST_Base.hpp"
#include "AST_Declaration_Local.hpp"
#include "AST_Forward.hpp"

namespace AST
{

struct Evaluator final {
  enum class EKind { None, Pattern, Condition };
  EKind kind = EKind::None;

  std::unique_ptr<Declaration::Local::Pattern> pattern;
  std::unique_ptr<AExpression>                 condition;

  AExpression* node() const
  {
    if (kind == EKind::Pattern)
      return dynamic_cast<AExpression*>(pattern.get());
    else
      return condition.get();
  }

  Evaluator() = default;

  Evaluator(std::unique_ptr<Declaration::Local::Pattern> _pattern)
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

} // namespace
  // AST