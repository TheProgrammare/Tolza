
#pragma once

#include <memory>

#include "ast/ast_forward.hpp"

enum class EBinOpType;
enum class EUnaryOpType;

namespace parser
{
struct Parser_Context;
struct Parser_Operator {
  Parser_Operator(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  bool no_literal_cop_mode = false;

  [[nodiscard]] std::unique_ptr<ast::AExpression>           try_operation();
  [[nodiscard]] std::unique_ptr<ast::operation::Assignment> assignment(std::unique_ptr<ast::AExpression> p_left);

private:
  [[nodiscard]] std::unique_ptr<ast::AExpression> _15_power();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _14_scalar();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _13_cumulate();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _12_shift();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _11_comparison();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _10_equality();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _9_bitwise_not();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _8_bitwise_and_nand();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _7_bitwise_xor_xnor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _6_bitwise_or_nor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _5_logical_not();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _4_logical_and_nand();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _3_logicial_xor_xnor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _2_logicial_or_nor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> _1_memory_distance();

  [[nodiscard]] std::unique_ptr<ast::operation::Binary>
  Create_BinOp(std::unique_ptr<ast::AExpression> p_left, EBinOpType p_op, std::unique_ptr<ast::AExpression> p_right);
  [[nodiscard]] std::unique_ptr<ast::operation::Unary> Create_UnOp(EUnaryOpType                      p_op,
                                                                   std::unique_ptr<ast::AExpression> p_base);

  Parser_Context& ctx;
};
} // namespace parser