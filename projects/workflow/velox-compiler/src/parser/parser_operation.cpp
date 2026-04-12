
#include "parser_operation.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_operation.hpp"

#include "parser_context.hpp"
#include "parser_expression.hpp"

// Parsing
// precedence
// hierarchy:
//
// primary
//   -> unary (+, -, NOT, B_NOT)
//     ->   power   (^)
//       -> multiply/divide/modulo (*, /, %, %/)
//         -> add/subtract (+, -)
//           -> bitwise shift (<<, >>)
//             -> comparison (<, >, <=, >=)
//               -> equality (==, !=)
//                 -> bitwise AND (B_AND)
//                   -> bitwise XOR (B_XOR)
//                     -> bitwise OR (B_OR)
//                       -> logical AND (AND, NAND)
//                         -> logical XOR (XOR)
//                           -> logical OR (OR, NOR)
//                             -> logical XNOR (XNOR)

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_15_power()
{
  auto left = ctx.p_expr->parse_expression_term();
  if (ctx.tok_v.match(TokTy::OP_POWER)) {
    auto right = _15_power();
    return Create_BinOp(std::move(left), EBinOpType::Pow, std::move(right));
  }
  return left;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_14_scalar()
{
  auto node = _15_power();
  while (ctx.tok_v.match_any(
      {TokTy::OP_ASTERISK, TokTy::OP_DIVIDE, TokTy::OP_MODULO, TokTy::OP_QUOTIEN, TokTy::OP_REMAIN})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _15_power();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_13_cumulate()
{
  auto node = _14_scalar();
  while (ctx.tok_v.match_any({TokTy::OP_PLUS, TokTy::OP_MINUS})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _14_scalar();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_12_shift()
{
  auto node = _13_cumulate();
  while (ctx.tok_v.match_any(k_op_bitwise_shift)) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _13_cumulate();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_11_comparison()
{
  auto node = _12_shift();
  while (ctx.tok_v.match_any({TokTy::OPEN_BRACKETS, TokTy::CLOSE_BRACKETS, TokTy::OP_LEQ, TokTy::OP_GEQ})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _12_shift();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_10_equality()
{
  auto node = _11_comparison();
  while (ctx.tok_v.match_any({TokTy::OP_EQ, TokTy::OP_NEQ, TokTy::OP_EQS, TokTy::OP_NEQS, TokTy::IN, TokTy::IS})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _11_comparison();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_9_bitwise_not()
{
  if (ctx.tok_v.match(TokTy::B_NOT)) {
    auto op      = TokTy_to_EUnaryOpType(ctx.tok_v.peek(-1).type);
    auto operand = _9_bitwise_not();
    return Create_UnOp(op, std::move(operand));
  }
  return _10_equality();
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_8_bitwise_and_nand()
{
  auto node = _9_bitwise_not();
  while (ctx.tok_v.match_any({TokTy::B_AND, TokTy::B_NAND})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _9_bitwise_not();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_7_bitwise_xor_xnor()
{
  auto node = _8_bitwise_and_nand();
  while (ctx.tok_v.match_any({TokTy::B_XOR, TokTy::B_XNOR})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _8_bitwise_and_nand();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_6_bitwise_or_nor()
{
  auto node = _7_bitwise_xor_xnor();
  while (ctx.tok_v.match_any({TokTy::B_OR, TokTy::B_NOR})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _7_bitwise_xor_xnor();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_5_logical_not()
{
  if (ctx.tok_v.match(TokTy::NOT)) {
    auto op      = TokTy_to_EUnaryOpType(ctx.tok_v.peek(-1).type);
    auto operand = _5_logical_not();
    return Create_UnOp(op, std::move(operand));
  }
  return _6_bitwise_or_nor();
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_4_logical_and_nand()
{
  auto node = _5_logical_not();
  while (ctx.tok_v.match_any({TokTy::AND, TokTy::NAND})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _5_logical_not();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_3_logicial_xor_xnor()
{
  auto node = _4_logical_and_nand();
  while (ctx.tok_v.match_any({TokTy::XOR, TokTy::XNOR})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
    auto right = _4_logical_and_nand();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_2_logicial_or_nor()
{
  auto node = _3_logicial_xor_xnor();
  while (ctx.tok_v.check_any({TokTy::OR, TokTy::NOR})) {
    auto op    = TokTy_to_EBinOpType(ctx.tok_v.next().type);
    auto right = _3_logicial_xor_xnor();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::_1_memory_distance()
{
  auto node = _2_logicial_or_nor();
  while (ctx.tok_v.check(TokTy::MEM_DIST)) {
    auto dist   = ctx.Create_Node<ast::operation::Ptr_Dist>(ctx.tok_v.peek());
    auto right  = _2_logicial_or_nor();
    dist->left  = std::move(node);
    dist->right = std::move(right);
    node        = std::move(dist);
  }
  return node;
}

std::unique_ptr<ast::operation::Assignment>
parser::Parser_Operator::assignment(std::unique_ptr<ast::AExpression> p_left)
{
  auto assign_tok = ctx.tok_v.expect_any(110, kAssignationTokens, "Expected assignation token.", "");

  auto assign             = ctx.Create_Node<ast::operation::Assignment>(assign_tok);
  assign->left            = std::move(p_left);
  assign->assignment_type = TokTy_to_ETransfertType(assign_tok.type);
  assign->right           = ctx.p_expr->parse_expression();
  return assign;
}

std::unique_ptr<ast::AExpression> parser::Parser_Operator::try_operation()
{
  return _1_memory_distance();
}

std::unique_ptr<ast::operation::Binary> parser::Parser_Operator::Create_BinOp(std::unique_ptr<ast::AExpression> p_left,
                                                                              EBinOpType                        p_op,
                                                                              std::unique_ptr<ast::AExpression> p_right)
{
  auto node   = ctx.Create_Node<ast::operation::Binary>(p_left->_token);
  node->left  = std::move(p_left);
  node->op_ty = p_op;
  node->right = std::move(p_right);
  return node;
}

std::unique_ptr<ast::operation::Unary> parser::Parser_Operator::Create_UnOp(EUnaryOpType                      p_op,
                                                                            std::unique_ptr<ast::AExpression> p_base)
{
  auto node      = ctx.Create_Node<ast::operation::Unary>(p_base->_token);
  node->unary_op = p_op;
  node->base     = std::move(p_base);
  return node;
}
