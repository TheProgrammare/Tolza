#pragma once

#include <memory>
#include <vector>

#include "ast/ast_data.hpp"
#include "ast/ast_forward.hpp"
#include "ast/ast_evaluator.hpp"

namespace parser
{
struct Parser_Context;

struct Parser_Declaration_Local {
  Parser_Declaration_Local(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::ALocal> parse_local(bool silent_error = false);

  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Element>
  pattern_mapping(ast::declaration::local::Pattern& p_parent_pattern);

  [[nodiscard]] ast::Evaluator parse_evaluator(std::shared_ptr<ast::AExpression> p_comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern>
  parse_pattern(std::shared_ptr<ast::AExpression> p_comparison_ref);

  [[nodiscard]] std::shared_ptr<ast::declaration::local::Variable>               variable();
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Tuple_Destructuring>    tuple_destructuring();
  [[nodiscard]] std::shared_ptr<ast::declaration::local::Lambda>                 lambda();
  [[nodiscard]] std::shared_ptr<ast::declaration::local::Capability>             capability();
  [[nodiscard]] std::unique_ptr<ast::declaration::local::CodeBlock>              code_block_instruction();
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Lambda_Capture>         lambda_capture();
  [[nodiscard]] std::vector<std::shared_ptr<ast::declaration::local::Parameter>> parameters();

  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Component>
  component_pattern(ECapability p_capa, std::unique_ptr<ast::AIdentifier> p_comp_id,
                    std::shared_ptr<ast::AExpression> p_comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Entity>
  entity_pattern(ECapability p_capa, std::unique_ptr<ast::AIdentifier> p_entity_id,
                 std::shared_ptr<ast::AExpression> p_comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Tuple>
  tuple_pattern(ECapability p_capa, std::shared_ptr<ast::AExpression> p_comparison_ref);
  [[nodiscard]] std::unique_ptr<ast::declaration::local::Pattern_Enum>
  enum_pattern(ECapability p_capa, std::unique_ptr<ast::AIdentifier> p_enum_id,
               std::shared_ptr<ast::AExpression> p_comparison_ref);

  parser::Parser_Context& ctx;
};
} // namespace parser
