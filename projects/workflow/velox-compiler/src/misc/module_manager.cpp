#include "module_manager.hpp"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <filesystem>
#include <ranges>
#include <vector>

#include "ast/ast_declaration.hpp"
#include "common.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"

#include "compiler/compiler.hpp"
#include "compiler_context.hpp"
#include "pipeline/pipeline_filesystem.hpp"
#include "script_info.hpp"

namespace fs = std::filesystem;


[[nodiscard]] bool module::Path_Regex::is_compatible_path(const std::vector<std::string>& p_path) const
{
  if (!is_prefix) return p_path == path;

  if (p_path == path) {
    return true;
  } else {
    auto view = p_path;
    while (!view.empty()) {
      view.pop_back();
      if (view == path) return true;
    }
    return false;
  }
}
[[nodiscard]] std::vector<std::string> module::Path_Regex::decorate_name(const std::string& p_name) const
{
  if (!is_prefix) return path;
  auto view = path;
  view.push_back(p_name);
  return view;
}

module::Module::Module(std::shared_ptr<ScriptInfo> p_script_info) :owner(p_script_info->root_node),
    name(p_script_info->file_info.get_file_name()), debug_name("file " + p_script_info->file_info.get_file_name())
{
}

bool module::Module::is_file() const
{
  return dynamic_cast<ast::Root*>(owner.lock().get());
}

module::Module::Module(ast::_Symbol_Id p_owner, const std::string& p_debug_name) :owner(p_owner),
    debug_name(p_debug_name), name()
{
}


std::string module::Module::get_mangling_name() const
{
  std::vector<std::string> out;
  auto                     current_scope = shared_from_this();
  while (current_scope) {
    out.push_back(current_scope->name);
    current_scope = current_scope->parent_module;
  }

  return ast::mangle_path(out);
}

bool module::Symbols::add_symbol(ast::_Symbol_Id p_sym)
{
  p_declaration->node_module = shared_from_this();

  auto [it, success] = items.try_emplace(p_declaration->declaration_name, p_declaration);
  return success;
}

bool module::Module::import_module(module::_Mod_Id p_mod, const _Path& p_alias)
{
  auto [it, success] = imported_modules.try_emplace(p_alias.empty() ? _Path{p_mod->name} : p_alias, p_mod);
  return success;
}

bool module::Module::reexport_module(module::_Mod_Id p_mod, const _Path& p_alias)
{
  auto [it, success] = imported_modules.try_emplace(p_alias.empty() ? _Path{p_mod->name} : p_alias, p_mod);
  return success;
}

module::_Item module::Module::find_item(const std::string& p_name)
{
  if (auto it = items.find(p_name); it != items.end()) return it->second;
  return nullptr;
}


module::_Mod_Id module::Module::find_parent_module(module::_Mod_Id p_mod, const _Path& p_path)
{
  if (p_path.empty()) return parent_module;


  if (p_path.size() == 1) {
    auto checked_scope = shared_from_this();
    while (checked_scope) {
      if (checked_scope->name == p_path[0]) return checked_scope;

      checked_scope = checked_scope->parent_module;
    }
    return checked_scope
  } else {
  }

  return nullptr;
}

module::_Mod_Id module::Graph::find_sibling_module(module::_Mod_Id p_mod, const _Path& p_path) const
{
  if (!parent_module) return nullptr;

  return parent_module->find_sub_module(p_name);
}


module::_Mod_Id module::Module::find_sub_module(module::_Mod_Id p_mod, const _Path& p_path)
{
  if (sub_modules.empty()) return nullptr;

  auto sub_it = sub_modules.find(p_name);
  if (sub_it != sub_modules.end()) return sub_it->second;

  return nullptr;
}

module::_Mod module::Module::resolve_from_parent(module::_Mod_Id p_start_mod, const std::vector<std::string>& p_path)
{
  if (p_path.empty()) return nullptr;

  auto current_mod = parent_module;

  for (auto& mod_name : p_path) {
    current_mod = current_mod->find_sub_module(mod_name);

    // no scope found : finding failed
    if (!current_mod) return nullptr;
  }
  return current_mod;
}
module::_Mod module::Module::resolve_from_self(module::_Mod_Id p_start_mod, const _Path& p_path)
{
  if (p_path.empty()) return nullptr;
  if (p_path.size() == 1 && p_path[0] == name) return shared_from_this();

  std::vector<std::string> path = p_path;
  {
    auto view = p_path | std::views::drop(1);
    if (p_path[0] == name) path = std::vector(view.begin(), view.end());
  }


  auto current_mod = shared_from_this();

  for (auto& mod_name : path) {
    current_mod = current_mod->find_sub_module(mod_name);

    // no scope found : finding failed
    if (!current_mod) break;
  }

  if (!current_mod) {
    auto& access_name = path[0];
    auto  it          = imported_modules.find(access_name);
    if (it != imported_modules.end()) {
      return it->second->resolve_from_self(path);
    }
  }

  return current_mod;
}
module::_Mod module::Module::resolve_from_root(module::_Mod_Id p_start_mod, const _Path& p_path) const
{
  if (p_path.empty()) return find_file_module();

  auto current_mod = find_file_module();

  for (auto& mod_name : p_path) {
    current_mod = current_mod->find_sub_module(mod_name);

    // no scope found : finding failed
    if (!current_mod) return nullptr;
  }
  return current_mod;
}
module::_Mod module::Module::resolve_from_any(module::_Mod_Id p_start_mod, const _Path& p_path) const
{
  // implicit lookup
  for (size_t i = 0; i < p_path.size(); i++) {
    auto current = shared_from_this();

    _Mod start = nullptr;

    // find nearest match
    while (current) {
      if (auto found = current->find_sub_module(p_path[i])) {
        start = found;
        break;
      }
      current = current->parent_module;
    }

    if (!start) continue;

    if (i == p_path.size() - 1) return start;

    // validate suffix
    auto current_2 = start;
    bool success   = true;

    for (size_t j = i + 1; j < p_path.size(); j++) {
      current_2 = current_2->find_sub_module(p_path[j]);
      if (!current_2) {
        success = false;
        break;
      }
    }

    if (success) return current_2;
  }

  return nullptr;
}

std::expected<module::_Mod, std::string> module::build_module_from_path(module::_Mod_Id p_current_module, _Path& p_path,
                                                                        EFileSource p_file_source)
{
  std::vector<_Mod> modules;
  _Path             files;

  fs::path dir = EFileSource_to_dir(p_file_source);
  if (p_file_source == EFileSource::relative)
    dir = fs::path(p_current_module->find_file_module()->owner.lock()->node_scr_info->file_info.path).stem();

  if (!fs::exists(dir)) return std::unexpected("Directory at \"" + dir.string() + "\" dosen't exists.");

  for (auto elem : p_path) {
    dir /= elem;
    if (!fs::exists(dir)) return std::unexpected("Directory at \"" + dir.string() + "\" dosen't exists.");

    files.push_back(dir);
  }

  auto scripts = pipeline_start_filesystem_on_files({files.back()});

  return scripts[0]->module_root;
}

module::Module* module::Graph::get_module(ModuleId id) const
{
  if (id >= modules.size()) return nullptr;

  return &graph.modules[id];
}

module::_Mod_Id module::Graph::new_module(_Mod_Id p_parent, Module p_mod)
{
  const _Mod_Id new_id = static_cast<_Mod_Id>(modules.size());

  modules.emplace_back(std::move(p_mod));

  parent[new_id] = p_parent;

  children[p_parent].push_back(new_id);

  return new_id;
}