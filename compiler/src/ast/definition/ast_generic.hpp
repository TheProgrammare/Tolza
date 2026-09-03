#pragma once

#include "ast/data.hpp"
#include "ast/definition.hpp"

namespace ast
{

struct Generic_Type final {
  NODE_HEADER(Generic_Type);

  std::string source_typename;
  SET_VECTOR_TYPE(in_type)
};

struct Generic_Cast final {
  NODE_HEADER(Generic_Cast);

  std::string source_typename; // typename
  SET_TYPE(target);
  bool is_cast_from = false; // false = cast to | true = cast from
};

struct Generic_Op final {
  NODE_HEADER(Generic_Op);

  std::string target_gen_sym;        // typename
  EOp_Bin     op_ty = EOp_Bin::_add; // operator
  SET_TYPE(ret);
};

struct Generic_View final {
  NODE_HEADER(Generic_View);

  std::string target_gen_sym;
  SET_NODE(view);
};

struct Generic_Facet final {
  NODE_HEADER(Generic_Facet);

  std::string target_gen_sym;
  SET_NODE(facet);
};

struct Generic_Extension final {
  NODE_HEADER(Generic_Extension);

  std::string target_gen_sym;
  SET_NODE(extension);
};

struct Generic_Rule final {
  NODE_HEADER(Generic_Rule);

  std::string target_gen_sym;
  SET_NODE(rule);
};

} // namespace ast
  // AST