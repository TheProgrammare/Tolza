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

bool Visitor_Symbol::resolve_sym(ast::AIdentifier& p_id, Symbol_Data*& p_target_resolution, bool p_is_silent_error)
{
  auto local_search = [&](std::span<const std::string> scope, const std::string& name) {
    if (auto sym = scr_info.m_sym->find_local_symbol(scope, name)) {
      p_target_resolution = sym;
      return true;
    }
    return false;
  };

  // from qualification
  if (p_id.is_qualified_id()) {
    // search on qualification
    if (local_search(p_id.get_qualification_path(), p_id.get_base_name())) {
      return true;
    }
    // search in imported modules
    else {
      if (auto imp = scr_info.get_import_module(p_id.get_qualification_path())) {
        std::span<const std::string> sub_qualification = std::span<const std::string>(
            p_id.get_qualification_path().begin(), p_id.get_qualification_path().size() - 1);

        for (auto& mod : imp->target_modules) {
          if (auto sym = mod->m_sym->find_exported_symbol(sub_qualification, p_id.get_base_name())) {
            p_target_resolution = sym;
            return true;
          }
        }
      }
    }

    // qualification is invalid
    if (!p_is_silent_error)
      add_error(152, p_id, "Qualified symbol [" + p_id.debug_str() + "] definition not found!", "");
    return false;
  }

  // from contextual scope
  if (local_search(p_id._scope, p_id.get_base_name())) return true;

  if (!p_is_silent_error) add_error(165, p_id, "Symbol [" + p_id.get_base_name() + "] definition not found!", "");
  return false;
}

void Visitor_Symbol::visit(ast::Expr_ID& n)
{
  resolve_sym(n, n.identifier_symbol, false);

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Qualified& n)
{
  resolve_sym(n, n.identifier_symbol, false);

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Type& n)
{
  resolve_sym(n, n.identifier_symbol, false);
  n.name->identifier_symbol = n.identifier_symbol;

  Visitor_Default::visit(n);
}

void Visitor_Symbol::visit(ast::expression::Call& n)
{
  if (auto ptr = dynamic_cast<ast::AIdentifier*>(n.callee.get())) {
    if (resolve_sym(*ptr, n.function_symbol, false)) {
      ptr->identifier_symbol = n.function_symbol;
    } else {
      add_error(195, n, "Impossible to find the function symbol", "");
      return;
    }
  } else {
    add_error(196, n, "The callee is not an indentifier", "");
    return;
  }

  ast::ACallable* fn_ptr = dynamic_cast<ast::ACallable*>(n.function_symbol->symbol.get());
  if (!fn_ptr) {
    add_error(199, n, "The symbol must be callable type (function, system, lambda)", "");
  }

  if (n.param_args.size() > fn_ptr->prototype->parameters.size() && !fn_ptr->prototype->is_variadic) {
    add_error_two_nodes(200, n, *fn_ptr, "Too much parameters provided", "");
  }

  size_t arg_count = 0;
  for (auto& arg : n.param_args) {
    arg->fn_type = fn_ptr->prototype;

    if (arg->name.empty()) {
      if (fn_ptr->prototype->is_variadic && arg_count > fn_ptr->prototype->parameters.size()) {
        // variadic arg, no type inferred on parameter
        // variadic prameters resolved in visitor type on expression inferred type
      } else {
        auto& param                   = fn_ptr->prototype->parameters[arg_count];
        arg->expression_inferred_type = param->type;
        arg_count++;
      }
    } else {
      bool found = false;
      for (auto& param : fn_ptr->prototype->parameters) {
        if (param->declaration_name == arg->name) {
          arg->expression_inferred_type = param->type;
          found                         = true;
          break;
        }
      }

      if (!found) add_error_two_nodes(201, *arg, *fn_ptr, "The parameter name dosen't exists.", "");
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
    add_error(197, n, "The referenced name is not a label", "");
  }
  add_error(198, n, "Impossible to find the label", "");

  Visitor_Default::visit(n);
}
