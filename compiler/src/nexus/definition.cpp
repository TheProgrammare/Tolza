#include "definition.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/module.hpp"
#include "nexus/resolved.hpp"

#include <algorithm>

std::string definition::Definition::mangle_name() const noexcept
{
  auto modid = defid.module();

  return module::mangle_canonical_module_path(modid) + "." + std::string(get_name());
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
  auto& binds = compiler::resolved.bindings;
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