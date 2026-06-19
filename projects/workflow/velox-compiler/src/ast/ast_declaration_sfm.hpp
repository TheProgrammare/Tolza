#pragma once

#include "nexus/ast/ast.hpp"
#include <string>

namespace ast
{

AST_NODE(SFM_Facet_Field)
{
  std::string name;

  SET_TYPE(type);
  SET_NODE(default_value);
  bool        is_no_default = false;
  ECapability capability    = ECapability::Copy;
};

AST_NODE(SFM_Facet)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(gen_params);
  SET_VECTOR_NODE(fields);
};

AST_NODE(SFM_View)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  // facets, expressions
  SET_VECTOR_TYPE(facets)
};


AST_NODE(SFM_Form)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(facets);

  SET_VECTOR_NODE(constructors);
  SET_NODE(destructor);

  SET_VECTOR_NODE(gen_params);

  SET_VECTOR_NODE(operators);
  SET_VECTOR_NODE(accessors);
  SET_VECTOR_NODE(casters);

  bool isDestructible = true;
  bool isMoveable     = true;
  bool isCastable     = true;
  bool isExtCastable  = true;
};

AST_NODE(SFM_Rule)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(parameters);
  SET_TYPE(prototype);
  SET_VECTOR_NODE(cases);
  bool is_const             = false;
  bool is_pure              = false;
  bool is_explicit_ret_type = false;
};

AST_NODE(SFM_Rule_Case)
{
  bool is_default  = false;
  bool have_return = false;

  SET_VECTOR_NODE(bindings);
  SET_NODE(codeblock);
};

} // namespace ast
  // AST