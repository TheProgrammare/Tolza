#include "module/tool.hpp"

#include "ast/data.hpp"
#include "common/fileutils.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/modid.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "nexus/forward.hpp"
#include "pipeline/pipeline.hpp"
#include "scope/pool.hpp"
#include "scope/scope.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

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
  if (p_file_source == ::cu::EFileSource::self) {
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

  cu::ID _cuid = PIPELINE.query_CU_at_path(parent_cuid, dir.string());

  return _cuid.get().modules->get_file_root().modid;
}

module::ID module::resolve_anchor(ID ctx, ast::EPathAnchor anchor) noexcept
{
  // anchor resolution = context resolution
  switch (anchor) {
  case ast::EPathAnchor::src:            return module::get_src().modid;
  case ast::EPathAnchor::vendor_lib:     return module::get_vendor().modid;
  case ast::EPathAnchor::stdlib:         return module::get_std().modid;
  case ast::EPathAnchor::pkg_lib:        return module::get_pkg().modid;
  case ast::EPathAnchor::binding:        return module::get_bind().modid;
  case ast::EPathAnchor::relative_super: {
    if (auto ctx_parent = ctx.parent()) return ctx_parent;
    return NO_ID;
  }
  case ast::EPathAnchor::relative_root: return ctx.cu().get().modules->get_file_root().modid;
  case ast::EPathAnchor::NONE:
  case ast::EPathAnchor::relative_self: return ctx; // already the good context
  }
}

module::ID module::resolve_from_children(ID ctx, const std::vector<std::string>& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  for (auto child_id : module::get_children(ctx)) {
    const auto& child = child_id.get();

    if (child.name != path[index]) continue;

    if (auto r = resolve_from_children(child_id, path, index + 1)) return r;
  }

  return NO_ID;
}

module::ID module::resolve_from_import(ID ctx, const std::vector<std::string>& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  const auto& imports = ctx.scope().get().port.find_imports(path[index]);
  for (const auto& [imported_mod, _] : imports) {
    if (auto r = resolve_from_import(imported_mod, path, index + 1)) return r;
  }

  return NO_ID;
}

module::ID module::resolve_from_reexport(ID ctx, const std::vector<std::string>& path, size_t index) noexcept
{
  if (index >= path.size()) return ctx;
  const auto& reexports = ctx.get().port.find_reexport(path[index]);
  for (const auto& [reexp_mod, _] : reexports) {
    if (auto r = resolve_from_reexport(reexp_mod, path, index + 1)) return r;
  }

  return NO_ID;
}

std::pair<module::ID, definition::ID> module::resolve_definition_from_children(ID                              ctx,
                                                                               const std::vector<std::string>& path,
                                                                               size_t                          index,
                                                                               const std::string& sym) noexcept
{
  if (index >= path.size()) {
    const auto defid = scope::find_lexical_symbol(ctx.scope(), sym);
    return {ctx, defid};
  }
  for (auto child_id : module::get_children(ctx)) {
    const auto& child = child_id.get();

    if (child.name != path[index]) continue;

    if (auto [r_ctx, r_defid] = resolve_definition_from_children(child_id, path, index + 1, sym); r_defid)
      return {r_ctx, r_defid};
  }

  return NO_ID;
}
std::pair<module::ID, definition::ID> module::resolve_definition_from_import(ID                              ctx,
                                                                             const std::vector<std::string>& path,
                                                                             size_t                          index,
                                                                             const std::string& sym) noexcept
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
std::pair<module::ID, definition::ID> module::resolve_definition_from_reexport(ID                              ctx,
                                                                               const std::vector<std::string>& path,
                                                                               size_t                          index,
                                                                               const std::string& sym) noexcept
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

module::ID module::resolve_module_path(ID ctx, const std::vector<std::string>& path, ast::EPathAnchor anchor) noexcept
{
  assert(!path.empty());

  ctx = resolve_anchor(ctx, anchor);

  if (auto end = resolve_from_children(ctx, path, 0)) return end;
  if (auto end = resolve_from_import(ctx, path, 0)) return end;
  if (auto end = resolve_from_reexport(ctx, path, 0)) return end;

  return NO_ID;
}

definition::ID module::resolve_path_symbol(module::ID ctx, const std::vector<std::string>& path,
                                           ast::EPathAnchor anchor, const std::string& sym_name) noexcept
{
  assert(!path.empty());

  ctx = resolve_anchor(ctx, anchor);

  if (auto [_, defid] = resolve_definition_from_children(ctx, path, 0, sym_name); defid) return defid;
  if (auto [_, defid] = resolve_definition_from_import(ctx, path, 0, sym_name); defid) return defid;
  if (auto [_, defid] = resolve_definition_from_reexport(ctx, path, 0, sym_name); defid) return defid;

  return NO_ID;
}


std::vector<module::ID>& module::get_children(ID id) noexcept
{
  static std::vector<module::ID> empty;

  assert(id && "Must be valid id");

  if (id.cu()) {
    auto* graph = id.cu().get().modules;
    auto  it    = graph->children.find(id);
    if (it == graph->children.end()) return empty;
    return it->second;
  }


  assert(id.index() > 0 && id.index() < module::get_vendor().modid.index()
         && "Must be to a standard module without script attachment");

  return empty;
}