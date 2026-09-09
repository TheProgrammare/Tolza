#include "id/modid.hpp"

#include "compiler/compilation_unit.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"

#include <cassert>

module::ID module::ID::parent() const noexcept
{
  assert(*this && "Must be valid id");

  assert(cu() && "Must be atteched to a valid script");

  const auto* graph = cu().get().modules;
  auto        it    = graph->parent.find(*this);
  if (it == graph->parent.end()) return module::ID::invalid();
  return it->second;
}
ast::ID module::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
scope::ID module::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return get().scpid;
}
