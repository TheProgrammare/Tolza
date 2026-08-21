#include "nexus/module.hpp"

#include <llvm-19/llvm/Analysis/ScalarEvolutionExpressions.h>
#include <llvm/Analysis/LoopNestAnalysis.h>

#include <cassert>
#include <expected>
#include <filesystem>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <common/common.hpp>
#include <common/fileutils.hpp>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/scope.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"


namespace fs = std::filesystem;

module::Graph::Graph(cu::ID _parent_cuid, cu::ID _cuid) :cuid(_cuid)
{
  if (_parent_cuid) {
    auto& parent_mod = _parent_cuid.get().modules->get_file_root();
    (void)add(parent_mod.modid, Module::from_script());
  } else {
    (void)add(module::ID::invalid(), Module::from_script());
  }
}


module::Module module::Module::new_node_module(ast::ID nodeid, std::string_view p_debug_name) noexcept
{
  Module mod;
  mod.nodeid     = nodeid;
  mod.debug_name = p_debug_name;
  mod.kind       = EModuleKind::Inline;
  return mod;
}

module::Module module::Module::from_script()
{
  Module mod;
  mod.kind = EModuleKind::File;

  return mod;
}

void module::Module::Port::export_item(ast::ID nodeid) noexcept
{
  exported_items.try_emplace(ast::get_decl_name(nodeid), nodeid);
}
bool module::Module::Port::reexport_item(module::ID modid, ast::ID nodeid) noexcept
{
  const auto* reexp = nodeid.as<ast::Global_Reexport>();
  assert(reexp && "Invalid node type");

  auto [it, _] = reexported_module.try_emplace(reexp->alias);

  const auto pair   = std::make_pair(modid, nodeid);
  auto [_, success] = it->second.emplace(pair);
  return success;
}

const std::set<module::ReexpRef>& module::Module::Port::find_reexport(std::string_view alias) const noexcept
{
  static const std::set<module::ReexpRef> empty;

  const auto it = reexported_module.find(alias);
  if (it == reexported_module.end()) return empty;

  return it->second;
}


std::string module::mangle_canonical_module_path(module::ID p_start_module) noexcept
{
  std::vector<std::string_view> out;
  const auto*                   cur_mod = &p_start_module.get();

  while (cur_mod) {
    out.emplace_back(cur_mod->name);

    if (cur_mod->kind == EModuleKind::File) break;

    auto p = cur_mod->modid.parent();
    if (!p) break;

    cur_mod = &p.get();
  }

  std::string final;
  for (auto elem : out) {
    final += std::string(elem) + ".";
  }

  return final.substr(0, final.size() - 1);
}


module::ID module::build_module_from_path(cu::ID parent_cuid, const std::vector<std::string>& p_path,
                                          ::cu::EFileSource p_file_source, std::string& str_err) noexcept
{
  std::vector<ID> modules;

  fs::path dir = EFileSource_to_dir(p_file_source);
  if (p_file_source == ::cu::EFileSource::relative) {
    auto& cu = parent_cuid.get();
    dir      = fs::path(cu.file_info.path).parent_path() / fs::path(cu.file_info.path).stem();
    if (!fs::exists(dir)) {
      str_err = std::format("Directory at \"{}\" doesn't exists.", dir.string());
      return NO_ID;
    }
  }

  for (size_t i = 0; i < p_path.size(); i++) {
    dir /= p_path[i];

    if (i == p_path.size() - 1) {
      const std::string old_dir = dir;
      dir                       = common::fileutils::get_tolza_file(dir.string());
      // is not a tolza file : it could be a directory module: mod.tlz
      if (dir.empty()) {
        dir /= "mod";
        dir = common::fileutils::get_tolza_file(dir.string());
        if (!common::fileutils::is_tolza_file(dir.string())) {
          str_err = std::format("File at \"{}\" doesn't exists or isn't a tolza file.", old_dir);
          return NO_ID;
        }
      } else if (!common::fileutils::is_tolza_file(dir.string())) {
        str_err = std::format("Directory at \"{}\" doesn't exists or isn't a tolza file.", dir.string());
        return NO_ID;
      }

      break;
    }

    if (!fs::exists(dir)) {
      str_err = std::format("Directory at \"{}\" doesn't exists.", dir.string());
      return NO_ID;
    }
  }

  cu::ID _cuid = compiler::pipeline.query_CU_at_path(parent_cuid, dir.string());

  return _cuid.get().modules->get_file_root().modid;
}

module::ID module::resolve_anchor(ID ctx, EPathAnchor anchor) noexcept
{
  // anchor resolution = context resolution
  switch (anchor) {
  case EPathAnchor::src:            return module::get_src().modid;
  case EPathAnchor::vendor_lib:     return module::get_vendor().modid;
  case EPathAnchor::stdlib:         return module::get_std().modid;
  case EPathAnchor::pkg_lib:        return module::get_pkg().modid;
  case EPathAnchor::binding:        return module::get_bind().modid;
  case EPathAnchor::relative_super: {
    if (auto ctx_parent = ctx.parent()) return ctx_parent;
    return NO_ID;
  }
  case EPathAnchor::relative_root: return ctx.cu().get().modules->get_file_root().modid;
  case EPathAnchor::relative_self: return ctx; // already the good context
  }
}

