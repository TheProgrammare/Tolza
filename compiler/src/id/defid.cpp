#include "id/defid.hpp"

#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "id/typeid.hpp"
#include "pool/node_to_def.hpp"

#include <cassert>

type::ID definition::ID::type() const noexcept
{
  assert(*this && "Must be valid id");
  return node().type();
}
ast::ID definition::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
scope::ID definition::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return node().scope();
}
module::ID definition::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return node().module();
}
