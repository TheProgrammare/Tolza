#pragma once

#include "ast/ast_data.hpp"
#include "ast_base.hpp"
#include "visitor/visitor_base.hpp"
#include <memory>
#include <string>

namespace ast
{
namespace memory
{

// del var
struct Del final : public Node {
  std::unique_ptr<AExpression> target;

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
  std::string debug_str() const override
  {
    return "delete";
  }
};

struct Align final : public Node {
  std::unique_ptr<AExpression> target;
  size_t                       align = 0;

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
  std::string debug_str() const override
  {
    return "<mem> align(" + std::to_string(align) + ")";
  }
};

struct Drop final : public Node {
  std::unique_ptr<AExpression> target;

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
  std::string debug_str() const override
  {
    return "drop";
  }
};

} // namespace memory
  // Memory
} // namespace ast
  // AST