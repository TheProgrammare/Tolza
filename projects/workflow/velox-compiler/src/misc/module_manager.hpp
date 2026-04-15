#pragma once

#include <expected>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>


#include "ast/ast_base.hpp"
#include "ast/ast_forward.hpp"
#include "misc/script_info.hpp"


struct ScriptInfo;

enum class EFileSource;

namespace ast
{
using _Symbol_Id = size_t;
}

namespace module
{

struct Path_Regex {
  std::vector<std::string> path;
  bool                     is_prefix = false;

  [[nodiscard]] bool                     is_compatible_path(const std::vector<std::string>& p_path) const;
  [[nodiscard]] std::vector<std::string> decorate_name(const std::string& p_name) const;
};

struct Module;
struct Graph;

using _Owner  = std::weak_ptr<ast::ADeclaration>;
using _Import = std::shared_ptr<ast::declaration::Import>;
using _Mod    = std::shared_ptr<Module>;
using _Mod_Id = size_t;
using _Export = std::shared_ptr<ast::declaration::Export>;
using _Path   = std::vector<std::string>;
using _Id     = std::string;

constexpr _Mod_Id INVALID_PARENT = -1;

enum class EVisibility { Private, Internal, Public };

struct Symbols final {
  // declaration name, ast::ADeclaration
  std::unordered_map<_Id, ast::_Symbol_Id> items;

  [[nodiscard]] bool            add_symbol(ast::_Symbol_Id p_sym);
  [[nodiscard]] ast::_Symbol_Id find_symbol(const _Id& p_name);
};

struct Port final {
  std::unordered_map<_Id, _Mod_Id> imported_modules;
  std::unordered_map<_Id, _Mod_Id> exported_modules;

  [[nodiscard]] bool import_module(_Mod_Id target, const _Path& p_alias);
  [[nodiscard]] bool reexport_module(_Mod_Id target, const _Path& p_alias);
};


struct Module final {
  Module() = delete;
  Module(const std::string p_name, const std::string& p_debug_name)
    : name(p_name)
    , debug_name(p_debug_name)
  {
  }
  Module(ast::_Symbol_Id p_owner, const std::string& p_debug_name);
  Module(std::shared_ptr<ScriptInfo> p_script_info);


  EVisibility visibility = EVisibility::Internal;
  _Id         name;
  std::string debug_name;

  Symbols items;
  Port    port;

  _Symbol_Id owner;


  // if owner is ast::root
  [[nodiscard]] bool is_file() const;
  [[nodiscard]] bool is_inline() const
  {
    return !is_file();
  }

  [[nodiscard]] std::string get_mangling_name() const;
  [[nodiscard]] bool        add_sub_module(_Mod p_mod);
};

struct Graph final {
  std::vector<Module> modules;

  std::unordered_map<_Mod_Id, _Mod_Id>              parent;
  std::unordered_map<_Mod_Id, std::vector<_Mod_Id>> children;

  Module* get(_Mod_Id id) const;

  [[nodiscard]] _Mod_Id resolve_from_parent(_Mod_Id p_start_mod, const _Path& p_path) const;
  [[nodiscard]] _Mod_Id resolve_from_self(_Mod_Id p_start_mod, const _Path& p_path) const;
  [[nodiscard]] _Mod_Id resolve_from_root(_Mod_Id p_start_mod, const _Path& p_path) const;
  [[nodiscard]] _Mod_Id resolve_from_any(_Mod_Id p_start_mod, const _Path& p_path) const;

  [[nodiscard]] _Mod_Id find_parent_module(_Mod_Id p_mod, const _Id& p_path) const;
  [[nodiscard]] _Mod_Id find_sibling_module(_Mod_Id p_mod, const _Id& p_path) const;
  [[nodiscard]] _Mod_Id find_sub_module(_Mod_Id p_mod, const _Id& p_path) const;

  // will go up to the file with a ast::root owner
  [[nodiscard]] _Mod_Id find_file_module(_Mod p_start_mod) const;

  [[nodiscard]] _Mod_Id new_module(_Mod_Id p_parent, Module p_mod);
};

static Graph& get_graph()
{
  static Graph gh = []() -> Graph {
    Graph graph;

    auto root_id = graph.new_module(INVALID_PARENT, Module("::", "root"));
    graph.new_module(root_id, Module("usr", "usr"));
    graph.new_module(root_id, Module("std", "std"));
    graph.new_module(root_id, Module("pkg", "pkg"));
    graph.new_module(root_id, Module("bind", "bind"));

    return graph;
  }();

  return gh;
}


// will generates / returns scripts and modules for each path segment
// if one script dosen't exists, a runtime error will return unexpected
std::expected<_Mod, std::string> build_module_from_path(module::_Mod_Id p_current_module, _Path& p_path,
                                                        EFileSource p_file_source);

} // namespace module