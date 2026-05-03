
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

  [[nodiscard]] ast::_gnid try_operation();
  [[nodiscard]] ast::_gnid assignment(ast::_gnid p_left);

private:
  [[nodiscard]] ast::_gnid _15_power();
  [[nodiscard]] ast::_gnid _14_scalar();
  [[nodiscard]] ast::_gnid _13_cumulate();
  [[nodiscard]] ast::_gnid _12_shift();
  [[nodiscard]] ast::_gnid _11_comparison();
  [[nodiscard]] ast::_gnid _10_equality();
  [[nodiscard]] ast::_gnid _9_bitwise_not();
  [[nodiscard]] ast::_gnid _8_bitwise_and_nand();
  [[nodiscard]] ast::_gnid _7_bitwise_xor_xnor();
  [[nodiscard]] ast::_gnid _6_bitwise_or_nor();
  [[nodiscard]] ast::_gnid _5_logical_not();
  [[nodiscard]] ast::_gnid _4_logical_and_nand();
  [[nodiscard]] ast::_gnid _3_logicial_xor_xnor();
  [[nodiscard]] ast::_gnid _2_logicial_or_nor();
  [[nodiscard]] ast::_gnid _1_memory();

  [[nodiscard]] ast::_gnid Create_BinOp(ast::_gnid p_left, ast::EBinOpType p_op, ast::_gnid p_right);
  [[nodiscard]] ast::_gnid Create_UnOp(ast::EUnaryOpType p_op, ast::_gnid p_base);

  Parser_Context& p;
};
} // namespace parser