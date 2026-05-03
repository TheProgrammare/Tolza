#include "nexus/module.hpp"

#include <llvm/Analysis/LoopNestAnalysis.h>

#include <cassert>
#include <expected>
#include <filesystem>
#include <initializer_list>
#include <ranges>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "misc/error_output.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/scope.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "compiler/compiler.hpp"


namespace fs = std::filesystem;

module::Module module::Module::new_node_module(ast::_gnid p_gnid, std::string_view p_debug_name)
{
  Module mod;
  mod.scr_id     = p_gnid.get_script_id();
  mod.gnid       = p_gnid;
  mod.debug_name = p_debug_name;
  mod.kind       = EModuleKind::Inline;
  return mod;
}

module::Module module::Module::from_script(script::_id scr_id)
{
  Module mod;
  mod.scr_id = scr_id;
  mod.kind   = EModuleKind::File;

  auto& scr = compiler::COMPILER.pipeline.get_script(scr_id);
  mod.gnid  = scr.root_node_id;

  return mod;
}


scope::Scope& module::Module::get_scope() const
{
  assert(kind != EModuleKind::System);
  return compiler::COMPILER.scopes.get(scp_id);
}

script::ScriptInfo& module::Module::get_script() const
{
  assert(kind != EModuleKind::System);
  return compiler::COMPILER.pipeline.get_script(scr_id);
}

void module::Module::Port::export_item(ast::_gnid n)
{
  exported_items.insert(n);
}
void module::Module::Port::reexport_item(ast::_gnid n)
{
  auto& scr = mod.get_script();
  assert(!scr.nodes->get_as<ast::Global_Reexport>(n.get_node_id()));

  reexported_module.insert(n);
}


symbol::_id module::Graph::Tools::find_symbol(_id start_module_id, std::string_view name) const
{
  auto& cur_mod = graph.get(start_module_id);
  auto& scp     = cur_mod.get_scope();

  return compiler::COMPILER.scopes.tools.find_symbol(scp.id, name);
}

std::string module::Graph::Tools::mangle_name(module::_id p_start_module) const
{
  std::vector<std::string_view> out;
  auto                          cur_mod = &graph.get(p_start_module);

  while (cur_mod) {
    out.push_back(cur_mod->name);

    if (cur_mod->kind == EModuleKind::File) break;

    auto it = compiler::COMPILER.modules.parent.find(cur_mod->id);
    if (it != compiler::COMPILER.modules.parent.end())
      cur_mod = &compiler::COMPILER.modules.get(it->second);
    else
      break;
  }

  std::string final;
  for (auto elem : out) {
    final += std::string(elem) + ".";
  }

  return final.substr(0, final.size() - 1);
}


module::_id module::Graph::Crawler::find_parent_module(module::_id p_mod, std::string_view p_parent_name) const
{
  if (p_parent_name.empty()) return NO_ID;

  auto& cur_mod = graph.get(p_mod);

  auto it = graph.parent.find(cur_mod.id);
  if (it == graph.parent.end()) return NO_ID;

  return it->second;
}

module::_id module::Graph::Crawler::find_sibling_module(module::_id p_mod, std::string_view p_sibling_name) const
{
  if (p_sibling_name.empty()) return NO_ID;

  auto parent_id = graph.parent.find(p_mod);
  if (parent_id == graph.parent.end()) return NO_ID;

  auto children_ids = graph.children.find(parent_id->second);
  if (children_ids == graph.children.end()) return NO_ID;

  for (auto child_id : children_ids->second) {
    auto& child = graph.get(child_id);
    if (child.name == p_sibling_name) return child_id;
  }

  return NO_ID;
}


module::_id module::Graph::Crawler::find_sub_module(module::_id p_mod, std::string_view p_sub_name) const
{
  if (p_sub_name.empty()) return NO_ID;

  auto& mod = graph.get(p_mod);

  // scope elements
  if (auto children = graph.children.find(p_mod); children != graph.children.end()) {
    for (auto child_id : children->second) {
      auto& child = graph.get(child_id);
      if (child.name == p_sub_name) return child_id;
    }
  }


  // search on imported modules
  for (auto reexport_node_id : mod.port.reexported_module) {
    auto n = mod.get_script().nodes->get_as<ast::Global_Reexport>(reexport_node_id.get_node_id());
    assert(!n);


    if (n->alias == p_sub_name) {
      auto it = graph.owning_module.find(n->node_id);
      assert(it != graph.owning_module.end());

      return it->second;
    }
  }

  return NO_ID;
}

module::_id module::Graph::Crawler::find_local_module(module::_id p_mod, std::string_view p_local_name) const
{
  if (p_local_name.empty()) return NO_ID;

  auto& mod = graph.get(p_mod);

  // parent scope elements
  if (auto parent = graph.parent.find(p_mod); parent != graph.parent.end()) {
    return find_sub_module(parent->second, p_local_name);
  }

  // search on imported modules
  for (auto exp_mod_id : mod.port.reexported_module) {
    auto it = graph.owning_module.find(exp_mod_id);
    assert(it != graph.owning_module.end());

    auto& exp_mod = graph.get(it->second);
    if (exp_mod.name == p_local_name) return it->second;
  }

  return NO_ID;
}

