#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"

#include <string>

namespace ast
{

AST_NODE(Global_Extend_Fn)
{
  SET_TYPE(target_type);

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
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(as_type);

  SET_NODE(codeblock);
};

AST_NODE(Global_Extend_Op_Bin)
{
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EBinOpType bin_op = EBinOpType::NONE;
};

AST_NODE(Global_Extend_Op_Un)
{
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EUnaryOpType unary_op = EUnaryOpType::NONE;
};

AST_NODE(Global_Extend_Op_Access)
{
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  EAccessOpType access_op = EAccessOpType::NONE;

  // index exclusive
  SET_NODE(index);
  // range exclusive
  SET_NODE(range_start);
  SET_NODE(range_end);

  SET_TYPE(ret)
};

AST_NODE(Global_Extend_Op_Transfert)
{
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  ETransfertType transfert_op = ETransfertType::NONE;
};

AST_NODE(Global_Extend_Op_Other)
{
  SET_TYPE(target_type);

  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);

  bool is_predicat_op = false;
  bool is_del_op      = false;
};

} // namespace ast
