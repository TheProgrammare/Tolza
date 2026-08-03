#pragma once

#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include <string>

namespace ast
{

struct SFM_Facet_Field final {
  NODE_HEADER(SFM_Facet_Field);

  std::string name;

  SET_TYPE(type);
  SET_NODE(default_value);
  bool        is_no_default = false;
  ECapability capability    = ECapability::copy;
};

struct SFM_Facet final {
  NODE_HEADER(SFM_Facet);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(gen_params);
  SET_VECTOR_NODE(fields);
};

struct SFM_View final {
  NODE_HEADER(SFM_View);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  // facets, expressions
  SET_VECTOR_TYPE(facets)
};


struct SFM_Form final {
  NODE_HEADER(SFM_Form);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  // literal facet (Literal_Record)
  // or identifier
  // or identifier typed
  SET_VECTOR_NODE(facets);

  SET_VECTOR_NODE(gen_params);

  bool isDestructible = true;
  bool isMoveable     = true;
  bool isCastable     = true;
  bool isExtCastable  = true;
};

struct SFM_Rule final {
  NODE_HEADER(SFM_Rule);

  std::string name;
  EVisibility visibility = EVisibility::File_Scope;

  SET_VECTOR_NODE(parameters);
  SET_TYPE(prototype);
  SET_VECTOR_NODE(cases);
  bool is_const        = false;
  bool is_pure         = false;
  bool is_explicit_ret = false;
};

struct SFM_Rule_Case final {
  NODE_HEADER(SFM_Rule_Case);

  bool is_default  = false;
  bool have_return = false;

  SET_VECTOR_NODE(bindings);
  SET_NODE(codeblock);
};

} // namespace ast
  // AST