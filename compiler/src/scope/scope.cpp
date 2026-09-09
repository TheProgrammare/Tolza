#include "scope/scope.hpp"

#include "ast/dumper.hpp"
#include "ast/node/base.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/file_info.hpp"
#include "id/base.hpp"
#include "id/defid.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "pool/node_to_def.hpp"

#include <cassert>
#include <format>
#include <set>
#include <string>
#include <utility>


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


bool scope::Port::import_module(module::ID target_module, ast::ID import_nodeid) noexcept
{
  const auto* imp = import_nodeid.as<ast::Import>();
  assert(imp && "Invalid node");

  auto [it, _] = imported.try_emplace(imp->alias);

  auto pair         = std::make_pair(target_module, import_nodeid);
  auto [_, success] = it->second.emplace(pair);
  return success;
}
const std::set<scope::Imp_Ref>& scope::Port::find_imports(const std::string& alias) const noexcept
{
  static const std::set<scope::Imp_Ref> empty;
  const auto                            it = imported.find(alias);
  if (it == imported.end()) return empty;

  return it->second;
}


bool scope::Items::add_definition(definition::ID sym) noexcept
{
  auto [_, success] = definitions.try_emplace(sym.get().get_name(), sym);
  return success;
}


definition::ID scope::Items::find_definition(const std::string& name) const noexcept
{
  auto it = definitions.find(name);
  if (it != definitions.end()) return it->second;
  return NO_ID;
}
