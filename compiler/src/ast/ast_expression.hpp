#pragma once

#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

namespace ast
{

struct Expression_If_Ternary final {
  NODE_HEADER(Expression_If_Ternary);

  SET_NODE(left);
  SET_NODE(evaluator);
  SET_NODE(statement_true);
  SET_NODE(statement_false);
};


struct Expression_Member_Access final {
  NODE_HEADER(Expression_Member_Access);

  SET_NODE(left_expression);
  SET_NODE(right_identifier);
};

struct Expression_Self final {
  NODE_HEADER(Expression_Self);

  SET_NODE(target);
};

struct Expression_Other final {
  NODE_HEADER(Expression_Other);

  SET_NODE(target);
};

// (10, a, param3 = b, param5 = c)
struct Expression_Invocation_Arg final {
  NODE_HEADER(Expression_Invocation_Arg);

  [[maybe_unused]] std::string explicit_name;
  SET_NODE(expression);
};

struct Expression_Invocation final {
  NODE_HEADER(Expression_Invocation);

  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);

  EInvocationKind invocation_kind = EInvocationKind::NONE;
};

struct Expression_Invocation_Rule final {
  NODE_HEADER(Expression_Invocation_Rule);

  SET_NODE(target_form);

  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);
  SET_VECTOR_TYPE(arguments_types)
};

struct Expression_Invocation_Extend final {
  NODE_HEADER(Expression_Invocation_Extend);

  SET_NODE(target_form);

  SET_NODE(callee);
  SET_VECTOR_NODE(arguments);
  SET_VECTOR_TYPE(arguments_types)
};

// a[i] a?[i]
struct Expression_Table_Access final {
  NODE_HEADER(Expression_Table_Access);
  // most of time only one arg
  SET_NODE(target);
  SET_NODE(selector);

  bool bounded = false;
};

// val'my_ptr
struct Expression_Ptr_Val final {
  NODE_HEADER(Expression_Ptr_Val);

  SET_NODE(target);
};

// mut'my_val
struct Expression_Mut_Of final {
  NODE_HEADER(Expression_Mut_Of);

  SET_NODE(target);
};

// ref'my_val
struct Expression_Ref_Of final {
  NODE_HEADER(Expression_Ref_Of);

  SET_NODE(target);
};

// move'p
struct Expression_Move_Of final {
  NODE_HEADER(Expression_Move_Of);

  SET_NODE(target);
};

// copy'my_val
struct Expression_Copy_Of final {
  NODE_HEADER(Expression_Copy_Of);

  SET_NODE(target);
};

// addr'my_val
struct Expression_Addr_Of final {
  NODE_HEADER(Expression_Addr_Of);

  SET_NODE(target);
};

struct Expression_Size_Of final {
  NODE_HEADER(Expression_Size_Of);

  SET_NODE(target);
  size_t size = 0;
};

// target~[0..8] | target~[16..24]
struct Expression_GetBits final {
  NODE_HEADER(Expression_GetBits);

  SET_NODE(target);
  SET_NODE(range);
  // 8, 16, 32, 64, 128
  enum class EBitSize : uint8_t { _8, _16, _32, _64, _128 };
  EBitSize bit_size = EBitSize::_8;
};

// new ptr'T(val)
struct Expression_New_Ptr final {
  NODE_HEADER(Expression_New_Ptr);

  SET_TYPE(type);
  SET_NODE(expression);
  //::type::EPtrType pointer = ::type::EPtrType::raw_ptr;
};

struct Expression_Get_Type final {
  NODE_HEADER(Expression_Get_Type);

  SET_NODE(target);
};

} // namespace ast
  // AST