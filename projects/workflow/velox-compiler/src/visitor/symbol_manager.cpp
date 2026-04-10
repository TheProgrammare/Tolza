#include "symbol_manager.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "misc/script_info.hpp"
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

  auto sym            = std::make_shared<Symbol_Data>();
  sym->symbol         = declaration;
  sym->mangling       = declaration->mangle_scope() + mangle_id(declaration->name);
  sym->is_exported    = !get_current_export_name().empty();
  sym->type           = declaration->get_symbol_type();
  declaration->symbol = sym.get();
  sym->is_external    = declaration->is_external;

  declarations.push_back(sym);
  if (declaration->is_exported) exportations.push_back(sym);
  return sym;
}

void Symbols_Manager::try_add_extern_sym_to_generate(const ast::AIdentifier& sym, EExtern_Kind kind)
{

  std::string              name;
  std::vector<std::string> path;

  if (auto ptr = dynamic_cast<const ast::Expr_ID_Qualified*>(&sym)) {
    name = ptr->name;
    path = ptr->path;
  } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Type*>(&sym)) {
    if (auto ptr2 = dynamic_cast<const ast::Expr_ID_Qualified*>(ptr)) {
      name = ptr2->name;
      path = ptr2->path;
    } else {
      return;
    }
  } else {
    return;
  }

  auto find_item = [&](const std::vector<Extern_Item>& items) -> bool {
    for (auto item : items) {
      if (item.kind == kind && item.name == name && item.scope == path) return true;
    }
    return false;
  };

  for (auto& ext : scr_info.get_externs()) {
    if (ext->name != path[0]) continue;

    switch (kind) {
    case EExtern_Kind::Function: {
      if (!find_item(ext->extern_fn)) ext->extern_fn.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Type: {
      if (!find_item(ext->extern_ty)) ext->extern_ty.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Global: {
      if (!find_item(ext->extern_glo)) ext->extern_glo.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Enum: {
      if (!find_item(ext->extern_enum)) ext->extern_enum.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Union: {
      if (!find_item(ext->extern_union)) ext->extern_union.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Flag: {
      if (!find_item(ext->extern_flag)) ext->extern_flag.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Component: {
      if (!find_item(ext->extern_comp)) ext->extern_comp.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::System: {
      if (!find_item(ext->extern_sys)) ext->extern_sys.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Entity: {
      if (!find_item(ext->extern_entity)) ext->extern_entity.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Generic: {
      if (!find_item(ext->extern_gen)) ext->extern_gen.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    case EExtern_Kind::Metacode: {
      if (!find_item(ext->extern_metacode)) ext->extern_metacode.emplace_back(Extern_Item(name, path, kind, sym));
      break;
    }
    }
  }
}

bool Symbols_Manager::is_external_symbol(const ast::AIdentifier& sym) const
{
  if (auto ptr = dynamic_cast<const ast::Expr_ID*>(&sym)) {
    return false;
  } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Type*>(&sym)) {
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

  return result;
}

Symbol_Data* Symbols_Manager::find_local_symbol(const std::span<const std::string>& scope, const std::string& name)
{
  for (auto& decl : declarations) {
    if (decl->symbol->name == name && decl->symbol->is_visible_in(scope)) {
      return decl.get();
    }
  }
  return nullptr;
}

Symbol_Data* Symbols_Manager::find_exported_symbol(const std::span<const std::string>& scope, const std::string& name)
{
  for (auto& exp : exportations) {
    if (exp->symbol->name == name && exp->symbol->is_visible_in(scope)) {
      return exp.get();
    }
  }
  return nullptr;
}
