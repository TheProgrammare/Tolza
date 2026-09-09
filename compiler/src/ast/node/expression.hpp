#pragma once

#include "ast/data.hpp"
#include "ast/node/base.hpp"
#include "id/typeid.hpp"
#include "nexus/forward.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

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

  enum class EBitSize : uint8_t { NONE, _8, _16, _32, _64, _128 };
  EBitSize bit_size = EBitSize::NONE;
};

// new ptr'T(val)
struct Expression_New_Ptr final {
  NODE_HEADER(Expression_New_Ptr);

  SET_TYPE(type);
  SET_NODE(expression);
  //::type::EPtrType pointer = ::type::EPtrType::raw_ptr;
};

} // namespace ast
  // AST