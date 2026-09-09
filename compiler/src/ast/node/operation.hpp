#pragma once

#include "ast/data.hpp"
#include "ast/node/base.hpp"
#include "id/typeid.hpp"

#include <cstdint>


namespace ast
{

struct Operation_Cast_As final {
  NODE_HEADER(Operation_Cast_As);

  SET_NODE(expression);
  SET_TYPE(type);

  enum class ECastType : uint8_t { AS, AS_REINTERPRET, AS_SAFE };
  ECastType cast_type = ECastType::AS;
};

struct Operation_Is final {
  NODE_HEADER(Operation_Is);

  SET_NODE(left);
  SET_NODE(right);
};

struct Operation_In final {
  NODE_HEADER(Operation_In);

  SET_NODE(left);
  SET_NODE(right);
};

// a copy= b | a move= b | a += -= *= **= ...
struct Operation_Transfert final {
  NODE_HEADER(Operation_Transfert);

  SET_NODE(left);
  SET_NODE(right);
  ETransfertType assignment_type = ETransfertType::copy;
  EOp_Bin        assignment_op   = EOp_Bin::NONE;
};

// a op b
struct Operation_Binary final {
  NODE_HEADER(Operation_Binary);

  SET_NODE(left);
  SET_NODE(right);
  EOp_Bin op_ty = EOp_Bin::_add;
};

// !a
struct Operation_Unary final {
  NODE_HEADER(Operation_Unary);

  SET_NODE(base);
  EOp_Unary unary_op = EOp_Unary::_not;
};

// a </<= b >/>= c
struct Operation_Interval final {
  NODE_HEADER(Operation_Interval);

  SET_NODE(left);
  SET_NODE(center);
  SET_NODE(right);
  EOp_Bin left_comparator  = EOp_Bin::_low;
  EOp_Bin right_comparator = EOp_Bin::_low;
};

struct Operation_Mem final {
  NODE_HEADER(Operation_Mem);

  SET_NODE(target);

  EOp_Mem op = EOp_Mem::NONE;
};

} // namespace ast
  // AST