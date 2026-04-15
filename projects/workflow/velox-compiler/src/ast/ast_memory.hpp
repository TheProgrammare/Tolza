#pragma once

#include "ast_base.hpp"
#include <memory>
#include <string>

namespace ast
{
namespace memory
{

// del var
struct Del final : public Node {
  std::unique_ptr<AExpression> target;

  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "delete ptr";
  }
};

struct Align final : public Node {
  std::unique_ptr<AExpression> target;
  size_t                       align = 0;

  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "align(" + std::to_string(align) + ")";
  }
};

struct Drop final : public Node {
  std::unique_ptr<AExpression> target;

  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "drop";
  }
};

} // namespace memory
  // Memory
} // namespace ast
  // AST