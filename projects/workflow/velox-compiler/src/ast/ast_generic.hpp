#pragma once

#include "nexus/ast/ast.hpp"

namespace ast
{

AST_NODE(Generic_Is_Type)
{
  std::string_view source_typename;
  SET_VECTOR_TYPE(in_type)
};

AST_NODE(Generic_Can_Cast)
{
  std::string_view source_typename; // typename
  SET_TYPE(target);
  bool is_cast_from = false; // false = cast to | true = cast from
};

AST_NODE(Generic_Have_Op)
{
  std::string_view target_gen_sym;          // typename
  EBinOpType       op_ty = EBinOpType::Add; // operator
  SET_TYPE(ret);
};

AST_NODE(Generic_Have_Role)
{
  std::string_view target_gen_sym;
  SET_NODE(role);
};

AST_NODE(Generic_Use_Component)
{
  std::string_view target_gen_sym;
  SET_NODE(component);
};

AST_NODE(Generic_Compatible_System)
{
  std::string_view target_gen_sym;
  SET_NODE(system);
};

} // namespace ast
  // AST