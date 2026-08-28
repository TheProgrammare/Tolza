#pragma once

#include "nexus/ast/data.hpp"
#include "nexus/ids.hpp"

#include <vector>


namespace ast
{

#define NODE_HEADER(name)                                                                                              \
  NodeHeader                 header      = NodeHeader(ENodeKind::name);                                                \
  static constexpr ENodeKind static_kind = ENodeKind::name;                                                            \
  [[nodiscard]] ID           nodeid() const noexcept                                                                   \
  {                                                                                                                    \
    return header.nodeid;                                                                                              \
  }


#define SET_CALLABLE                                                                                                   \
  SET_VECTOR_NODE(parameters);                                                                                         \
  SET_TYPE(prototype);                                                                                                 \
  SET_NODE(codeblock);                                                                                                 \
                                                                                                                       \
  common::env::ECallConvention call_convention;                                                                        \
                                                                                                                       \
  bool is_pure         = false;                                                                                        \
  bool is_explicit_ret = false;

#define SET_NODE(name)   ast::ID name;
#define SET_SYMBOL(name) definition::ID name;
#define SET_TYPE(name)   type::ID name;

#define SET_INFERRED_TYPE        type::ID inferred_type;
#define SET_INFERRED_TYPE_(name) type::ID name;

#define SET_VECTOR_NODE(name)   std::vector<ast::ID> name;
#define SET_VECTOR_SYMBOL(name) std::vector<definition::ID> name;
#define SET_VECTOR_TYPE(name)   std::vector<type::ID> name;

using Path = std::vector<std::string>;


struct NodeHeader final {
  ID        nodeid;
  scope::ID scpid;
  token::ID start_tokid;
  token::ID end_tokid;
  ENodeKind kind = ENodeKind::NONE;

  explicit NodeHeader(ENodeKind k)
    : kind(k)
  {
  }
};


struct Unknown final {
  NODE_HEADER(NONE);

  Unknown()
  {
    assert(false);
  }
};

struct Root final {
  NODE_HEADER(Root);

  SET_VECTOR_NODE(global_nodes);
};

} // namespace ast