module::ID module::resolve_from_children(ID ctx, const ModPath& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  for (auto child_id : ctx.children()) {
    const auto& child = child_id.get();

    if (child.name != path[index]) continue;

    if (auto r = resolve_from_children(child_id, path, index + 1)) return r;
  }

  return NO_ID;
}

module::ID module::resolve_from_import(ID ctx, const ModPath& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  const auto& imports = ctx.scope().get().port.find_imports(path[index]);
  for (const auto& [imported_mod, _] : imports) {
    if (auto r = resolve_from_import(imported_mod, path, index + 1)) return r;
  }

  return NO_ID;
}

module::ID module::resolve_from_reexport(ID ctx, const ModPath& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  const auto& reexports = ctx.get().port.find_reexport(path[index]);
  for (const auto& [reexp_mod, _] : reexports) {
    if (auto r = resolve_from_reexport(reexp_mod, path, index + 1)) return r;
  }

  return NO_ID;
}

std::pair<module::ID, definition::ID>
module::resolve_definition_from_children(ID ctx, const ModPath& path, size_t index, std::string_view sym) noexcept
{
  if (index >= path.size()) {
    const auto defid = scope::find_lexical_symbol(ctx.scope(), sym);
    return {ctx, defid};
  }
  for (auto child_id : ctx.children()) {
    const auto& child = child_id.get();

    if (child.name != path[index]) continue;

    if (auto [r_ctx, r_defid] = resolve_definition_from_children(child_id, path, index + 1, sym); r_defid)
      return {r_ctx, r_defid};
  }

  return NO_ID;
}
std::pair<module::ID, definition::ID> module::resolve_definition_from_import(ID ctx, const ModPath& path, size_t index,
                                                                             std::string_view sym) noexcept
{
  if (index >= path.size()) {
    const auto defid = scope::find_lexical_symbol(ctx.scope(), sym);
    return {ctx, defid};
  }

  const auto& imports = ctx.scope().get().port.find_imports(path[index]);
  for (const auto& [imported_mod, _] : imports) {
    if (auto [r_ctx, r_defid] = resolve_definition_from_import(imported_mod, path, index + 1, sym); r_defid)
      return {r_ctx, r_defid};
  }

  return NO_ID;
}
std::pair<module::ID, definition::ID>
module::resolve_definition_from_reexport(ID ctx, const ModPath& path, size_t index, std::string_view sym) noexcept
{
  if (index >= path.size()) {
    const auto defid = scope::find_lexical_symbol(ctx.scope(), sym);
    return {ctx, defid};
  }
  const auto& reexports = ctx.get().port.find_reexport(path[index]);
  for (const auto& [imported_mod, _] : reexports) {
    if (auto [r_ctx, r_defid] = resolve_definition_from_import(imported_mod, path, index + 1, sym); r_defid)
      return {r_ctx, r_defid};
  }

  return NO_ID;
}

module::ID module::resolve_module_path(ID ctx, const ModPath& path, EPathAnchor anchor) noexcept
{
  assert(!path.empty());

  ctx = resolve_anchor(ctx, anchor);

  if (auto end = resolve_from_children(ctx, path, 0)) return end;
  if (auto end = resolve_from_import(ctx, path, 0)) return end;
  if (auto end = resolve_from_reexport(ctx, path, 0)) return end;

  return NO_ID;
}

definition::ID module::resolve_path_symbol(module::ID ctx, const ModPath& path, EPathAnchor anchor,
                                           std::string_view sym_name) noexcept
{
  assert(!path.empty());

  ctx = resolve_anchor(ctx, anchor);

  if (auto [_, defid] = resolve_definition_from_children(ctx, path, 0, sym_name); defid) return defid;
  if (auto [_, defid] = resolve_definition_from_import(ctx, path, 0, sym_name); defid) return defid;
  if (auto [_, defid] = resolve_definition_from_reexport(ctx, path, 0, sym_name); defid) return defid;

  return NO_ID;
}


void module::initialization()
{
  const auto& cu = cu::ID::main().get();
  (void)cu.modules->add(ID::invalid(), Module::from_system("root", "root"));
  (void)cu.modules->add(ID::make(cu.cuid, 0), Module::from_system("scr", "source"));
  (void)cu.modules->add(ID::make(cu.cuid, 0), Module::from_system("std", "standard"));
  (void)cu.modules->add(ID::make(cu.cuid, 0), Module::from_system("pkg", "package"));
  (void)cu.modules->add(ID::make(cu.cuid, 0), Module::from_system("bind", "binding"));
  (void)cu.modules->add(ID::make(cu.cuid, 0), Module::from_system("vendor", "vendor third party"));
}

module::Module module::Module::from_system(std::string_view p_name, std::string_view p_debug_name) noexcept
{
  Module mod;
  mod.name       = p_name;
  mod.debug_name = p_debug_name;
  mod.kind       = EModuleKind::System;
  return mod;
}

module::Module& module::get(ID modid) noexcept
{
  auto cuid = modid.cu();
  if (!cuid) {
    switch (modid.index()) {
    case 0:  return get_root();
    case 1:  return get_src();
    case 2:  return get_std();
    case 3:  return get_pkg();
    case 4:  return get_bind();
    case 5:  return get_vendor();
    default: return get_src();
    }
  }
  auto& cu = modid.cu().get();

  assert(modid.index() < cu.modules->modules.size());
  return cu.modules->modules[modid.index()];
}
