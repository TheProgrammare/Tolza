
#pragma once

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Operator final {
  Parser_Operator(Parser_Context& p_ctx)
    : p(p_ctx)
  {
  }

  bool no_literal_cop_mode = false;

  [[nodiscard]] ast::ID try_operation();
  [[nodiscard]] ast::ID assignment(ast::ID p_left);

private:
  [[nodiscard]] ast::ID _15_power();
  [[nodiscard]] ast::ID _14_scalar();
  [[nodiscard]] ast::ID _13_cumulate();
  [[nodiscard]] ast::ID _12_shift();
  [[nodiscard]] ast::ID _11_comparison();
  [[nodiscard]] ast::ID _10_equality();
  [[nodiscard]] ast::ID _9_bitwise_not();
  [[nodiscard]] ast::ID _8_bitwise_and_nand();
  [[nodiscard]] ast::ID _7_bitwise_xor_xnor();
  [[nodiscard]] ast::ID _6_bitwise_or_nor();
  [[nodiscard]] ast::ID _5_logical_not();
  [[nodiscard]] ast::ID _4_logical_and_nand();
  [[nodiscard]] ast::ID _3_logicial_xor_xnor();
  [[nodiscard]] ast::ID _2_logicial_or_nor();
  [[nodiscard]] ast::ID _1_memory();
  [[nodiscard]] ast::ID _0_unary();

  [[nodiscard]] ast::ID Create_BinOp(ast::ID p_left, ast::EBinOpType p_op, ast::ID p_right);
  [[nodiscard]] ast::ID Create_UnOp(ast::EUnaryOpType p_op, ast::ID p_base);

  Parser_Context& p;
};
} // namespace parser