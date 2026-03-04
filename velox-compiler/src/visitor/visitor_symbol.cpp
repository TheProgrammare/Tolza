#include "visitor_symbol.hpp"

#include <memory>
#include <string>

#include "ast/ast_base.hpp"
#include "script_info.hpp"
#include "symbol_manager.hpp"

Visitor_Symbol::~Visitor_Symbol() = default;

bool Visitor_Symbol::resolve_sym(ast::AIdentifier& id, std::weak_ptr<Symbol_Data>& target_resolution, bool silentError)
{
  if (!target_resolution.expired()) return true;

  const std::string name         = id.mangle_local_name();
  const std::string absolue_name = id.mangle_qualified_name();

  // get local symbol
  if (auto sym = scr_info.m_sym->find_symbol(name)) {
    target_resolution = sym.value();
    return true;
  } else if (auto sym = scr_info.m_sym->find_symbol(absolue_name)) {
    target_resolution = sym.value();
    return true;
  }
  // get imported symbol
  else if (!id.is_qualified_id()) {
    const std::string supposed_import_name = id.get_supposed_import_name();

    if (auto imp = scr_info.get_import_module(supposed_import_name)) {
      for (auto& mod : imp->target_modules) {
        if (auto sym = mod->m_sym->find_symbol(absolue_name)) {
          target_resolution = sym.value();
          return true;
        }
      }
    }
  }

  if (!silentError) error_add<152>(id, "Symbol '" + id.debug_str() + "' definition not found!", "");

  return false;
}
