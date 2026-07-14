#include "resolver_symbol.hpp"

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

#include "Neargye/magic_enum.hpp"
#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_literal.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/resolved.hpp"

#include "nexus/ast/ast.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/definition.hpp"
#include "nexus/type/definition.hpp"

#define RESOLUTION_GUARD                                                                                               \
  if (n.nodeid.is_resolved()) return;

void resolver::Symbol::add_resolution(ast::Node& n, definition::ID defid) noexcept
{
  assert(defid && "invalid symbol id");

  compiler::resolved.add(n.nodeid, defid);
}


definition::ID resolver::Symbol::resolve_id_sym(scope::ID ctx, const ast::Node& n, std::string_view id,
                                                bool is_silent_error) noexcept
{
  assert(!id.empty());

  if (auto sym = scope::find_in_chain_scope(ctx, id)) {
    sym_resolved_count++;
    return sym;
  }

  if (!is_silent_error) add_error(165, n, "Symbol definition not found!", "");
  return NO_ID;
}
definition::ID resolver::Symbol::resolve_path_sym(module::ID ctx, const ast::Node& n, std::string_view id,
                                                  const std::vector<std::string>& path, EPathAnchor anchor,
                                                  bool is_silent_error) noexcept
{
  assert(!path.empty());

  const auto defid = module::resolve_path_symbol(ctx, path, anchor, id);
  if (defid) return defid;

  if (!is_silent_error) add_error(165, n, "Symbol definition not found!", "");
  return NO_ID;
}

void resolver::Symbol::resolve_node(ast::Node& node) noexcept
{
  resolve_node(node.nodeid);
}

void resolver::Symbol::resolve_node(ast::ID nodeid) noexcept
{
#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*nodeid.as<ast::_kind>()); break;

  const auto kind = nodeid.get()->kind();

  switch (kind) {
    resolve(Symbol_Id);
    resolve(Symbol_Qualified);
    resolve(Symbol_Type);
    resolve(Expression_Invocation);
    resolve(Literal_Record);
    resolve(Statement_GoTo);
    resolve(Global_Function);
    resolve(Global_Extend_Fn);
    resolve(SFM_Rule);
    resolve(Local_Lambda);

  default: break;
  }

#undef resolve
}


size_t resolver::Symbol::start_resolver() noexcept
{
  for (const auto& node : CU.nodes->nodes) {
    resolve_node(*node);
  }

  ensure_types_symbols();

  return sym_resolved_count;
}


void resolver::Symbol::ensure_types_symbols() noexcept
{
  for (const auto& elem : CU.types->types) {
    const auto* ty = elem.get();

    if (const auto* array = ty->tyid.as<type::Array>()) {
      if (array->size_expression) {
        resolve_node(array->size_expression);
        if (auto def = array->size_expression.def()) {

          auto n = def.node();

          if (auto* glo = n.as<ast::Global_Variable>()) {
            if (glo->kind != ast::EVariableKind::_const) {
              add_error_two_nodes(282, *array->size_expression.get(), *glo,
                                  "Illegal size definition from a runtime variable",
                                  "define a static size with a compiletime variable like: `const size = 100`");
            }
          } else if (auto* loc = n.as<ast::Local_Variable>()) {
            if (loc->kind != ast::EVariableKind::_const) {
              add_error_two_nodes(282, *array->size_expression.get(), *loc,
                                  "Illegal size definition from a runtime variable",
                                  "define a static size with a compiletime variable like: `const size = 100`");
            }
          }
        }
      }
    }
  }
}

void resolver::Symbol::resolve_Symbol_Id(ast::Symbol_Id& n) noexcept
{
  RESOLUTION_GUARD

  auto sym = resolve_id_sym(n.scpid, n, n.name, false);
  assert(sym && "Resolution failed");
  if (!sym) return;

  add_resolution(n, sym);
}
void resolver::Symbol::resolve_Symbol_Qualified(ast::Symbol_Qualified& n) noexcept
{
  RESOLUTION_GUARD

  auto sym = resolve_path_sym(n.scpid.module(), n, n.name, n.path, n.anchor);
  assert(sym && "Resolution failed");
  if (!sym) return;

  add_resolution(n, sym);
}
void resolver::Symbol::resolve_Symbol_Type(ast::Symbol_Type& n) noexcept
{
  RESOLUTION_GUARD

  resolve_node(n.name);
  add_resolution(n, n.name.def());
}

void resolver::Symbol::resolve_Global_Function(ast::Global_Function& n) noexcept
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_Global_Extend_Fn(ast::Global_Extend_Fn& n) noexcept
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_SFM_Rule(ast::SFM_Rule& n) noexcept
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_Local_Lambda(ast::Local_Lambda& n) noexcept
{
  RESOLUTION_GUARD
}

void resolver::Symbol::resolve_Expression_Invocation(ast::Expression_Invocation& n) noexcept
{
  RESOLUTION_GUARD

  if (!ast::ENodeKind_is_symbol(n.callee.kind())) {
    add_error(196, n, "The callee is not an indentifier", "");
    return;
  }

  resolve_node(n.callee);
  auto defid = n.callee.def();

  if (n.arguments.empty()) {
    add_resolution(n, defid);
    return;
  }

  auto resolve_params = [&](const std::vector<ast::ID>& params) {
    if (!params.empty()) {
      size_t count = 0;
      for (auto& arg : n.arguments) {
        if (count < params.size()) {
          auto defid = params[count++].def();
          add_resolution(*arg.get(), defid);
        }
      }
    }
  };

  if (auto* ptr = defid.node().as<ast::Global_Function>()) {
    n.invocation_kind = ast::EInvocationKind::fn_call;
    resolve_params(ptr->parameters);
  } else if (auto* ptr = defid.node().as<ast::SFM_Rule>()) {
    n.invocation_kind = ast::EInvocationKind::fn_call;
    resolve_params(ptr->parameters);
  } else if (auto* ptr = defid.node().as<ast::Global_Extend_Fn>()) {
    n.invocation_kind = ast::EInvocationKind::fn_call;
    resolve_params(ptr->parameters);
  } else if (auto* ptr = defid.node().as<ast::Global_Enum>()) {
    n.invocation_kind = ast::EInvocationKind::enum_bind;
    resolve_params(ptr->variants);
  }

  add_resolution(n, defid);
}

void resolver::Symbol::resolve_Statement_GoTo(ast::Statement_GoTo& n) noexcept
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

void resolver::Symbol::resolve_Literal_Record(ast::Literal_Record& n) noexcept
{
  RESOLUTION_GUARD

  resolve_node(n.name);

  const auto& lit_def = n.name.def();
  add_resolution(n, lit_def);
  const auto* def = lit_def.node().as<ast::SFM_Facet>();

  for (size_t i = 0; i < n.fields_args.size(); i++) {
    auto&       fid   = n.fields_args[i];
    const auto& fname = n.fields_names[i];

    // named field
    const auto it = std::ranges::find_if(def->fields, [&](ast::ID def_fid) -> bool {
      const auto* def_f = def_fid.as<ast::SFM_Facet_Field>();
      return def_f->name == fname;
    });
    if (it != def->fields.end()) {
      add_error_two_nodes(268, *fid.get(), *lit_def.node().get(),
                          "The field name \"" + fname + "\" for the facet \""
                              + lit_def.node().as<ast::SFM_Facet>()->name + "\" dosen't exist.",
                          "");
      return;
    }

    add_resolution(*fid.get(), it->def());
  }
}
