#include "ast_evaluator.hpp"

#include "ast_base.hpp"
#include "ast_declaration_local.hpp"

ast::Evaluator::Evaluator() = default;
ast::Evaluator::Evaluator(std::unique_ptr<AExpression> _node)
{
  if (dynamic_cast<ast::declaration::local::Pattern*>(node.get()))
    kind = EKind::Pattern;
  else
    kind = EKind::Condition;

  node = std::move(_node);
}


ast::AExpression* ast::Evaluator::get_condition() const
{
  if (kind == EKind::Condition)
    return node.get();
  else
    return nullptr;
}
ast::declaration::local::Pattern* ast::Evaluator::get_pattern() const
{
  if (kind == EKind::Pattern)
    return dynamic_cast<ast::declaration::local::Pattern*>(node.get());
  else
    return nullptr;
}

ast::Node* ast::Evaluator::get_node() const
{
  if (kind == EKind::None)
    return nullptr;
  else
    return dynamic_cast<ast::Node*>(node.get());
}
