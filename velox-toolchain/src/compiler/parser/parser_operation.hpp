
#pragma once

#include <memory>

#include "compiler/ast/ast_forward.hpp"

enum class EBinOpType;
enum class EUnaryOpType;

namespace parser
{
struct Parser_Context;
struct Parser_Operator {
  Parser_Operator(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  bool no_literal_cop_mode = false;

  [[nodiscard]] std::unique_ptr<ast::AExpression>           try_operation();
  [[nodiscard]] std::unique_ptr<ast::operation::Assignment> assignment(std::unique_ptr<ast::AExpression> left);

private:
  [[nodiscard]] std::unique_ptr<ast::AExpression> power();
  [[nodiscard]] std::unique_ptr<ast::AExpression> multiply();
  [[nodiscard]] std::unique_ptr<ast::AExpression> add();
  [[nodiscard]] std::unique_ptr<ast::AExpression> shift();
  [[nodiscard]] std::unique_ptr<ast::AExpression> comparison();
  [[nodiscard]] std::unique_ptr<ast::AExpression> equality();
  [[nodiscard]] std::unique_ptr<ast::AExpression> bitwise_not();
  [[nodiscard]] std::unique_ptr<ast::AExpression> bitwise_and_nand();
  [[nodiscard]] std::unique_ptr<ast::AExpression> bitwise_xor_xnor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> bitwise_or_nor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> logical_not();
  [[nodiscard]] std::unique_ptr<ast::AExpression> logical_and_nand();
  [[nodiscard]] std::unique_ptr<ast::AExpression> logicial_xor_xnor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> logicial_or_nor();
  [[nodiscard]] std::unique_ptr<ast::AExpression> memory_distance();

  [[nodiscard]] std::unique_ptr<ast::operation::Binary>
  Create_BinOp(std::unique_ptr<ast::AExpression> left, EBinOpType op, std::unique_ptr<ast::AExpression> right);
  [[nodiscard]] std::unique_ptr<ast::operation::Unary> Create_UnOp(EUnaryOpType                      op,
                                                                   std::unique_ptr<ast::AExpression> base);

  Parser_Context& ctx;
};
} // namespace parser