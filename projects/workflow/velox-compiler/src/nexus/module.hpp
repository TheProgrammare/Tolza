#pragma once

#include <cassert>
#include <cstddef>
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

using _Path = std::vector<std::string_view>;

enum class EVisibility { Lexical_Scope, File_Scope, Cross_File_Scope };

enum class EModuleKind { Inline, File, Generated, System };

struct Module final {
  struct Port final {
    Module& mod;

    // any global declaration node
    std::unordered_set<ast::_gnid, ast::_gnid_hash> exported_items;

    // node is Reexport
    std::unordered_set<ast::_gnid, ast::_gnid_hash> reexported_module;

    void export_item(ast::_gnid n);
    void reexport_item(ast::_gnid n);
  };

  // generate module
  static Module new_node_module(ast::_gnid p_gnid, std::string_view p_debug_name);
  // generate system module
  static Module from_system(std::string_view p_name, std::string_view p_debug_name)
  {
    Module mod;
    mod.name       = p_name;
    mod.debug_name = p_debug_name;
    mod.kind       = EModuleKind::System;
    return mod;
  }
  static Module from_script(script::_id p_script);


  Port port{*this};

  EModuleKind kind       = EModuleKind::Inline;
  EVisibility visibility = EVisibility::File_Scope;
  _id         id;
  std::string name;
  std::string debug_name;

  script::_id scr_id; // script origin
  scope::_id  scp_id;
  ast::_gnid  gnid;


  scope::Scope&       get_scope() const;
  script::ScriptInfo& get_script() const;
};

struct Graph final {
  struct Crawler final {
    Graph& graph;

    [[nodiscard]] _id resolve_from_current(_id p_start_mod, const _Path& p_path) const;
    [[nodiscard]] _id resolve_from_parent(_id p_start_mod, const _Path& p_path) const;
    [[nodiscard]] _id resolve_from_self(_id p_start_mod, const _Path& p_path) const;
    [[nodiscard]] _id resolve_from_file_root(_id p_start_mod, const _Path& p_path) const;

    // can be reexport
    [[nodiscard]] _id find_parent_module(_id p_mod, std::string_view p_parent_name) const;
    // can be reexport
    [[nodiscard]] _id find_sibling_module(_id p_mod, std::string_view p_sibling_name) const;
    // can be reexport
    [[nodiscard]] _id find_sub_module(_id p_mod, std::string_view p_sub_name) const;
    // can be imported
    [[nodiscard]] _id find_local_module(_id p_mod, std::string_view p_local_name) const;

    // will go up to the file with a ast::root owner
    [[nodiscard]] _id              find_file_module(_id p_start_mod) const;
    [[nodiscard]] std::vector<_id> get_file_modules(_id p_file_mod) const;

    [[nodiscard]] _id find_module(const _Path& path, script::EFileSource src) const;
  };

  struct Tools final {
    Graph& graph;

    [[nodiscard]] symbol::_id find_symbol(_id start_module_id, std::string_view name) const;


    // will mangle his canonical name
    // start module -> parent modules -> file module
    // e.g. file_module_name.parent_modules_names.start_module
    [[nodiscard]] std::string mangle_name(_id p_start_module) const;
    // will generates / returns scripts and modules for each path segment
    // if one script dosen't exists, a runtime error will return unexpected
    [[nodiscard]] _id         build_module_from_path(script::_id scr_id, const std::vector<std::string_view>& p_path,
                                                     script::EFileSource p_file_source, std::string& str_err);
  };

  Graph();

  Crawler crawler{*this};
  Tools   tools{*this};

  std::vector<Module> modules;

  std::unordered_map<_id, _id, _id_hash>              parent;
  std::unordered_map<_id, std::vector<_id>, _id_hash> children;

  std::unordered_map<ast::_gnid, _id, ast::_gnid_hash> owning_module;


  [[nodiscard]] Module* get_by_name(std::string_view name)
  {
    for (auto& mod : modules) {
      if (mod.name == name) return &mod;
    }

    return nullptr;
  }


  [[nodiscard]] Module& get(_id id)
  {
    assert(id < modules.size());
    return modules[id.value()];
  }

  [[nodiscard]] _id new_module(_id p_parent, Module p_mod)
  {
    const _id new_id(modules.size());
    p_mod.id = new_id;

    modules.emplace_back(std::move(p_mod));

    parent[new_id] = p_parent;
    children[p_parent].push_back(new_id);

    return new_id;
  }


  Module& root;
  Module& src;
  Module& std;
  Module& pkg;
  Module& bind;
  Module& vendor;
};


} // namespace module