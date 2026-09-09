#include "module/module.hpp"

#include "ast/data.hpp"
#include "ast/dumper.hpp"
#include "ast/node/declaration_global.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"

#include <cassert>
#include <set>
#include <string>
#include <string_view>
#include <utility>

module::Module module::Module::new_node_module(ast::ID nodeid, std::string_view p_debug_name) noexcept
{
  Module mod;
  mod.nodeid     = nodeid;
  mod.debug_name = p_debug_name;
  mod.kind       = EModuleKind::Inline;
  mod.visibility = ast::EVisibility::File_Scope;
  return mod;
}

module::Module module::Module::from_script()
{
  Module mod;
  mod.kind       = EModuleKind::File;
  mod.visibility = ast::EVisibility::Lexical_Scope;

  return mod;
}

void module::Port::export_item(ast::ID nodeid) noexcept
{
  exported_items.try_emplace(ast::get_decl_name(nodeid), nodeid);
}
bool module::Port::reexport_item(module::ID modid, ast::ID nodeid) noexcept
{
  const auto* reexp = nodeid.as<ast::Global_Reexport>();
  assert(reexp && "Invalid node type");

  auto [it, _] = reexported_module.try_emplace(reexp->alias);

  const auto pair   = std::make_pair(modid, nodeid);
  auto [_, success] = it->second.emplace(pair);
  return success;
}

const std::set<module::ReexpRef>& module::Port::find_reexport(const std::string& alias) const noexcept
{
  static const std::set<module::ReexpRef> empty;

  const auto it = reexported_module.find(alias);
  if (it == reexported_module.end()) return empty;

  return it->second;
}