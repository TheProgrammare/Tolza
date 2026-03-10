#include "script_info.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"

#include "compiler.hpp"
#include "compiler.hpp"
#include <cstddef>
#include <memory>

void ScriptInfo::add_export(const ModuleExportation& exp)
{
  exported_mod.push_back(std::make_unique<ModuleExportation>(exp));
}

void ScriptInfo::add_import(const ModuleImportation& imp)
{
  imported_mod.push_back(std::make_unique<ModuleImportation>(imp));
}

std::shared_ptr<ModuleExportation> ScriptInfo::get_export_module(const fs::path& path)
{
  for (const auto& exp : exported_mod) {
    if (exp->script_exported->file_path == path) return exp;
  }

  return nullptr;
}

std::shared_ptr<ModuleImportation> ScriptInfo::get_import_module(std::span<const std::string> path)
{
  size_t                             bigger_seg_compatible = -1;
  std::shared_ptr<ModuleImportation> last_imp_found;
  for (const auto& imp : imported_mod) {
    if (imp->path.size() > path.size()) continue;

    if (imp->path.empty()) {
      if (imp->name == path[0]) {
        bigger_seg_compatible = 0;
        last_imp_found        = imp;
        continue;
      }
    }

    for (size_t i = 0; i < imp->path.size(); i++) {
      if (i == 0) {
        if (imp->path[0] == path[0]) {
          if (bigger_seg_compatible == -1 || bigger_seg_compatible < i) {
            bigger_seg_compatible = i;
            last_imp_found        = imp;
          }
        } else {
          break;
        }
      }
      if (imp->path[i] != path[i]) {
        if (bigger_seg_compatible == -1 || bigger_seg_compatible < i) {
          bigger_seg_compatible = i;
          last_imp_found        = imp;
        }
      }
    }
  }

  if (bigger_seg_compatible == -1) return nullptr;

  return last_imp_found;
}

fs::path ModuleImportation::get_normalized_path() const
{
  return get_path().parent_path() / get_path().stem();
}

fs::path ModuleImportation::get_path() const
{
  fs::path p_out;
  switch (import_source) {
  case EImportSource::User:        p_out = compiler::COMP_CTX.get_source_dir(); break;
  case EImportSource::StandardLib: p_out = compiler::get_stdlib_dir(); break;
  case EImportSource::UserLib:     p_out = compiler::get_packages_dir(); break;
  case EImportSource::Extern:      p_out = compiler::COMP_CTX.get_binding_dir(); break;
  default:                         p_out = compiler::COMP_CTX.get_source_dir(); break;
  }

  for (auto& elem : path) {
    p_out /= elem;
  }

  p_out /= name;
  if (!extern_lib.empty()) p_out /= extern_lib;
  p_out.replace_extension(".velox");

  return p_out;
}

std::set<ModuleImportation*> ScriptInfo::get_externs()
{
  std::set<ModuleImportation*> result;
  for (auto& imp : imported_mod) {
    if (imp->is_external()) {
      result.insert(imp.get());
    }
  }
  return result;
}

std::set<fs::path> ScriptInfo::get_extern_languages()
{
  std::set<fs::path> result;
  for (auto& imp : get_externs()) {
    if (imp->is_external()) {
      result.insert(imp->get_path());
    }
  }
  return result;
}

Extern_Item::Kind AST_AExpression_to_Extern_Item_Kind(const ast::AExpression& n)
{
  if (dynamic_cast<const ast::literal::Enum*>(&n)) return Extern_Item::Kind::Enum;
  if (dynamic_cast<const ast::expression::Call*>(&n)) return Extern_Item::Kind::Function;
  if (dynamic_cast<const ast::expression::Call_Pipe*>(&n)) return Extern_Item::Kind::Function;
  if (dynamic_cast<const ast::literal::Component*>(&n)) return Extern_Item::Kind::Component;
  if (dynamic_cast<const ast::literal::Entity*>(&n)) return Extern_Item::Kind::Entity;
  if (dynamic_cast<const ast::Expr_ID*>(&n)) return Extern_Item::Kind::Global;
  if (dynamic_cast<const ast::Expr_ID_Qualified*>(&n)) return Extern_Item::Kind::Global;
  if (dynamic_cast<const ast::Expr_ID_Type*>(&n)) return Extern_Item::Kind::Type;

  return Extern_Item::Kind::Global;
}