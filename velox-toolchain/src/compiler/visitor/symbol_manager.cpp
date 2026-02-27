#include "symbol_manager.hpp"

#include "compiler/ast/ast_base.hpp"
#include "compiler/ast/ast_declaration.hpp"
#include "compiler/ast/ast_statement.hpp"
#include "compiler/script_info.hpp"
#include <memory>
#include <unistd.h>

std::string Symbols_Manager::get_current_export_name() const
{
  if (current_scope_path.empty()) return "";

  if (current_scope_path[0].type == EScopeType::Export) return current_scope_path[0].name;

  return "";
}

std::shared_ptr<Symbol_Data> Symbols_Manager::add_decl(std::shared_ptr<ast::ADeclaration> declaration)
{
  declaration->_scope = get_current_path();

  auto sym         = std::make_shared<Symbol_Data>();
  sym->symbol      = declaration;
  sym->mangling    = declaration->mangle_scope() + mangle_id(declaration->name);
  sym->is_exported = !get_current_export_name().empty();
  sym->type        = declaration->get_symbol_type();

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::Global>(declaration))
    sym->is_external = ptr->isExtern;
  else if (auto ptr = std::dynamic_pointer_cast<ast::declaration::Function>(declaration))
    sym->is_external = ptr->isExtern;

  declarations.push_back(sym);
  return sym;
}

std::shared_ptr<Symbol_Data> Symbols_Manager::add_decl_ex_nihilo(std::shared_ptr<ast::ADeclaration> declaration)
{
  auto sym          = add_decl(declaration);
  sym->is_exported  = false;
  sym->is_external  = false;
  sym->is_ex_nihilo = true;
  return sym;
}

void Symbols_Manager::add_external_symbol(const ast::AIdentifier& sym, Extern_Item::Kind kind)
{
  std::string              name;
  std::vector<std::string> path;

  if (auto ptr = dynamic_cast<const ast::Expr_ID_Qualified*>(&sym)) {
    name = ptr->name;
    path = ptr->path;
  } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Generic*>(&sym)) {
    if (auto ptr2 = dynamic_cast<const ast::Expr_ID_Qualified*>(ptr)) {
      name = ptr2->name;
      path = ptr2->path;
    } else {
      return;
    }
  } else {
    return;
  }

  for (auto& ext : scr_info.get_externs()) {
    Extern_Item item(name, path, kind);
    ext->extern_references.push_back(item);
  }
}

bool Symbols_Manager::is_external_symbol(const ast::AIdentifier& sym) const
{
  if (auto ptr = dynamic_cast<const ast::Expr_ID*>(&sym)) {
    return false;
  } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Generic*>(&sym)) {
    if (auto ptr2 = dynamic_cast<const ast::Expr_ID*>(ptr)) return false;
  }

  return scr_info.get_extern_languages().contains(sym.get_supposed_import_name());
}


void Symbols_Manager::enter_scope(const std::string& name, EScopeType type, size_t depth)
{
  current_scope_path.push_back(ScopeData{name, type, depth});
}

void Symbols_Manager::exit_scope()
{
  current_scope_path.pop_back();
}

std::vector<std::string> Symbols_Manager::get_current_path() const
{
  std::vector<std::string> result;
  result.reserve(current_scope_path.size());
  for (auto& elem : current_scope_path) result.push_back(elem.name);

  std::reverse(result.begin(), result.end());
  return result;
}

std::optional<std::shared_ptr<Symbol_Data>> Symbols_Manager::find_symbol(const std::string& full_name)
{
  for (auto& sym : declarations) {
    if (sym->mangling == full_name) return sym;
  }
  return std::nullopt;
}
