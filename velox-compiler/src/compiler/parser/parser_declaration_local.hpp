#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "compiler/ast/ast_base.hpp"
#include "compiler/ast/ast_codeblock_instruction.hpp"
#include "compiler/ast/ast_evaluator.hpp"
#include "compiler/ast/ast_forward.hpp"

using proto_cb = std::function<ast::CodeBlock_instruction()>;

namespace parser
{
struct Parser_Context;

struct Parser_Declaration_Local {
  Parser_Declaration_Local(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::ALocal> parse_local(bool silent_error = false);

  [[nodiscard]] ast::declaration::local::Pattern_Element pattern_mapping(ECapability capa);

  [[nodiscard]] ast::Evaluator parse_evaluator(std::shared_ptr<ast::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern>
  parse_pattern(std::shared_ptr<ast::AExpression> comparison_ref);

  [[nodiscard]] std::shared_ptr<ast::declaration::local::Variable>               variable();
  [[nodiscard]] std::shared_ptr<ast::declaration::local::Variable_Unpack>        variable_unpack();
  [[nodiscard]] std::shared_ptr<ast::declaration::local::Lambda>                 lambda();
  [[nodiscard]] std::shared_ptr<ast::declaration::local::Capability>             capability();
  [[nodiscard]] std::unique_ptr<ast::declaration::local::CodeBlock>              code_block(bool     is_silent_error,
                                                                                            proto_cb in_function);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::CodeBlock>              code_block_instruction();
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Lambda_Capture>         lambda_capture();
  [[nodiscard]] std::vector<std::shared_ptr<ast::declaration::local::Parameter>> parameters();

  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Component>
  component_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> comp_id,
                    std::shared_ptr<ast::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Entity>
  entity_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> entity_id,
                 std::shared_ptr<ast::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Tuple>
  tuple_pattern(ECapability capa, std::shared_ptr<ast::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Enum>
  enum_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> enum_id,
               std::shared_ptr<ast::AExpression> comparison_ref);

  parser::Parser_Context& ctx;
};
} // namespace parser
