#include "Symbol_Manager.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Declaration.hpp"
#include "ScriptInfo.hpp"
#include <memory>
#include <unistd.h>

std::string Symbols_Manager::get_current_export_name() const
{
  if (current_scope_path.empty()) return "";

  if (current_scope_path[0].type == EScopeType::Export) return current_scope_path[0].name;

  return "";
}

std::shared_ptr<Symbol_Data> Symbols_Manager::add_decl(std::shared_ptr<AST::ADeclaration> declaration)
{
  declaration->_scope = get_current_path();

  auto sym         = std::make_shared<Symbol_Data>();
  sym->symbol      = declaration;
  sym->mangling    = declaration->mangle_scope() + mangle_id(declaration->name);
  sym->is_exported = !get_current_export_name().empty();
  sym->type        = declaration->get_symbol_type();

  if (auto ptr = std::dynamic_pointer_cast<AST::Declaration::Global>(declaration))
    sym->is_external = ptr->isExtern;
  else if (auto ptr = std::dynamic_pointer_cast<AST::Declaration::Function>(declaration))
    sym->is_external = ptr->isExtern;

  declarations.push_back(sym);
  return sym;
}

std::shared_ptr<Symbol_Data> Symbols_Manager::add_decl_ex_nihilo(std::shared_ptr<AST::ADeclaration> declaration)
{
  auto sym          = add_decl(declaration);
  sym->is_exported  = false;
  sym->is_external  = false;
  sym->is_ex_nihilo = true;
  return sym;
}

void Symbols_Manager::enter_scope(const std::string& name, EScopeType type, size_t depth)
{
  current_scope_path.push_back(ScopeData{name, type, depth});
}

void Symbols_Manager::exit_scope() { current_scope_path.pop_back(); }

std::vector<std::string> Symbols_Manager::get_current_path() const
{
  std::vector<std::string> result;
  result.reserve(current_scope_path.size());
  for (auto& elem : current_scope_path) result.push_back(elem.name);

  std::reverse(result.begin(), result.end());
  return result;
}

std::optional<std::shared_ptr<AST::ADeclaration>> Symbols_Manager::find_symbol(const std::string& full_name)
{
  for (auto& sym : declarations) {
    if (sym->mangling == full_name) return sym->symbol;
  }
  return std::nullopt;
}
