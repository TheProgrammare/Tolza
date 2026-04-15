#include "visitor_symbol.hpp"

#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <span>
#include <iostream>

#include "misc/script_info.hpp"
#include "misc/symbol_manager.hpp"

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

bool Visitor_Symbol::resolve_sym(ast::AIdentifier& p_id, bool p_is_silent_error)
{
  if (p_id.identifier_symbol) return true;

  auto qualified_error = [&]() {
    if (!p_is_silent_error)
      add_error(152, p_id, "Qualified symbol [" + p_id.debug_str() + "] definition not found!", "");
  };

  auto base_error = [&]() {
    if (!p_is_silent_error) add_error(165, p_id, "Symbol [" + p_id.debug_str() + "] definition not found!", "");
  };

  ast::Expr_ID_Qualified* id_quali = nullptr;
  ast::Expr_ID*           id_base  = nullptr;

  if (auto ptr = dynamic_cast<ast::Expr_ID_Qualified*>(&p_id)) {
    id_quali = ptr;
  } else if (auto ptr = dynamic_cast<ast::Expr_ID_Type*>(&p_id)) {
    if (auto ptr2 = dynamic_cast<ast::Expr_ID_Qualified*>(ptr->name)) {
      id_quali = ptr2;
    } else if (auto ptr2 = dynamic_cast<ast::Expr_ID*>(ptr->name)) {
      id_base = ptr2;
    }
  } else if (auto ptr = dynamic_cast<ast::Expr_ID*>(&p_id)) {
    id_base = ptr;
  }


  // search on qualification : local + imported
  if (id_quali) {
    // local search

    if (auto scp = scr_info.sym_m->find_scope_from_path(p_id.node_module, id_quali->path, id_quali->at_parent,
                                                        id_quali->at_self, id_quali->at_root)) {

      if (auto sym = scp->find_symbol(id_quali->name)) {
        p_id.identifier_symbol = sym;
        return true;
      }

      qualified_error();
      return false;
    }

    std::vector<std::string> path_descending = id_quali->path;


    // imported search
    if (auto imp = scr_info.mod_m->get_imported_module(id_quali->mangle_path())) {
      std::vector<std::string> sub(id_quali->path.size() > 1 ? id_quali->path.begin() + 1 : id_quali->path.end(),
                                   id_quali->path.end());

      if (auto imp_scp = imp->target_script->sym_m->find_scope_from_path(p_id.node_scope, sub, false, false, true)) {
        if (auto imp_sym = imp_scp->find_symbol(id_quali->name)) {
          p_id.identifier_symbol = imp_sym;
          return true;
        }
      }
    }

    qualified_error();
    return false;
  }
  // search on name : local
  else {
    // search on local module and ascending modules
    if (auto decl = p_id.node_module->find_symbol_recursive_ascending(id_base->name)) {
      p_id.identifier_symbol = decl;
      return true;
    }

    base_error();
    return false;
  }
}

void Visitor_Symbol::visit(ast::Expr_ID& n)
{
  resolve_sym(n, false);

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Qualified& n)
{
  resolve_sym(n, false);

  Visitor_Default::visit(n);
}
void Visitor_Symbol::visit(ast::Expr_ID_Type& n)
{
  resolve_sym(n, false);
  n.name->identifier_symbol = n.identifier_symbol;

  Visitor_Default::visit(n);
}

void Visitor_Symbol::visit(ast::expression::Call& n)
{
  n.callee->accept(*this);

  auto id_ptr = dynamic_cast<ast::AIdentifier*>(n.callee.get());
  if (!id_ptr) {
    add_error(196, n, "The callee is not an indentifier", "");
    return;
  }

  auto fn_ptr = dynamic_cast<ast::ACallable*>(id_ptr->identifier_symbol.get());
  if (!fn_ptr) {
    add_error(199, n, "The symbol must be callable type (function, system, lambda)", "");
    return;
  }

  if (n.param_args.size() > fn_ptr->prototype->parameters.size() && !fn_ptr->prototype->is_variadic) {
    add_error_two_nodes(200, n, *fn_ptr, "Too much parameters provided", "");
    return;
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
  if (auto sym = scr_info.sym_m->current_scope->find_symbol(n.label)) {
    if (auto ptr = dynamic_cast<ast::statement::GoTo_Label*>(sym.get())) {
      n.label_sym = ptr;
      return;
    }
    add_error(197, n, "The referenced name is not a label", "");
  }
  add_error(198, n, "Impossible to find the label", "");

  Visitor_Default::visit(n);
}
