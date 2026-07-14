#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"

namespace ast
{

AST_NODE(Expression_If_Ternary)
{
  SET_NODE(left);
  SET_NODE(evaluator);
  SET_NODE(statement_true);
  SET_NODE(statement_false);
};


AST_NODE(Expression_Member_Access)
{
  SET_NODE(left_expression);
  SET_NODE(right_identifier);
};

AST_NODE(Expression_Self)
{
  SET_NODE(target);
};

AST_NODE(Expression_Other)
{
  SET_NODE(target);
};

// (10, a, param3 = b, param5 = c)
AST_NODE(Expression_Invocation_Arg)
{
  [[maybe_unused]] std::string explicit_name;
  SET_NODE(expression);
};

AST_NODE(Expression_Invocation)
{
  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);

  EInvocationKind invocation_kind = EInvocationKind::NONE;
};

AST_NODE(Expression_Invocation_Rule)
{
  SET_NODE(target_form);

  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);
  SET_VECTOR_TYPE(arguments_types)
};

AST_NODE(Expression_Invocation_Extend)
{
  SET_NODE(target_form);

  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);
  SET_VECTOR_TYPE(arguments_types)
};

// a[i] a?[i]
AST_NODE(Expression_Table_Access)
{ // most of time only one arg
  SET_NODE(target);
  SET_NODE(selector);

  bool bounded = false;
};

// val'my_ptr
AST_NODE(Expression_Ptr_Val)
{
  SET_NODE(target);
};

// mut'my_val
AST_NODE(Expression_Mut_Of)
{
  SET_NODE(target);
};

// ref'my_val
AST_NODE(Expression_Ref_Of)
{
  SET_NODE(target);
};

// move'p
AST_NODE(Expression_Move_Of)
{
  SET_NODE(target);
};

// copy'my_val
AST_NODE(Expression_Copy_Of)
{
  SET_NODE(target);
};

// addr'my_val
AST_NODE(Expression_Addr_Of)
{
  SET_NODE(target);
};

AST_NODE(Expression_Size_Of)
{
  SET_NODE(target);
  size_t size = 0;
};

// target~[0..8] | target~[16..24]
AST_NODE(Expression_GetBits)
{
  SET_NODE(target);
  SET_NODE(range);
  // 8, 16, 32, 64, 128
  enum class EBitSize : uint8_t { _8, _16, _32, _64, _128 };
  EBitSize bit_size = EBitSize::_8;
};

// new ptr'T(val)
AST_NODE(Expression_New_Ptr)
{
  SET_TYPE(type);
  SET_NODE(expression);
  //::type::EPtrType pointer = ::type::EPtrType::raw_ptr;
};

AST_NODE(Expression_Get_Type)
{
  SET_NODE(target);
};

} // namespace ast
  // AST