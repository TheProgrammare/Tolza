#pragma once

#include <variant>

#include "AST_Base.hpp"

namespace AST
{

struct CodeBlock_instruction {
  std::variant<std::shared_ptr<ALocal>, std::unique_ptr<Node>> data;

  CodeBlock_instruction() = default;

  // no copy
  CodeBlock_instruction(const CodeBlock_instruction&)            = delete;
  CodeBlock_instruction& operator=(const CodeBlock_instruction&) = delete;

  // move semantic
  CodeBlock_instruction(CodeBlock_instruction&&) noexcept            = default;
  CodeBlock_instruction& operator=(CodeBlock_instruction&&) noexcept = default;

  Node* node() const
  {
    return std::visit([](auto const& v) -> Node* { return v.get(); }, data);
  }
};

} // namespace
  // AST