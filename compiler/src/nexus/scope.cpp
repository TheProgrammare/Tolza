#include "nexus/scope.hpp"

#include <cassert>
#include <set>
#include <string_view>
#include <utility>

#include "ast/ast_base.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/module.hpp"
#include "nexus/definition.hpp"
#include "nexus/ids.hpp"


scope::Scope scope::Scope::make_from_module(module::ID modid, ast::ID nodeid) noexcept
{
  return {
      .debug_name = modid.get().debug_name,
      .nodeid     = nodeid,
      .modid      = modid,
  };
}
scope::Scope scope::Scope::make_from_node(ast::ID nodeid) noexcept
{
  return {
      .debug_name = ast::dump(nodeid),
      .nodeid     = nodeid,
  };
}
scope::Scope scope::Scope::make_from_script_root(module::ID modid, ast::ID nodeid) noexcept
{
  return {
      .debug_name = std::format("root \"{}\"", modid.cu().get().file_info.get_file_name()),
      .nodeid     = nodeid,
      .modid      = modid,
  };
}


bool scope::Scope::Port::import_module(module::ID target_module, ast::ID import_nodeid) noexcept
{
  const auto* imp = import_nodeid.as<ast::Import>();
  assert(imp && "Invalid node");

  auto [it, _] = imported.try_emplace(imp->alias);

  auto pair         = std::make_pair(target_module, import_nodeid);
  auto [_, success] = it->second.emplace(pair);
  return success;
}
const std::set<scope::Imp_Ref>& scope::Scope::Port::find_imports(std::string_view alias) const noexcept
{
  static const std::set<scope::Imp_Ref> empty;
  const auto                            it = imported.find(alias);
  if (it == imported.end()) return empty;

  return it->second;
}


bool scope::Scope::Items::add_definition(definition::ID sym) noexcept
{
  auto [_, success] = definitions.try_emplace(sym.get().get_name(), sym);
  return success;
}


definition::ID scope::Scope::Items::find_definition(std::string_view name) const noexcept
{
  auto it = definitions.find(name);
  if (it != definitions.end()) return it->second;
  return NO_ID;
}

definition::ID scope::find_lexical_symbol(scope::ID ctx, std::string_view name) noexcept
{
  const auto& scp = ctx.get();

  return scp.items.find_definition(name);
}
definition::ID scope::find_in_chain_scope(scope::ID ctx, std::string_view name) noexcept
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

scope::Scope& scope::get(ID id) noexcept
{
  const auto& cu = id.cu().get();
  return cu.scopes->get(id);
}