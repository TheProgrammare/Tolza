#include "nexus/scope.hpp"

#include "ast/ast_declaration_global.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/symbol.hpp"
#include <cassert>
#include <string_view>


void scope::Scope::Port::prepare_import(ast::_gnid import_node_id)
{
  imports_prepared.insert(import_node_id);
}

bool scope::Scope::Port::import_module(module::_id target_module, ast::_gnid import_node_id)
{
  auto [_, success] = imported.try_emplace(target_module, import_node_id);
  return success;
}
void scope::Scope::Symbols::add_symbol(std::string_view name, symbol::_id sym)
{
  symbols[name] = sym;
}


symbol::_id scope::Scope::Symbols::get_scope_symbol(std::string_view name)
{
  auto it = symbols.find(std::string(name));
  if (it != symbols.end()) return it->second;
  return NO_ID;
}

symbol::_id scope::Graph::Tools::find_symbol(scope::_id start_scope_id, std::string_view name) const
{
  auto cur_scp = &graph.get(start_scope_id);
  while (cur_scp) {
    if (auto sym = cur_scp->symbols.get_scope_symbol(name)) return sym;

    auto it = graph.parent.find(cur_scp->id);
    if (it == graph.parent.end()) return NO_ID;
    cur_scp = &graph.get(it->second);
  }

  return NO_ID;
}

ast::_gnid scope::Graph::Tools::get_scope_node(ast::ENodeKind kind, scope::_id start_scope_id) const
{
  auto cur_scp = &graph.get(start_scope_id);

  while (cur_scp) {
    auto& n = compiler::COMPILER.nodes.get(cur_scp->gnid);
    if (n.kind() == kind) return cur_scp->gnid;

    auto it = graph.parent.find(cur_scp->id);
    if (it == graph.parent.end()) return NO_ID;
    cur_scp = &graph.get(it->second);
  }

  return NO_ID;
}
