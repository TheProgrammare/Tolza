#pragma once

#include "nexus/ast/ast.hpp"


namespace ast
{

AST_NODE(Operation_Cast_As)
{
  SET_NODE(expression);
  SET_TYPE(type);

  enum class ECastType { AS, AS_REINTERPRET, AS_SAFE };
  ECastType cast_type = ECastType::AS;
};

AST_NODE(Operation_Is)
{
  SET_NODE(left);
  SET_NODE(right);
};

AST_NODE(Operation_In)
{
  SET_NODE(right);
};

// a copy= b | a move= b | a ref= b | a mut= b
AST_NODE(Operation_Assignment)
{
  SET_NODE(left);
  SET_NODE(right);
  ETransfertType assignment_type = ETransfertType::Copy;
};

// a op b
AST_NODE(Operation_Binary)
{
  SET_NODE(left);
  SET_NODE(right);
  EBinOpType op_ty = EBinOpType::Add;
};

// !a
AST_NODE(Operation_Unary)
{
  SET_NODE(base);
  EUnaryOpType unary_op = EUnaryOpType::_not;
};

// a </<= b >/>= c
AST_NODE(Operation_Interval)
{
  SET_NODE(left);
  SET_NODE(center);
  SET_NODE(right);
  EBinOpType left_comparator  = EBinOpType::Low;
  EBinOpType right_comparator = EBinOpType::Low;
};

} // namespace ast
  // AST