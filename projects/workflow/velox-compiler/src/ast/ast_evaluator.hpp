#pragma once

#include <memory>

namespace ast
{
struct Node;
struct AExpression;

namespace declaration
{
namespace local
{

struct Pattern;

} // namespace local
} // namespace declaration

struct Evaluator final {
  Evaluator();
  Evaluator(std::unique_ptr<AExpression> _node);

  enum class EKind { None, Pattern, Condition };
  EKind kind = EKind::None;

  std::unique_ptr<AExpression> node;

  AExpression*                      get_condition() const;
  ast::declaration::local::Pattern* get_pattern() const;
  Node*                             get_node() const;
};

} // namespace ast
