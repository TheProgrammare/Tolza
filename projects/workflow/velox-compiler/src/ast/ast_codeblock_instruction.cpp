#include "ast_codeblock_instruction.hpp"

#include "ast_base.hpp"

ast::CodeBlock_instruction::CodeBlock_instruction::CodeBlock_instruction() = default;

ast::CodeBlock_instruction::CodeBlock_instruction(std::shared_ptr<ALocal> p_data_local)
  : kind(EKind::Shared_local)
  , data_local(p_data_local)
{
}
ast::CodeBlock_instruction::CodeBlock_instruction(std::unique_ptr<Node> p_data_base)
  : kind(EKind::Unique_base)
  , data_base(std::move(p_data_base))
{
}


ast::Node* ast::CodeBlock_instruction::node() const
{
  if (kind == EKind::Shared_local) return data_local.get();
  if (kind == EKind::Unique_base) return data_base.get();
  return nullptr;
}