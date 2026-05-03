
#include "parser_operation.hpp"

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/script.hpp"
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

ast::_gnid parser::Parser_Operator::_15_power()
{
  auto left = p.p_expr->parse_expression_term();
  if (p.match(token::ETokenKind::OP_POWER)) {
    auto right = _15_power();
    return Create_BinOp(std::move(left), ast::EBinOpType::Pow, std::move(right));
  }
  return left;
}

ast::_gnid parser::Parser_Operator::_14_scalar()
{
  auto node = _15_power();
  while (p.match_any({token::ETokenKind::OP_ASTERISK, token::ETokenKind::OP_DIVIDE, token::ETokenKind::OP_MODULO,
                      token::ETokenKind::OP_QUOTIEN, token::ETokenKind::OP_REMAIN})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _15_power();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_13_cumulate()
{
  auto node = _14_scalar();
  while (p.match_any({token::ETokenKind::OP_PLUS, token::ETokenKind::OP_MINUS})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _14_scalar();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_12_shift()
{
  auto node = _13_cumulate();
  while (p.match_any(token::k_op_bitwise_shift)) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _13_cumulate();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_11_comparison()
{
  auto node = _12_shift();
  while (p.match_any({token::ETokenKind::OPEN_BRACKETS, token::ETokenKind::CLOSE_BRACKETS, token::ETokenKind::OP_LEQ,
                      token::ETokenKind::OP_GEQ})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _12_shift();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_10_equality()
{
  auto node = _11_comparison();
  while (p.match_any({token::ETokenKind::OP_EQ, token::ETokenKind::OP_NEQ, token::ETokenKind::OP_EQS,
                      token::ETokenKind::OP_NEQS, token::ETokenKind::IN, token::ETokenKind::IS})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _11_comparison();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_9_bitwise_not()
{
  if (p.match(token::ETokenKind::OP_B_NOT)) {
    auto op      = ast::ETokenKind_to_EUnaryOpType(p.peek(-1).kind);
    auto operand = _9_bitwise_not();
    return Create_UnOp(op, std::move(operand));
  }
  return _10_equality();
}

ast::_gnid parser::Parser_Operator::_8_bitwise_and_nand()
{
  auto node = _9_bitwise_not();
  while (p.match_any({token::ETokenKind::OP_B_AND, token::ETokenKind::OP_B_NAND})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _9_bitwise_not();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_7_bitwise_xor_xnor()
{
  auto node = _8_bitwise_and_nand();
  while (p.match_any({token::ETokenKind::OP_B_XOR, token::ETokenKind::OP_B_XNOR})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _8_bitwise_and_nand();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_6_bitwise_or_nor()
{
  auto node = _7_bitwise_xor_xnor();
  while (p.match_any({token::ETokenKind::OP_B_OR, token::ETokenKind::OP_B_NOR})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _7_bitwise_xor_xnor();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_5_logical_not()
{
  if (p.match(token::ETokenKind::OP_NOT)) {
    auto op      = ast::ETokenKind_to_EUnaryOpType(p.peek(-1).kind);
    auto operand = _5_logical_not();
    return Create_UnOp(op, std::move(operand));
  }
  return _6_bitwise_or_nor();
}

ast::_gnid parser::Parser_Operator::_4_logical_and_nand()
{
  auto node = _5_logical_not();
  while (p.match_any({token::ETokenKind::OP_AND, token::ETokenKind::OP_NAND})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _5_logical_not();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_3_logicial_xor_xnor()
{
  auto node = _4_logical_and_nand();
  while (p.match_any({token::ETokenKind::OP_XOR, token::ETokenKind::OP_XNOR})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.peek(-1).kind);
    auto right = _4_logical_and_nand();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_2_logicial_or_nor()
{
  auto node = _3_logicial_xor_xnor();
  while (p.check_any({token::ETokenKind::OP_OR, token::ETokenKind::OP_NOR})) {
    auto op    = ast::ETokenKind_to_EBinOpType(p.next().kind);
    auto right = _3_logicial_xor_xnor();
    node       = Create_BinOp(std::move(node), op, std::move(right));
  }
  return node;
}

ast::_gnid parser::Parser_Operator::_1_memory()
{
  auto node = _2_logicial_or_nor();
  while (p.check_any({token::ETokenKind::OP_MEM_DIST, token::ETokenKind::OP_MEM_ADD, token::ETokenKind::OP_MEM_SUB})) {
    parser_add_node(dist, Operation_Binary, p.peek().id);
    auto right  = _2_logicial_or_nor();
    dist->left  = node;
    dist->right = right;
    node        = dist->node_id;
  }
  return node;
}

ast::_gnid parser::Parser_Operator::assignment(ast::_gnid p_left)
{
  auto assign_tok = p.expect_any(110, token::kAssignationTokens, "Expected assignation token.", "");

  parser_add_node(assign, Operation_Assignment, p.peek().id);
  assign->left            = p_left;
  assign->assignment_type = ast::ETokenKind_to_ETransfertType(assign_tok.kind);
  assign->right           = p.p_expr->parse_expression();
  return assign->node_id;
}

ast::_gnid parser::Parser_Operator::try_operation()
{
  return _1_memory();
}

ast::_gnid parser::Parser_Operator::Create_BinOp(ast::_gnid p_left, ast::EBinOpType p_op, ast::_gnid p_right)
{
  parser_add_node(node, Operation_Binary, p.peek().id);
  node->left  = p_left;
  node->op_ty = p_op;
  node->right = p_right;
  return node->node_id;
}

ast::_gnid parser::Parser_Operator::Create_UnOp(ast::EUnaryOpType p_op, ast::_gnid p_base)
{
  parser_add_node(node, Operation_Unary, p.peek().id);
  node->unary_op = p_op;
  node->base     = p_base;
  return node->node_id;
}
