#include "visitor_symbol.hpp"

#include <span>
#include <string>
#include <span>
#include <iostream>

#include "script_info.hpp"
#include "symbol_manager.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"

Visitor_Symbol::~Visitor_Symbol() = default;

bool Visitor_Symbol::resolve_sym(ast::AIdentifier& id, Symbol_Data*& target_resolution, bool silentError)
{
  auto local_search = [&](std::span<const std::string> scope, const std::string& name) {
    if (auto sym = scr_info.m_sym->find_local_symbol(scope, name)) {
      target_resolution = sym;
      return true;
    }
    return false;
  };

  // from qualification
  if (id.is_qualified_id()) {
    // search on qualification
    if (local_search(id.get_qualification_path(), id.get_base_name())) {
      return true;
    }
    // search in imported modules
    else {
      if (auto imp = scr_info.get_import_module(id.get_qualification_path())) {
        std::span<const std::string> sub_qualification =
            std::span<const std::string>(id.get_qualification_path().begin(), id.get_qualification_path().size() - 1);

        for (auto& mod : imp->target_modules) {
          if (auto sym = mod->m_sym->find_exported_symbol(sub_qualification, id.get_base_name())) {
            target_resolution = sym;
            return true;
          }
        }
      }
    }

    // qualification is invalid
    if (!silentError) error_add(152, id, "Qualified symbol [" + id.debug_str() + "] definition not found!", "");
    return false;
  }

  // from contextual scope
  if (local_search(id._scope, id.get_base_name())) return true;

  if (!silentError) error_add(165, id, "Symbol [" + id.get_base_name() + "] definition not found!", "");
  return false;
}

void Visitor_Symbol::visit(ast::Expr_ID& n)
{
  resolve_sym(n, n.symbol, false);
}
void Visitor_Symbol::visit(ast::Expr_ID_Qualified& n)
{
  resolve_sym(n, n.symbol, false);
}
void Visitor_Symbol::visit(ast::Expr_ID_Type& n)
{
  resolve_sym(n, n.symbol, false);
  n.name->symbol = n.symbol;
}

void Visitor_Symbol::visit(ast::expression::Call& n)
{
  if (auto ptr = dynamic_cast<ast::AIdentifier*>(n.callee.get())) {
    resolve_sym(*ptr, n.function_symbol, false);
  } else {
  }
}
