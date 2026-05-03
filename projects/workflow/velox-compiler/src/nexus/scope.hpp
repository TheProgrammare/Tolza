#pragma once

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include <cassert>
#include <string_view>
#include <unordered_map>
#include <unordered_set>


namespace scope
{


struct Scope final {
  struct Symbols final {
    // declaration name, ast::ADeclaration
    std::unordered_map<std::string_view, symbol::_id> symbols;

    void                      add_symbol(std::string_view name, symbol::_id sym);
    [[nodiscard]] symbol::_id get_scope_symbol(std::string_view name);
  };

  struct Port final {
    // import node instruction
    std::unordered_set<ast::_gnid, ast::_gnid_hash> imports_prepared;

    // target_module, import node instruction
    std::unordered_map<module::_id, ast::_gnid, module::_id_hash> imported;

    void               prepare_import(ast::_gnid import_node_id);
    [[nodiscard]] bool import_module(module::_id target_module, ast::_gnid import_node_id);
  };

  Scope() = default;

  static Scope from_any(std::string_view p_debug_name)
  {
    Scope scp;
    scp.debug_name = p_debug_name;
    return scp;
  }
  static Scope from_node(ast::_gnid p_gnid, std::string_view p_debug_name)
  {
    Scope scp;
    scp.gnid       = p_gnid;
    scp.scr_id     = p_gnid.get_script_id();
    scp.debug_name = p_debug_name;
    return scp;
  }
  static Scope from_module(script::_id p_script, module::_id p_mod)
  {
    Scope scp;
    scp.scr_id    = p_script;
    scp.module_id = p_mod;
    return scp;
  }

  _id         id;
  std::string debug_name;


  Symbols symbols;
  Port    port;

  script::_id scr_id;    // script origin
  module::_id module_id; // module origin
  ast::_gnid  gnid;      // node origin
};

struct Graph final {
  struct Tools final {
    Graph& graph;

    // will check in local scope and parent scopes
    [[nodiscard]] symbol::_id find_symbol(_id start_scope_id, std::string_view name) const;
    [[nodiscard]] ast::_gnid  get_scope_node(ast::ENodeKind kind, _id start_scope_id) const;
  };

  Tools tools{*this};

  std::vector<Scope> scopes;

  std::unordered_map<_id, _id, _id_hash>              parent;
  std::unordered_map<_id, std::vector<_id>, _id_hash> children;

  std::unordered_map<_id, module::_id, _id_hash> module;


  Scope& get(_id id)
  {
    assert(id < scopes.size());
    return scopes[id.value()];
  }

  _id new_scope(_id p_parent, Scope p_scp)
  {
    const _id new_id(scopes.size());
    p_scp.id = new_id;

    scopes.emplace_back(std::move(p_scp));

    parent[new_id] = p_parent;
    children[p_parent].push_back(new_id);

    return new_id;
  }
};


} // namespace scope