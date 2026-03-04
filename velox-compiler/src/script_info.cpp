#include "script_info.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"

#include "compiler.hpp"
#include "compiler.hpp"

void ScriptInfo::add_export(const ModuleExportation& exp)
{
  exported_mod.push_back(std::make_unique<ModuleExportation>(exp));
}

void ScriptInfo::add_import(const ModuleImportation& imp)
{
  imported_mod.push_back(std::make_unique<ModuleImportation>(imp));
}

ModuleExportation* ScriptInfo::get_export_module(const fs::path& path)
{
  for (const auto& exp : exported_mod) {
    if (exp->script_exported->file_path == path) return exp.get();
  }

  return nullptr;
}

ModuleImportation* ScriptInfo::get_import_module(const fs::path& path)
{
  for (const auto& imp : imported_mod) {
    if (imp->get_path() == path) return imp.get();
  }

  return nullptr;
}

fs::path ModuleImportation::get_path() const
{
  fs::path p_out;
  switch (import_source) {
  case EImportSource::User:        return p_out = compiler::COMP_CTX.get_source_dir();
  case EImportSource::StandardLib: return p_out = compiler::get_stdlib_dir();
  case EImportSource::UserLib:     return p_out = compiler::get_packages_dir();
  default:                         return p_out = compiler::COMP_CTX.get_source_dir();
  }

  for (auto& elem : path) {
    p_out /= elem;
  }

  p_out /= name;
  p_out.replace_extension(".vlxb");

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
  if (dynamic_cast<const ast::expression::Enum*>(&n)) return Extern_Item::Kind::Enum;
  if (dynamic_cast<const ast::expression::Call*>(&n)) return Extern_Item::Kind::Function;
  if (dynamic_cast<const ast::expression::Call_Pipe*>(&n)) return Extern_Item::Kind::Function;
  if (dynamic_cast<const ast::literal::Component*>(&n)) return Extern_Item::Kind::Component;
  if (dynamic_cast<const ast::literal::Entity*>(&n)) return Extern_Item::Kind::Entity;
  if (dynamic_cast<const ast::Expr_ID*>(&n)) return Extern_Item::Kind::Global;
  if (dynamic_cast<const ast::Expr_ID_Qualified*>(&n)) return Extern_Item::Kind::Global;
  if (dynamic_cast<const ast::Expr_ID_Generic*>(&n)) return Extern_Item::Kind::Type;

  return Extern_Item::Kind::Global;
}