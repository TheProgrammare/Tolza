#pragma once

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include <cassert>
#include <set>
#include <string_view>
#include <unordered_map>


namespace scope
{

using Imp_Ref = std::pair<module::ID, ast::ID>;


struct Scope final {
  [[nodiscard]] static Scope make_from_module(module::ID modid, ast::ID nodeid) noexcept;
  [[nodiscard]] static Scope make_from_node(ast::ID nodeid) noexcept;
  [[nodiscard]] static Scope make_from_script_root(module::ID modid, ast::ID nodeid) noexcept;

  struct Items final {
    // declaration name, reference
    StringMap<symbol::ID> symbols;

    [[nodiscard]] bool       add_symbol(symbol::ID sym) noexcept;
    [[nodiscard]] symbol::ID find_symbol(std::string_view name) const noexcept;
  };

  struct Port final {
    // alias, <module, node>
    StringMap<std::set<Imp_Ref>> imported;

    [[nodiscard]] bool                     import_module(module::ID target_module, ast::ID import_nodeid) noexcept;
    [[nodiscard]] const std::set<Imp_Ref>& find_imports(std::string_view alias) const noexcept;
  };

  ID          scpid;
  std::string debug_name;


  Items items;
  Port  port;

  EVisibility visibility;

  ast::ID    nodeid; // node origin
  module::ID modid;
};

[[nodiscard]] Scope& get(ID id) noexcept;

[[nodiscard]] ast::ID get_scope_node(ast::ENodeKind kind, ID start_scpid) noexcept;

[[nodiscard]] symbol::ID find_lexical_symbol(scope::ID ctx, std::string_view name) noexcept;
[[nodiscard]] symbol::ID find_in_chain_scope(scope::ID ctx, std::string_view name) noexcept;

struct Graph final {
  Graph() = delete;

  Graph(cu::ID _cuid)
    : cuid(_cuid)
  {
    (void)add(ID::invalid(), Scope());
  }

  bool freeze = false;

  const cu::ID cuid;

  std::vector<Scope> scopes;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  std::unordered_map<ID, module::ID, ID::Hash> module;


  [[nodiscard]] Scope& get(ID id) noexcept
  {

    assert(id.cu() == cuid && "Must be the same script");
    assert(id.offset() < scopes.size());
    return scopes[id.offset()];
  }

  [[nodiscard]] const Scope& get(ID id) const noexcept
  {
    assert(id.cu() == cuid && "Must be the same script");
    assert(id.offset() < scopes.size());
    return scopes[id.offset()];
  }

  [[nodiscard]] ID add(ID p_parent, Scope p_scp)
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    const auto new_id = ID::make(cuid, scopes.size());
    p_scp.scpid       = new_id;

    scopes.emplace_back(std::move(p_scp));

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].emplace_back(new_id);
    }

    return new_id;
  }

  [[nodiscard]] Scope& get_file_root() noexcept
  {
    return get(ID::make(cuid, 0));
  }
};


} // namespace scope