module::_id module::Graph::Crawler::resolve_from_current(module::_id p_start_mod, const _Path& p_path) const
{
  if (p_path.empty()) return NO_ID;

  // local search
  {
    auto cur_mod = resolve_from_self(p_start_mod, p_path);
    if (cur_mod) return cur_mod;
  }

  // recursive ascending parent search
  {
    _id cur_parent;
    do {
      auto it = graph.parent.find(p_start_mod);
      if (it == graph.parent.end()) {
        break;
      }

      cur_parent = it->second;
    } while (cur_parent);
    if (cur_parent) return NO_ID;

    return resolve_from_self(cur_parent, p_path);
  }

  return NO_ID;
}
module::_id module::Graph::Crawler::resolve_from_parent(module::_id p_start_mod, const _Path& p_path) const
{
  if (p_path.empty()) return NO_ID;

  _id cur_mod = p_start_mod;
  for (auto& mod_name : p_path) {
    cur_mod = find_local_module(cur_mod, mod_name);
    // no scope found : finding failed
    if (cur_mod) return NO_ID;
  }
  return cur_mod;
}
module::_id module::Graph::Crawler::resolve_from_self(module::_id p_start_mod, const _Path& p_path) const
{
  if (p_path.empty()) return NO_ID;

  auto cur_mod = p_start_mod;
  for (auto& mod_name : p_path) {
    cur_mod = find_local_module(cur_mod, mod_name);
    // no scope found : finding failed
    if (cur_mod) return NO_ID;
  }

  return cur_mod;
}
module::_id module::Graph::Crawler::resolve_from_file_root(module::_id p_start_mod, const _Path& p_path) const
{
  if (p_path.empty()) return NO_ID;

  auto file_mod = find_file_module(p_start_mod);
  if (file_mod) return NO_ID;

  auto cur_mod = file_mod;
  for (auto& mod_name : p_path) {
    cur_mod = find_local_module(cur_mod, mod_name);
    // no scope found : finding failed
    if (cur_mod) return NO_ID;
  }
  return cur_mod;
}

module::_id module::Graph::Tools::build_module_from_path(script::_id                          scr_id,
                                                         const std::vector<std::string_view>& p_path,
                                                         ::script::EFileSource p_file_source, std::string& str_err)
{
  std::vector<_id> modules;
  _Path            files;

  fs::path dir = EFileSource_to_dir(p_file_source);
  if (p_file_source == ::script::EFileSource::relative) {
    auto& scr = compiler::COMPILER.pipeline.get_script(scr_id);

    dir = fs::path(scr.file_info.path).stem();
  }

  if (!fs::exists(dir)) {
    str_err = "Directory at \"" + dir.string() + "\" dosen't exists.";
    return NO_ID;
  }

  for (auto elem : p_path) {
    dir /= elem;
    if (!fs::exists(dir)) {
      str_err = "Directory at \"" + dir.string() + "\" dosen't exists.";
      return NO_ID;
    }

    files.push_back(dir.string());
  }

  auto  __scr_id = compiler::pipeline.query_script_at_path(files.back());
  auto& scr      = compiler::COMPILER.pipeline.get_script(__scr_id);

  return scr.module_id;
}

std::vector<module::_id> module::Graph::Crawler::get_file_modules(_id p_file_mod) const
{
  std::vector<_id>                  result;
  std::unordered_set<_id, _id_hash> visited;
  std::stack<_id>                   stack;

  stack.push(p_file_mod);
  visited.insert(p_file_mod);

  while (!stack.empty()) {
    auto current = stack.top();
    stack.pop();

    auto it = graph.children.find(current);
    if (it == graph.children.end()) continue;

    for (const auto& child : it->second) {
      if (visited.insert(child).second) {
        result.push_back(child);
        stack.push(child);
      }
    }
  }

  return result;
}
module::_id module::Graph::Crawler::find_file_module(_id p_start_mod) const
{
  auto _it = graph.parent.find(p_start_mod);
  if (_it == graph.parent.end()) return NO_ID;

  auto parent_mod_id = _it->second;
  while (parent_mod_id) {
    auto& parent_mod = graph.get(parent_mod_id);
    if (parent_mod.kind == EModuleKind::File) return parent_mod_id;

    auto it = graph.parent.find(parent_mod_id);
    if (it == graph.parent.end()) return NO_ID;
    parent_mod_id = it->second;
  }
}


module::_id module::Graph::Crawler::find_module(const _Path& path, script::EFileSource src) const
{
  assert(!path.empty());

  Module* parent_mod = nullptr;

  switch (src) {
  case script::EFileSource::src:        parent_mod = &graph.src; break;
  case script::EFileSource::vendor_lib: parent_mod = &graph.vendor; break;
  case script::EFileSource::stdlib:     parent_mod = &graph.std; break;
  case script::EFileSource::pkg_lib:    parent_mod = &graph.pkg; break;
  case script::EFileSource::binding:    parent_mod = &graph.bind; break;
  case script::EFileSource::relative:   {
    _Path relative_path(path.begin() + 1, path.end());
    parent_mod = graph.get_by_name(path[0]);

    if (!parent_mod) return NO_ID;

    return resolve_from_self(parent_mod->id, relative_path);
  }
  }

  return resolve_from_self(parent_mod->id, path);
}


module::Graph::Graph() :root(get(new_module(_id(-1), Module::from_system("root", "compilation root")))),
    src(get(new_module(root.id, Module::from_system("src", "user module")))),
    std(get(new_module(root.id, Module::from_system("std", "standard library module")))),
    pkg(get(new_module(root.id, Module::from_system("pkg", "packages module")))),
    bind(get(new_module(root.id, Module::from_system("bind", "bindings module")))),
    vendor(get(new_module(root.id, Module::from_system("vendor", "vendor module"))))
{
}