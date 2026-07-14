#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/forward.hpp"

#include <string>

namespace ast
{

AST_NODE(Global_Extend_Fn)
{
  SET_TYPE(extended_type);
  SET_NODE(self);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;


  // mut self = false, ref self = true
  bool is_self_const = false;
  // no self
  bool is_static     = false;

  SET_CALLABLE

  std::string extern_abi;
};


AST_NODE(Global_Extend_Cast)
{
  SET_TYPE(extended_type);
  SET_NODE(self);

  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(as_type);

  SET_NODE(codeblock);
};

AST_NODE(Global_Extend_Op_Bin)
{
  SET_TYPE(extended_type);
  SET_NODE(self);
  SET_NODE(other);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EOp_Bin bin_op = EOp_Bin::NONE;
};

AST_NODE(Global_Extend_Op_Un)
{
  SET_TYPE(extended_type);
  SET_NODE(self);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EOp_Unary unary_op = EOp_Unary::NONE;
};

AST_NODE(Global_Extend_Op_Subscript)
{
  SET_TYPE(extended_type);
  SET_NODE(self);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EOp_Subscript subscript_op = EOp_Subscript::NONE;

  // index exclusive
  SET_NODE(index);
  // range exclusive
  SET_NODE(range_start);
  SET_NODE(range_end);

  SET_TYPE(ret)
  bool is_explicit_ret = false;
};

AST_NODE(Global_Extend_Op_Transfert)
{
  SET_TYPE(extended_type);
  SET_NODE(self);
  SET_NODE(other);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  ETransfertType transfert_op = ETransfertType::NONE;
};

AST_NODE(Global_Extend_Op_Other)
{
  SET_TYPE(extended_type);
  SET_NODE(self);
  SET_NODE(other);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EOp_Other other_op = EOp_Other::NONE;

  SET_TYPE(ret)
  bool is_explicit_ret = false;
};

} // namespace ast
