#pragma once

#include "nexus/ast/ast.hpp"

namespace ast
{

AST_NODE(Generic_Type)
{
  std::string source_typename;
  SET_VECTOR_TYPE(in_type)
};

AST_NODE(Generic_Cast)
{
  std::string source_typename; // typename
  SET_TYPE(target);
  bool is_cast_from = false; // false = cast to | true = cast from
};

AST_NODE(Generic_Op)
{
  std::string target_gen_sym;        // typename
  EOp_Bin     op_ty = EOp_Bin::_add; // operator
  SET_TYPE(ret);
};

AST_NODE(Generic_View)
{
  std::string target_gen_sym;
  SET_NODE(view);
};

AST_NODE(Generic_Facet)
{
  std::string target_gen_sym;
  SET_NODE(facet);
};

AST_NODE(Generic_Extension)
{
  std::string target_gen_sym;
  SET_NODE(extension);
};

AST_NODE(Generic_Rule)
{
  std::string target_gen_sym;
  SET_NODE(rule);
};

} // namespace ast
  // AST