#include "resolver_symbol.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <string_view>

#include "Neargye/magic_enum.hpp"
#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/resolved.hpp"

#include "nexus/ast/ast.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/symbol.hpp"

symbol::ID resolver::Symbol::resolve_id_sym(scope::ID ctx, const ast::Node& n, std::string_view id,
                                            bool is_silent_error)
{
  assert(!id.empty());

  if (auto sym = scope::find_in_chain_scope(ctx, id)) {
    sym_resolved_count++;
    return sym;
  }

  if (!is_silent_error) add_error(165, n, "Symbol definition not found!", "");
  return NO_ID;
}
symbol::ID resolver::Symbol::resolve_path_sym(module::ID ctx, const ast::Node& n, std::string_view id,
                                              const std::vector<std::string>& path, EPathAnchor anchor,
                                              bool is_silent_error)
{
  assert(!path.empty());

  const auto symid = module::resolve_path_symbol(ctx, path, anchor, id);
  if (symid) return symid;

  if (!is_silent_error) add_error(165, n, "Symbol definition not found!", "");
  return NO_ID;
}

void resolver::Symbol::resolve_node(ast::Node& node)
{
  resolve_node(node.nodeid);
}

void resolver::Symbol::resolve_node(ast::ID nodeid)
{
#define resolve(kind)                                                                                                  \
  case ast::ENodeKind::kind: resolve_##kind(*nodeid.as<ast::kind>()); break;

  switch (nodeid.get()->kind()) {
    resolve(Identifier);
    resolve(ID_Qualified);
    resolve(ID_Typed);
    resolve(Expression_Call);
    resolve(Statement_GoTo);
  default: break;
  }

#undef resolve
}


size_t resolver::Symbol::start_resolver()
{
  for (const auto& node : CU.nodes->nodes) {
    resolve_node(*node);
  }

  return sym_resolved_count;
}


void resolver::Symbol::resolve_Identifier(ast::Identifier& n)
{
  // std::cout << "find sym: \"" << n.name << "\""
  //           << "\n"; /*endl*/
  if (compiler::resolved.is_resolved(n.nodeid)) return;

  auto sym = resolve_id_sym(n.scpid, n, n.name, false);
  assert(sym && "Resolution failed");
  if (!sym) return;

  compiler::resolved.add(n.nodeid, sym);
}
void resolver::Symbol::resolve_ID_Qualified(ast::ID_Qualified& n)
{
  if (compiler::resolved.is_resolved(n.nodeid)) return;

  auto sym = resolve_path_sym(n.scpid.module(), n, n.name, n.path, n.anchor);
  assert(sym && "Resolution failed");
  if (!sym) return;

  std::cout << "Symbol found: " << magic_enum::enum_name(sym.get().nodeid.get()->kind()) << ": " << sym.get().get_name()
            << "\n";

  compiler::resolved.add(n.nodeid, sym);
}
void resolver::Symbol::resolve_ID_Typed(ast::ID_Typed& n)
{
  if (compiler::resolved.is_resolved(n.nodeid)) return;

  if (const auto* id = n.name.as<ast::Identifier>()) {
    auto sym = resolve_id_sym(n.scpid, n, id->name);
    assert(sym && "Resolution failed");
    if (!sym) return;

    compiler::resolved.add(n.nodeid, sym);
  } else if (const auto* id_quali = n.name.as<ast::ID_Qualified>()) {
    auto sym = resolve_path_sym(n.scpid.module(), n, id_quali->name, id_quali->path, id_quali->anchor, false);
    assert(sym && "Resolution failed");
    if (!sym) return;

    compiler::resolved.add(n.nodeid, sym);
  } else {
    assert(false);
  }
}

void resolver::Symbol::resolve_Expression_Call(ast::Expression_Call& n)
{
  auto* n_callee = n.callee.get();
  if (!ast::ENodeKind_is_ID(n_callee->kind())) {
    add_error(196, n, "The callee is not an indentifier", "");
    return;
  }

  resolve_node(n.callee);
  compiler::resolved.add(n.nodeid, n.callee.symbol());
}

void resolver::Symbol::resolve_Statement_GoTo(ast::Statement_GoTo& n)
{
  /*
  if (auto sym = CU.sym_m->current_scope->find_symbol(n.label)) {
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
