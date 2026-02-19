#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "AST/AST_Base.hpp"
#include "AST/AST_CodeBlock_Instruction.hpp"
#include "AST/AST_Evaluator.hpp"
#include "AST/AST_Forward.hpp"

using proto_cb = std::function<AST::CodeBlock_instruction()>;

namespace PAR
{
struct Parser_Context;

struct Parser_Declaration_Local {
  Parser_Declaration_Local(Parser_Context& ctx) : ctx(ctx) {}

  [[nodiscard]] std::shared_ptr<AST::ALocal> parse_local(bool silent_error = false);

  [[nodiscard]] AST::Declaration::Local::Pattern_Element pattern_mapping(ECapability capa);

  [[nodiscard]] AST::Evaluator parse_evaluator(std::shared_ptr<AST::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern>
  parse_pattern(std::shared_ptr<AST::AExpression> comparison_ref);

  [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Variable>               variable();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Variable_Unpack>        variable_unpack();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Lambda>                 lambda();
  [[nodiscard]] std::shared_ptr<AST::Declaration::Local::Capability>             capability();
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::CodeBlock>              code_block(bool     is_silent_error,
                                                                                            proto_cb in_function);
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::CodeBlock>              code_block_instruction();
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Lambda_Capture>         lambda_capture();
  [[nodiscard]] std::vector<std::shared_ptr<AST::Declaration::Local::Parameter>> parameters();

  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Component>
  component_pattern(ECapability capa, std::unique_ptr<AST::AIdentifier> comp_id,
                    std::shared_ptr<AST::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Entity>
  entity_pattern(ECapability capa, std::unique_ptr<AST::AIdentifier> entity_id,
                 std::shared_ptr<AST::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Tuple>
  tuple_pattern(ECapability capa, std::shared_ptr<AST::AExpression> comparison_ref);
  [[nodiscard]] std::unique_ptr<AST::Declaration::Local::Pattern_Enum>
  enum_pattern(ECapability capa, std::unique_ptr<AST::AIdentifier> enum_id,
               std::shared_ptr<AST::AExpression> comparison_ref);

  PAR::Parser_Context& ctx;
};
} // namespace PAR
