#include "id/scpid.hpp"

#include "compiler/compilation_unit.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "scope/pool.hpp"
#include "scope/scope.hpp"

#include <cassert>

scope::ID scope::ID::parent() const noexcept
{
  assert(*this && "Must be valid id");
  scope::Graph* graph;

  assert(cu() && "Must be attached to a valid script");

  if (cu()) {
    const auto* graph = cu().get().scopes;

    auto it = graph->parent.find(*this);
    if (it == graph->parent.end()) return scope::ID::invalid();
    return it->second;
  }

  return scope::ID::invalid();
}
ast::ID scope::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
module::ID scope::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return get().modid;
}
