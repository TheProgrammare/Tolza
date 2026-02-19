#include "Visitor_Symbol.hpp"

#include <string>

#include "AST/AST_Base.hpp"
#include "ScriptInfo.hpp"
#include "Symbol_Manager.hpp"

Visitor_Symbol::~Visitor_Symbol() = default;

bool Visitor_Symbol::resolve_sym(AST::AIdentifier& id, SYM_DEFINITION& target_resolution, bool silentError)
{
  if (SYM_DEFINITION) return true;

  const std::string name         = id.mangle_local_name();
  const std::string absolue_name = id.mangle_absolute_name();

  // get local symbol
  if (auto sym = scr_info.m_sym->find_symbol(name)) {
    target_resolution = sym.value();
    return sym.value();
  } else if (auto sym = scr_info.m_sym->find_symbol(absolue_name)) {
    target_resolution = sym.value();
    return sym.value();
  }
  // get imported symbol
  else if (!ref.id.path.empty()) {
    const std::string supposed_import_name = ref.id.path[0];

    if (auto imp = scr_info.get_import_module(supposed_import_name)) {
      for (auto& mod : imp->target_modules) {
        if (auto sym = mod->m_sym->find_symbol(absolue_name)) {
          target_resolution = sym.value();
          return sym.value();
        }
      }
    }
  }

  if (!silentError) error_add<152>(ref, "Symbol '" + ref.id.debug_str() + "' definition not found!", "");

  return nullptr;
}
