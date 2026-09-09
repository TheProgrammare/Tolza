#include "scope/pool.hpp"

#include "compiler/compilation_unit.hpp"
#include "id/base.hpp"
#include "id/defid.hpp"
#include "id/scpid.hpp"
#include "nexus/forward.hpp"
#include "scope/scope.hpp"

#include <cassert>
#include <string>
#include <utility>
#include <vector>


definition::ID scope::find_lexical_symbol(scope::ID ctx, const std::string& name) noexcept
{
  const auto& scp = ctx.get();

  return scp.items.find_definition(name);
}
definition::ID scope::find_in_chain_scope(scope::ID ctx, const std::string& name) noexcept
{
  auto cu = ctx.cu();
  while (ctx) {
    if (auto sym = find_lexical_symbol(ctx, name)) return sym;

    ctx = ctx.parent();
    // outside the internal script
    if (ctx.cu() != cu) return NO_ID;
    cu = ctx.cu();
  }

  return NO_ID;
}

scope::Graph::Graph(cu::ID _cuid)
  : cuid(_cuid)
{
  auto scp = scope::Scope();
  (void)add(ID::invalid(), scp);
}


[[nodiscard]] scope::ID scope::Graph::add(ID p_parent, Scope& p_scp) noexcept
{
  assert(!freeze && "Pool is immutable after parsing pass");

  const auto new_id = ID::make(cuid, storage.size());
  p_scp.scpid       = new_id;

  auto* scp  = storage.create_get<Scope>(p_scp);
  scp->scpid = new_id;

  if (p_parent) {
    parent[new_id] = p_parent;
    children[p_parent].emplace_back(new_id);
  }

  return new_id;
}

std::vector<scope::ID>& scope::get_children(ID id) noexcept
{
  static std::vector<scope::ID> empty;

  assert(id && "Must be valid id");

  assert(id.cu() && "Must be attached to a valid script");

  if (id.cu()) {
    auto* graph = id.cu().get().scopes;

    auto it = graph->children.find(id);
    if (it == graph->children.end()) return empty;
    return it->second;
  }

  return empty;
}