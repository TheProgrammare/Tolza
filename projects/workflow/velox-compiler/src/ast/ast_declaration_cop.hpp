#pragma once

#include "nexus/ast/ast.hpp"
#include <string_view>

namespace ast
{

AST_NODE(COP_Component_Field)
{
  std::string_view name;
  SET_TYPE(type);
  ;
  SET_NODE(default_value);
  bool        is_no_default = false;
  ECapability capability    = ECapability::Copy;
};

AST_NODE(COP_Component)
{
  std::string_view name;

  SET_VECTOR_NODE(gen_params);
  SET_VECTOR_NODE(fields);
};

AST_NODE(COP_Role)
{
  std::string_view name;

  // components, expressions
  SET_VECTOR_TYPE(components)
};


AST_NODE(COP_Entity)
{
  std::string_view name;

  SET_VECTOR_NODE(components);

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

AST_NODE(COP_Entity_New)
{
  SET_CALLABLE;
};

AST_NODE(COP_Entity_Del)
{

  SET_CALLABLE;
};

AST_NODE(COP_Entity_Cast)
{
  SET_NODE(source);
  SET_TYPE(target);
  SET_NODE(codeblock);

  bool source_is_self = false;
};

AST_NODE(COP_Entity_Op)
{
  SET_NODE(codeblock);
  EBinOpType op_ty = EBinOpType::Add;
};

// the only non boolean operator and Iter operator who can return other type than the entity
AST_NODE(COP_Entity_Access_Op)
{
  std::string_view parameter_name;
  EAccessOpType    op_ty = EAccessOpType::Index;
  SET_NODE(codeblock);
  SET_TYPE(ret);
};

AST_NODE(COP_Entity_Transfert)
{
  ETransfertType transfet = ETransfertType::Copy;
};

AST_NODE(COP_System)
{
  std::string_view name;
  SET_VECTOR_NODE(parameters);
  SET_TYPE(prototype);
  SET_VECTOR_NODE(cases);
  bool is_const             = false;
  bool is_pure              = false;
  bool is_explicit_ret_type = false;
};

AST_NODE(COP_System_Case)
{
  bool is_default  = false;
  bool have_return = false;

  SET_VECTOR_NODE(bindings);
  SET_NODE(codeblock);
};

} // namespace ast
  // AST