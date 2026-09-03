#pragma once

#include "ast/data.hpp"
#include "ast/definition.hpp"
#include "nexus/forward.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace ast
{

struct Enum_Field final {
  NODE_HEADER(Enum_Field);

  std::string name;
  type::ID    type;
  size_t      position = 0;
};

struct Global_Enum final {
  NODE_HEADER(Global_Enum);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(variants);

  size_t discriminant_max = 0;
  SET_TYPE(discriminant_type);
};

struct Flag_Field final {
  NODE_HEADER(Flag_Field);

  std::string name;
};

struct Global_Flag final {
  NODE_HEADER(Global_Flag);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(flags);
  SET_TYPE(underlying_type);
};

struct Union_Field final {
  NODE_HEADER(Union_Field);

  std::string name;

  SET_TYPE(type);
};

struct Global_Union final {
  NODE_HEADER(Global_Union);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(variants);
};

struct Global_Module final {
  NODE_HEADER(Global_Module);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_NODE(codeblock);
};

struct Global_Export final {
  NODE_HEADER(Global_Export);

  SET_NODE(codeblock);
};

struct Global_Reexport final {
  NODE_HEADER(Global_Reexport);

  SET_NODE(regex);
  std::string alias;
};

struct Global_Extern final {
  NODE_HEADER(Global_Extern);

  std::string abi;
  SET_NODE(codeblock);
};

struct Call_Contract final {
  NODE_HEADER(Call_Contract);

  SET_NODE(pre);
  ECallContract pre_mode = ECallContract::Static;
  SET_NODE(pre_err);
  SET_NODE(post);
  ECallContract post_mode = ECallContract::Static;
  SET_NODE(post_err);
};


struct Global_Function final {
  NODE_HEADER(Global_Function);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_CALLABLE

  std::string extern_abi;
};


struct Global_Alias_Module final {
  NODE_HEADER(Global_Alias_Module);

  SET_NODE(regex);
  EVisibility visibility = EVisibility::File_Scope;

  std::string alias;
};

struct Global_Alias_Type final {
  NODE_HEADER(Global_Alias_Type);

  std::string alias;
  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(type);
};

// gen name<T, U,...> { condition }
struct Global_Generic final {
  NODE_HEADER(Global_Generic);

  std::string              name;
  std::vector<std::string> typenames;
  EVisibility              visibility = EVisibility::File_Scope;

  SET_VECTOR_TYPE(gen_args)

  SET_VECTOR_NODE(gen_conds);
};

// let/var a: ptr'type#tableSize = expression;
struct Global_Variable final {
  NODE_HEADER(Global_Variable);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_TYPE(type);
  ETransfertType assignment = ETransfertType::move; // assign type
  SET_NODE(expression);                             // affectation
  EVariableKind kind      = EVariableKind::_const;
  bool          is_uninit = false;

  std::string extern_abi;
};

} // namespace ast
  // AST