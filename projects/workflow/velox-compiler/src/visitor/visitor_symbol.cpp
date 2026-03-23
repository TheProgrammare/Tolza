#include "visitor_symbol.hpp"

#include <span>
#include <string>
#include <span>
#include <iostream>

#include "misc/script_info.hpp"
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
#include "visitor/visitor_default.hpp"

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

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Qualified& n)
{
  resolve_sym(n, n.symbol, false);

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Type& n)
{
  resolve_sym(n, n.symbol, false);
  n.name->symbol = n.symbol;

  Visitor_Default::visit(n);
}

void Visitor_Symbol::visit(ast::expression::Call& n)
{
  if (auto ptr = dynamic_cast<ast::AIdentifier*>(n.callee.get())) {
    if (resolve_sym(*ptr, n.function_symbol, false)) {
      ptr->symbol = n.function_symbol;
    } else {
      error_add(195, n, "Impossible to find the function symbol", "");
      return;
    }
  } else {
    error_add(196, n, "The callee is not an indentifier", "");
    return;
  }

  ast::ACallable* fn_ptr = dynamic_cast<ast::ACallable*>(n.function_symbol->symbol.get());
  if (!fn_ptr) {
    error_add(199, n, "The symbol must be callable type (function, system, lambda)", "");
  }

  if (n.param_args.size() > fn_ptr->prototype->parameters.size() && !fn_ptr->prototype->isVariadic) {
    error_two_lines(200, n, *fn_ptr, "Too much parameters provided", "");
  }

  size_t arg_count = 0;
  for (auto& arg : n.param_args) {
    arg->fn_type = fn_ptr->prototype.get();

    if (arg->name.empty()) {
      if (fn_ptr->prototype->isVariadic && arg_count > fn_ptr->prototype->parameters.size()) {
        // variadic arg, no type inferred on parameter
        // variadic prameters resolved in visitor type on expression inferred type
      } else {
        auto& param        = fn_ptr->prototype->parameters[arg_count];
        arg->inferred_type = param->type.get();
        arg_count++;
      }
    } else {
      bool found = false;
      for (auto& param : fn_ptr->prototype->parameters) {
        if (param->name == arg->name) {
          arg->inferred_type = param->type.get();
          found              = true;
          break;
        }
      }

      if (!found) error_two_lines(201, *arg, *fn_ptr, "The parameter name dosen't exists.", "");
    }
  }

  Visitor_Default::visit(n);
}

void Visitor_Symbol::visit(ast::statement::GoTo& n)
{
  if (auto sym = scr_info.m_sym->find_local_symbol(n._scope, n.label)) {
    if (auto ptr = dynamic_cast<ast::statement::GoTo_Label*>(sym->symbol.get())) {
      n.label_sym = ptr;
      return;
    }
    error_add(197, n, "The referenced name is not a label", "");
  }
  error_add(198, n, "Impossible to find the label", "");

  Visitor_Default::visit(n);
}
