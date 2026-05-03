#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include <cstddef>
#include <string_view>
#include <vector>

namespace ast
{

AST_NODE(Global_Enum)
{
  struct Enum_Field final {
    std::string_view       name;
    // if empty : it's a simple enum key element
    std::vector<type::_id> types;
    size_t                 position = 0;
  };

  std::string_view        name;
  std::vector<Enum_Field> variants;
  bool                    is_global        = false;
  size_t                  discriminant_max = 0;
  SET_TYPE(discriminant_type);
};

AST_NODE(Global_Flag)
{
  std::string_view name;

  std::vector<std::string_view> fields;
};

AST_NODE(Global_Union)
{
  std::string_view name;

  struct Union_Field {
    std::string_view name;
    SET_TYPE(type);
  };

  std::vector<Union_Field> fields;
};

AST_NODE(Global_Module)
{
  std::string_view name;
  SET_NODE(codeblock);
};

AST_NODE(Global_Export)
{
  SET_NODE(codeblock);
};

AST_NODE(Global_Reexport)
{
  SET_NODE(regex);
  std::string_view alias;
};

AST_NODE(Global_Extern)
{
  std::string_view api;
  SET_NODE(codeblock);
};

AST_NODE(Global_Function)
{
  std::string_view name;
  SET_CALLABLE

  std::string_view extern_abi;
};

AST_NODE(Global_Alias_Module)
{
  SET_NODE(regex);
  std::string_view alias;
};

AST_NODE(Global_Alias_Type)
{
  std::string_view alias;
  SET_TYPE(type);
  ;
};

// gen name<T, U,...> { condition }
AST_NODE(Global_Generic)
{
  std::string_view              name;
  std::vector<std::string_view> typenames;
  SET_VECTOR_TYPE(gen_args)

  SET_VECTOR_NODE(gen_conds);
};

// let/var a: ptr'type#tableSize = expression;
AST_NODE(Global_Variable)
{
  std::string_view name;
  SET_TYPE(type);
  ETransfertType assignment = ETransfertType::MoveSemantic; // assign type
  SET_NODE(expression);                                     // affectation
  EVariableKind kind = EVariableKind::Const;
};

} // namespace ast
  // AST