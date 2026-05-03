#include "resolver_symbol.hpp"

#include <cassert>
#include <memory>
#include <ranges>
#include <span>
#include <string>
#include <span>
#include <iostream>
#include <string_view>

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/resolved.hpp"

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/type.hpp"

symbol::_id resolver::Symbol::resolve_id_sym(module::_id current_mod, const ast::Node& n, std::string_view id,
                                             bool is_silent_error)
{
  assert(!id.empty());

  auto m_cur = &compiler::COMPILER.modules.get(current_mod);

  while (m_cur) {
    if (auto sym = compiler::COMPILER.scopes.tools.find_symbol(m_cur->scp_id, id); sym) {
      sym_resolved_count++;
      return sym;
    }

    auto it = compiler::COMPILER.modules.parent.find(m_cur->id);
    if (it == compiler::COMPILER.modules.parent.end()) break;

    m_cur = &compiler::COMPILER.modules.get(it->second);
  }

  if (!is_silent_error) add_error(165, n, "Symbol definition not found!", "");
  return NO_ID;
}
symbol::_id resolver::Symbol::resolve_path_sym(module::_id current_mod, const ast::Node& n, std::string_view id,
                                               const std::vector<std::string_view>& id_quali, ast::EPathSource path_src,
                                               bool is_silent_error)
{
  assert(!id_quali.empty());

  auto path_error = [&]() {
    if (!is_silent_error) add_error(242, n, "Invalid path specified.", "");
  };

  auto m_cur = &compiler::COMPILER.modules.get(current_mod);

  switch (path_src) {
  case ast::EPathSource::NONE: {
    auto mod_found_id = compiler::COMPILER.modules.crawler.resolve_from_current(current_mod, id_quali);
    if (!mod_found_id) return NO_ID;

    return resolve_id_sym(mod_found_id, n, id);
  }
  case ast::EPathSource::Self: {
    auto mod_found_id = compiler::COMPILER.modules.crawler.resolve_from_self(current_mod, id_quali);
    if (!mod_found_id) return NO_ID;

    return resolve_id_sym(mod_found_id, n, id);
  }
  case ast::EPathSource::Super: {
    auto mod_found_id = compiler::COMPILER.modules.crawler.resolve_from_parent(current_mod, id_quali);
    if (!mod_found_id) return NO_ID;

    return resolve_id_sym(mod_found_id, n, id);
  }
  case ast::EPathSource::Root: {
    auto mod_found_id = compiler::COMPILER.modules.crawler.resolve_from_file_root(current_mod, id_quali);
    if (!mod_found_id) return NO_ID;

    return resolve_id_sym(mod_found_id, n, id);
  }
  }
}

size_t resolver::Symbol::start_resolver()
{
  auto root = scr_info.nodes->get_as<ast::Root>(scr_info.root_node_id.get_node_id());
  assert(root);

  for (auto& node : scr_info.nodes->nodes) {
    switch (node->kind()) {
    case ast::ENodeKind::ID: resolve_ID(*scr_info.nodes->get_as<ast::ID>(node->node_id.get_node_id())); break;
    case ast::ENodeKind::ID_Qualified:
      resolve_ID_Qualified(*scr_info.nodes->get_as<ast::ID_Qualified>(node->node_id.get_node_id()));
      break;
    case ast::ENodeKind::ID_Typed:
      resolve_ID_Typed(*scr_info.nodes->get_as<ast::ID_Typed>(node->node_id.get_node_id()));
      break;
    case ast::ENodeKind::Expression_Call:
      resolve_Expression_Call(*scr_info.nodes->get_as<ast::Expression_Call>(node->node_id.get_node_id()));
      break;
    case ast::ENodeKind::Statement_GoTo:
      resolve_Statement_GoTo(*scr_info.nodes->get_as<ast::Statement_GoTo>(node->node_id.get_node_id()));
      break;
    default: break;
    }
  }

  return sym_resolved_count;
}


void resolver::Symbol::resolve_ID(ast::ID& n)
{
  if (compiler::COMPILER.resolved.is_resolved(n.node_id)) return;

  auto scp = compiler::scopes.get(n.scope_id);
  auto mod = compiler::modules.get(scp.module_id);

  auto sym = resolve_id_sym(mod.id, n, n.name, false);
  if (!sym) return;

  compiler::COMPILER.resolved.add(n.node_id, sym);
}
void resolver::Symbol::resolve_ID_Qualified(ast::ID_Qualified& n)
{
  if (compiler::COMPILER.resolved.is_resolved(n.node_id)) return;

  auto scp = compiler::scopes.get(n.scope_id);
  auto mod = compiler::modules.get(scp.module_id);

  auto sym = resolve_id_sym(mod.id, n, n.name, false);
  if (!sym) return;

  compiler::COMPILER.resolved.add(n.node_id, sym);
}
void resolver::Symbol::resolve_ID_Typed(ast::ID_Typed& n)
{
  if (compiler::COMPILER.resolved.is_resolved(n.node_id)) return;

  auto scp = compiler::scopes.get(n.scope_id);
  auto mod = compiler::modules.get(scp.module_id);

  if (auto id = scr_info.nodes->get_as<ast::ID>(n.name.get_node_id())) {
    auto sym = resolve_id_sym(mod.id, n, id->name);
    if (!sym) return;

    compiler::COMPILER.resolved.add(n.node_id, sym);
  } else if (auto id_quali = scr_info.nodes->get_as<ast::ID_Qualified>(n.name.get_node_id())) {
    auto sym = resolve_path_sym(mod.id, n, id_quali->name, id_quali->path, id_quali->src, false);
    if (!sym) return;

    compiler::COMPILER.resolved.add(n.node_id, sym);
  } else {
    assert(false);
  }
}

void resolver::Symbol::resolve_Expression_Call(ast::Expression_Call& n)
{
  auto& n_callee = scr_info.nodes->get(n.callee.get_node_id());
  if (ast::ENodeKind_is_ID(n_callee.kind())) {
    add_error(196, n, "The callee is not an indentifier", "");
    return;
  }

  /*
  auto fn_ptr = dynamic_cast<ast::ACallable*>(id_ptr->identifier_symbol.get());
  if (!fn_ptr) {
    add_error(199, n, "The symbol must be callable type (function, system, lambda)", "");
    return;
  }

  if (n.arguments.size() > fn_ptr->prototype->parameters.size() && !fn_ptr->prototype->is_variadic) {
    add_error_two_nodes(200, n, *fn_ptr, "Too much parameters provided", "");
    return;
  }

  size_t arg_count = 0;
  for (auto& arg : n.arguments) {
    auto n_arg = scr_info.nodes->get_as<ast::Expression_Call_Argument>(arg);
    assert(!n_arg);

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

  Visitor_Default::visit(n);*/
}

void resolver::Symbol::resolve_Statement_GoTo(ast::Statement_GoTo& n)
{
  /*
  if (auto sym = scr_info.sym_m->current_scope->find_symbol(n.label)) {
    if (auto ptr = dynamic_cast<ast::statement::GoTo_Label*>(sym.get())) {
      n.label_sym = ptr;
      return;
    }
    add_error(197, n, "The referenced name is not a label", "");
  }
  add_error(198, n, "Impossible to find the label", "");

  Visitor_Default::visit(n);
  */
}
