#pragma once

#include <string>
#include <vector>

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"

namespace ast
{

AST_NODE(Local_CodeBlock)
{
  SET_VECTOR_NODE(elements);
};

// inside of Enum/Tuple pattern
// e.g. Some(a) = value
// e.g. Player { CId.name: name, CId.age: age } = value
// e.g. Player { CId { name: name, age: age } } = value
// e.g. (a, b, c) = triple
// e.g. match val { Some(a) => ... }
// e.g. sys name() { Component(c) => ... }
AST_NODE(Local_Binding)
{
  std::string_view name;
  SET_TYPE(type);
  SET_NODE(expression);
  ECapability capability = ECapability::NONE;

  SET_INFERRED_TYPE
};

AST_NODE(Local_Pattern_Element)
{
  enum class Kind { Ignore, Binding, Literal };

  SET_NODE(bind);
  SET_NODE(literal);

  Kind kind = Kind::Ignore;
};

// e.g. [if/elif/while] let Some(a) = value {...}
AST_NODE(Local_Pattern_Enum)
{
  ECapability capability = ECapability::Ref;

  SET_NODE(name);
  // mapping
  SET_VECTOR_NODE(pattern_elements);
  SET_NODE(expression);
};

// e.g. [if/while/for] let (a, b, 10) in triple_collection {...}
AST_NODE(Local_Pattern_Tuple)
{
  ECapability capability = ECapability::Ref;

  // mapping
  SET_VECTOR_NODE(pattern_elements);
  SET_NODE(expression);
};

// e.g. [if/while] let Player{ CId.name: name, CId.id: 10 }
// e.g. [if/while] let Player{ CId{ name: name, id: 10 } }
AST_NODE(Local_Pattern_Entity)
{
  ECapability capability = ECapability::Ref;

  SET_NODE(name);
  // mapping
  SET_VECTOR_NODE(pattern_components);
  SET_NODE(expression);
};

AST_NODE(Local_Pattern_Sys_Comp)
{
  ECapability capability = ECapability::Ref;

  SET_NODE(name);
  SET_NODE(bind);
  SET_NODE(expression);
};

// e.g. [if/while] let CId{ name: ref'name, id: 10 }
AST_NODE(Local_Pattern_Comp)
{
  struct Field final {
    std::string_view name;
    ast::_gnid       mapping;
  };

  ECapability capability = ECapability::Ref;

  SET_NODE(name);
  // mapping
  std::vector<Field> mapping;
  SET_NODE(expression);
};

// var (a, b, _, d) = call(); var (a, _, c, d) = tupleVariable;
AST_NODE(Local_Tuple_Destructuring)
{
  SET_VECTOR_NODE(bindings);

  SET_NODE(expression);
};

AST_NODE(Local_Lambda)
{
  std::string_view name;
  SET_NODE(capture);
  SET_CALLABLE
};

// let/var a: ptr'type?$ = expression;
AST_NODE(Local_Variable)
{
  std::string_view name;

  SET_TYPE(type);
  SET_NODE(expression);

  ETransfertType assignment = ETransfertType::Copy; // assign type
  EVariableKind  kind       = EVariableKind::Const;
  bool           isStatic   = false;
};

// ref/mut name = expression
AST_NODE(Local_Capability)
{
  std::string_view name;

  SET_NODE(expression);

  ECapability kind = ECapability::NONE;
};

AST_NODE(Local_Lambda_Capture)
{
  SET_VECTOR_NODE(capture_members);

  bool is_all_ref      = false;
  bool is_capture_self = false;
};

// (a: str, copy b: i32 = 10, mut c: f32 = nullptr, args: ...)
AST_NODE(Local_Parameter)
{
  std::string_view name;
  SET_TYPE(type);
  SET_NODE(default_value);
  EPassMode passmode    = EPassMode::Copy;
  bool      is_variadic = false;
};

AST_NODE(Local_Gen_Param_Elem)
{
  std::string_view name;

  SET_VECTOR_TYPE(generic_references)
};

AST_NODE(Local_Gen_Params)
{
  SET_VECTOR_NODE(gen_parameters);
};

} // namespace ast
  // AST