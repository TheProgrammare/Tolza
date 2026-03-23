#pragma once

#include <memory>

namespace ast
{
struct Node;
struct ALocal;

struct CodeBlock_instruction {
  enum class EKind { None, Shared_local, Unique_base };

  CodeBlock_instruction();
  CodeBlock_instruction(std::shared_ptr<ALocal> _data_local);
  CodeBlock_instruction(std::unique_ptr<Node> _data_base);

  EKind                   kind = EKind::None;
  std::shared_ptr<ALocal> data_local;
  std::unique_ptr<Node>   data_base;

  // no copy
  CodeBlock_instruction(const CodeBlock_instruction&)            = delete;
  CodeBlock_instruction& operator=(const CodeBlock_instruction&) = delete;

  // move semantic
  CodeBlock_instruction(CodeBlock_instruction&&) noexcept            = default;
  CodeBlock_instruction& operator=(CodeBlock_instruction&&) noexcept = default;

  Node* node() const;
  bool  is_valid() const
  {
    switch (kind) {
    case EKind::None:         return false;
    case EKind::Shared_local: return data_local.get();
    case EKind::Unique_base:  return data_base.get();
    }
  }
};

} // namespace ast
  // AST