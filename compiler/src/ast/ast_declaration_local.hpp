#pragma once

#include <string>
#include <vector>

#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

namespace ast
{

// inside of Enum/Tuple pattern
// e.g. Some(a) = value
// e.g. Player { CId.name: name, CId.age: age } = value
// e.g. Player { CId { name: name, age: age } } = value
// e.g. (a, b, c) = triple
// e.g. match val { Some(a) => ... }
// e.g. rule name() { Facet(c) => ... }
struct Local_Binding final {
  NODE_HEADER(Local_Binding);

  std::string name;
  SET_TYPE(type);
  SET_NODE(expression);
  ECapability capability = ECapability::NONE;

  SET_INFERRED_TYPE
};

struct Local_Pattern_Element final {
  NODE_HEADER(Local_Pattern_Element);

  enum class Kind : uint8_t { Ignore, Binding, Literal };

  SET_NODE(bind);
  SET_NODE(literal);

  Kind kind = Kind::Ignore;
};

// e.g. [if/elif/while] let Some(a) = value {...}
struct Local_Pattern_Enum final {
  NODE_HEADER(Local_Pattern_Enum);

  ECapability capability = ECapability::ref;

  SET_NODE(name);
  // mapping
  SET_VECTOR_NODE(pattern_elements);
  SET_NODE(expression);
};

// e.g. [if/while/for] let (a, b, 10) in triple_collection {...}
struct Local_Pattern_Tuple final {
  NODE_HEADER(Local_Pattern_Tuple);

  ECapability capability = ECapability::ref;

  // mapping
  SET_VECTOR_NODE(pattern_elements);
  SET_NODE(expression);
};

// e.g. [if/while] let Player{ CId{ .name= name, .nodeid= 10 } }
struct Local_Pattern_Form final {
  NODE_HEADER(Local_Pattern_Form);

  ECapability capability = ECapability::ref;

  SET_NODE(name);
  // mapping
  SET_VECTOR_NODE(pattern_facets);
  SET_NODE(expression);
};

struct Local_Pattern_Rule_Facet final {
  NODE_HEADER(Local_Pattern_Rule_Facet);

  ECapability capability = ECapability::ref;

  SET_NODE(name);
  SET_NODE(bind);
  SET_NODE(expression);
};

// e.g. [if/while] let CId{ .name: ref'name, .id: 10 }
struct Local_Pattern_Facet final {
  NODE_HEADER(Local_Pattern_Facet);

  struct Field final {
    std::string name;
    ast::ID     mapping;
  };

  ECapability capability = ECapability::ref;

  SET_NODE(name);
  // mapping
  std::vector<Field> mapping;
  SET_NODE(expression);
};

// var (a, b, _, d) = call(); var (a, _, c, d) = tupleVariable;
struct Local_Tuple_Destructuring final {
  NODE_HEADER(Local_Tuple_Destructuring);

  SET_VECTOR_NODE(bindings);

  SET_NODE(expression);
  EVariableKind kind = EVariableKind::_let;
};

struct Local_Lambda final {
  NODE_HEADER(Local_Lambda);

  std::string name;
  SET_NODE(capture);
  SET_CALLABLE
};

// let/var a: ptr'type?$ = expression;
struct Local_Variable final {
  NODE_HEADER(Local_Variable);

  std::string name;

  SET_TYPE(type);
  SET_NODE(expression);

  ETransfertType assignment = ETransfertType::move; // assign type
  EVariableKind  kind       = EVariableKind::_const;
  bool           is_static  = false;
  bool           is_uninit  = false;
};

// ref/mut name = expression
struct Local_Capability final {
  NODE_HEADER(Local_Capability);

  std::string name;

  SET_TYPE(type);
  SET_NODE(expression);

  ECapability kind = ECapability::NONE;
};

struct Local_Lambda_Capture final {
  NODE_HEADER(Local_Lambda_Capture);

  SET_VECTOR_NODE(capture_members);

  bool is_all_ref      = false;
  bool is_capture_self = false;
};

// (a: str, copy b: i32 = 10, mut c: f32 = nullptr, args: ...)
struct Local_Parameter final {
  NODE_HEADER(Local_Parameter);

  std::string name;
  SET_NODE(parent_callable);
  size_t position = 0;
  SET_TYPE(type);
  SET_NODE(default_value);
  EPassMode passmode    = EPassMode::copy;
  bool      is_restrict = false;
};

struct Local_Gen_Param_Elem final {
  NODE_HEADER(Local_Gen_Param_Elem);

  std::string name;

  SET_VECTOR_TYPE(generic_references)
};

struct Local_Gen_Params final {
  NODE_HEADER(Local_Gen_Params);

  SET_VECTOR_NODE(gen_parameters);
};

} // namespace ast