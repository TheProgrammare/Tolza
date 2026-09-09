#include "pool/node_to_def.hpp"

#include "ast/dumper.hpp"
#include "ast/forward.hpp"
#include "ast/pool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "id/defid.hpp"
#include "module/pool.hpp"
#include "module/tool.hpp"
#include "pool/node_resolved.hpp"

#include <algorithm>
#include <cassert>
#include <format>
#include <string>

std::string definition::Definition::mangle_name() const noexcept
{
  auto modid = defid.module();

  return std::format("{}.{}", module::mangle_canonical_module_path(modid), std::string(get_name()));
}

std::string definition::Definition::get_name() const noexcept
{
  return ast::get_decl_name(nodeid);
}

std::string definition::Arena::get_def_name(ID defid) const noexcept
{
  const auto& def = get(defid);

  return ast::get_decl_name(def.nodeid);
}

void definition::Arena::inject_to_resolved_def() const noexcept
{
  auto& binds = COMPILER.resolved.bindings;
  binds.reserve(binds.size() + definitions.size());

  for (const auto& def : definitions) {
    binds.emplace(def.nodeid, def.defid);
  }
}


definition::Definition* definition::Arena::from_node(ast::ID nodeid) noexcept
{
  const auto it =
      std::ranges::find_if(definitions, [&](const definition::Definition& def) { return def.nodeid == nodeid; });
  if (it == definitions.end()) return nullptr;
  return &(*it);
}


definition::Definition& definition::get(ID defid) noexcept
{
  auto cu = defid.cu();
  assert(cu && "Must be a valid script");
  return cu.get().definitions->get(defid);
}