#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/forward.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace ast
{

AST_NODE(Enum_Field)
{
  std::string name;
  type::ID    type;
  size_t      position = 0;
};

AST_NODE(Global_Enum)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(variants);

  size_t discriminant_max = 0;
  SET_TYPE(discriminant_type);
};

AST_NODE(Flag_Field)
{
  std::string name;
};

AST_NODE(Global_Flag)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(flags);
  SET_TYPE(underlying_type);
};

AST_NODE(Union_Field)
{
  std::string name;

  SET_TYPE(type);
};

AST_NODE(Global_Union)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(variants);
};

AST_NODE(Global_Module)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);
};

AST_NODE(Global_Export)
{
  SET_NODE(codeblock);
};

AST_NODE(Global_Reexport)
{
  SET_NODE(regex);
  std::string alias;
};

AST_NODE(Global_Extern)
{
  std::string abi;
  SET_NODE(codeblock);
};

AST_NODE(Global_Function)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_CALLABLE

  std::string extern_abi;
};


AST_NODE(Global_Alias_Module)
{
  SET_NODE(regex);
  EVisibility visibility = EVisibility::File_Scope;

  std::string alias;
};

AST_NODE(Global_Alias_Type)
{
  std::string alias;
  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(type);
};

// gen name<T, U,...> { condition }
AST_NODE(Global_Generic)
{
  std::string              name;
  std::vector<std::string> typenames;
  EVisibility              visibility = EVisibility::File_Scope;

  SET_VECTOR_TYPE(gen_args)

  SET_VECTOR_NODE(gen_conds);
};

// let/var a: ptr'type#tableSize = expression;
AST_NODE(Global_Variable)
{
  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(type);
  ETransfertType assignment = ETransfertType::MoveSemantic; // assign type
  SET_NODE(expression);                                     // affectation
  EVariableKind kind = EVariableKind::Const;
};

} // namespace ast
  // AST