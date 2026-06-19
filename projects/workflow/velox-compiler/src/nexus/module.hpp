#pragma once

#include <cassert>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace module
{

struct Module;
struct Graph;

using Mod_Path  = std::vector<std::string>;
using Reexp_Ref = std::pair<module::ID, ast::ID>;


enum class EModuleKind : uint8_t { Inline, File, Generated, System };

struct Module final {
  struct Port final {
    Module& mod;

    // any global declaration node
    StringMap<ast::ID> exported_items;

    // node is Reexport
    // alias, <module, node>
    StringMap<std::set<Reexp_Ref>> reexported_module;

    void               export_item(ast::ID n) noexcept;
    [[nodiscard]] bool reexport_item(module::ID m, ast::ID n) noexcept;

    [[nodiscard]] const std::set<Reexp_Ref>& find_reexport(std::string_view alias) const noexcept;
  };

  // generate module
  [[nodiscard]] static Module new_node_module(ast::ID nodeid, std::string_view p_debug_name) noexcept;
  // generate rule module
  [[nodiscard]] static Module from_system(std::string_view p_name, std::string_view p_debug_name) noexcept;
  static Module               from_script();

  // to get private items (non exported nodes)
  // go to the scope port
  Port port{.mod = *this};

  EModuleKind kind       = EModuleKind::Inline;
  EVisibility visibility = EVisibility::File_Scope;
  ID          modid;
  std::string name;
  std::string debug_name;

  scope::ID scpid;
  ast::ID   nodeid;
};

struct Graph final {
  Graph() = delete;

  Graph(cu::ID _parent_cuid, cu::ID _cuid);

  bool         freeze = false;
  const cu::ID cuid;

  std::vector<Module> modules;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  // origin, target
  std::unordered_map<ast::ID, ID, ast::ID::Hash> shortcuts;

  [[nodiscard]] Module* get_by_name(std::string_view name) noexcept
  {
    for (auto& mod : modules) {
      if (mod.name == name) return &mod;
    }

    return nullptr;
  }


  [[nodiscard]] Module& get(ID id) noexcept
  {
    assert(id.offset() < modules.size());
    return modules[id.offset()];
  }
  [[nodiscard]] const Module& get(ID id) const noexcept
  {
    assert(id.offset() < modules.size());
    return modules[id.offset()];
  }

  [[nodiscard]] ID add(ID p_parent, Module modid) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    const auto new_id = ID::make(cuid, modules.size());
    modid.modid       = new_id;

    modules.emplace_back(std::move(modid));

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].emplace_back(new_id);
    }

    return new_id;
  }

  [[nodiscard]] Module& get_file_root() noexcept
  {
    return modules[0];
  }
};


// will mangle his canonical name
// start module -> parent modules -> file module
// e.g. file_module_name.parent_modules_names.start_module
[[nodiscard]] std::string mangle_canonical_module_path(ID p_start_module) noexcept;


// will generates / returns compilation units and modules for each path segment
// if one script doesn't exists, a runtime error will return unexpected
[[nodiscard]] ID build_module_from_path(cu::ID parent_cuid, const std::vector<std::string>& p_path,
                                        cu::EFileSource p_file_source, std::string& str_err) noexcept;


[[nodiscard]] ID                        resolve_anchor(ID ctx, EPathAnchor anchor) noexcept;
[[nodiscard]] ID                        resolve_from_children(ID ctx, const Mod_Path& path, size_t index) noexcept;
[[nodiscard]] ID                        resolve_from_import(ID ctx, const Mod_Path& path, size_t index) noexcept;
[[nodiscard]] ID                        resolve_from_reexport(ID ctx, const Mod_Path& path, size_t index) noexcept;
[[nodiscard]] std::pair<ID, symbol::ID> resolve_symbol_from_children(ID ctx, const Mod_Path& path, size_t index,
                                                                     std::string_view sym) noexcept;
[[nodiscard]] std::pair<ID, symbol::ID> resolve_symbol_from_import(ID ctx, const Mod_Path& path, size_t index,
                                                                   std::string_view sym) noexcept;
[[nodiscard]] std::pair<ID, symbol::ID> resolve_symbol_from_reexport(ID ctx, const Mod_Path& path, size_t index,
                                                                     std::string_view sym) noexcept;
[[nodiscard]] ID                        resolve_module_path(ID ctx, const Mod_Path& path, EPathAnchor anchor) noexcept;
[[nodiscard]] symbol::ID                resolve_path_symbol(ID ctx, const Mod_Path& path, EPathAnchor anchor,
                                                            std::string_view sym_name) noexcept;


void initialization();

[[nodiscard]] Module& get(ID id) noexcept;

// index : 0
[[nodiscard]] static Module& get_root() noexcept
{
  return get(ID::make(cu::ID::main(), 0));
}
// index : 1
[[nodiscard]] static Module& get_src() noexcept
{
  return get(ID::make(cu::ID::main(), 1));
}
// index : 2
[[nodiscard]] static Module& get_std() noexcept
{
  return get(ID::make(cu::ID::main(), 2));
}
// index : 3
[[nodiscard]] static Module& get_pkg() noexcept
{
  return get(ID::make(cu::ID::main(), 3));
}
// index : 4
[[nodiscard]] static Module& get_bind() noexcept
{
  return get(ID::make(cu::ID::main(), 4));
}
// index : 5
[[nodiscard]] static Module& get_vendor() noexcept
{
  return get(ID::make(cu::ID::main(), 5));
}


} // namespace module