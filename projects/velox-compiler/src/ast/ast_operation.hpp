#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"


namespace ast
{

AST_NODE(Operation_Cast_As)
{
  SET_NODE(expression);
  SET_TYPE(type);

  enum class ECastType : uint8_t { AS, AS_REINTERPRET, AS_SAFE };
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

// a copy= b | a move= b | a += -= *= **= ...
AST_NODE(Operation_Transfert)
{
  SET_NODE(left);
  SET_NODE(right);
  ETransfertType assignment_type = ETransfertType::copy;
  EOp_Bin        assignment_op   = EOp_Bin::NONE;
};

// a op b
AST_NODE(Operation_Binary)
{
  SET_NODE(left);
  SET_NODE(right);
  EOp_Bin op_ty = EOp_Bin::_add;
};

// !a
AST_NODE(Operation_Unary)
{
  SET_NODE(base);
  EOp_Unary unary_op = EOp_Unary::_not;
};

// a </<= b >/>= c
AST_NODE(Operation_Interval)
{
  SET_NODE(left);
  SET_NODE(center);
  SET_NODE(right);
  EOp_Bin left_comparator  = EOp_Bin::_low;
  EOp_Bin right_comparator = EOp_Bin::_low;
};

} // namespace ast
  // AST