#include "resolver_symbol.hpp"

#include "ast/data.hpp"
#include "ast/definition/ast_base.hpp"
#include "ast/definition/ast_declaration_extension.hpp"
#include "ast/definition/ast_declaration_global.hpp"
#include "ast/definition/ast_declaration_local.hpp"
#include "ast/definition/ast_declaration_sfm.hpp"
#include "ast/definition/ast_expression.hpp"
#include "ast/definition/ast_literal.hpp"
#include "ast/definition/ast_operation.hpp"
#include "ast/definition/ast_statement.hpp"
#include "ast/forward.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "pool/ast.hpp"
#include "pool/link/definition.hpp"
#include "pool/link/resolved.hpp"
#include "pool/module.hpp"
#include "pool/scope.hpp"
#include "pool/type.hpp"
#include "type/definition.hpp"

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>

#define RESOLUTION_GUARD                                                                                               \
  if (n.nodeid().is_resolved()) return;

void resolver::Symbol::add_resolution(ast::ID nodeid, definition::ID defid)
{
  assert(defid && "invalid symbol id");

  COMPILER.resolved.add(nodeid, defid);
}


definition::ID resolver::Symbol::resolve_id_sym(scope::ID ctx, ast::ID nodeid, std::string_view id,
                                                bool is_silent_error)
{
  assert(!id.empty());

  if (auto sym = scope::find_in_chain_scope(ctx, id)) {
    sym_resolved_count++;
    return sym;
  }

  if (!is_silent_error) add_error(165, nodeid.get(), "Symbol definition not found!", "");
  return NO_ID;
}
definition::ID resolver::Symbol::resolve_path_sym(module::ID ctx, ast::ID nodeid, std::string_view id,
                                                  const std::vector<std::string>& path, EPathAnchor anchor,
                                                  bool is_silent_error)
{
  assert(!path.empty());

  const auto defid = module::resolve_path_symbol(ctx, path, anchor, id);
  if (defid) return defid;

  if (!is_silent_error) add_error(165, nodeid.get(), "Symbol definition not found!", "");
  return NO_ID;
}


void resolver::Symbol::resolve_node(ast::ID nodeid)
{
#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*nodeid.as<ast::_kind>()); break;
#define ignore(_kind)                                                                                                  \
  case ast::ENodeKind::_kind: break;

  auto kind = nodeid.kind();

  switch (kind) {

    resolve(Root);
    resolve(CodeBlock);
    resolve(Global_Export);
    resolve(Global_Extern);
    resolve(Symbol_Id);
    resolve(Symbol_Qualified);
    resolve(Symbol_Type);
    resolve(Global_Function);
    resolve(Global_Variable);
    resolve(Call_Contract);
    resolve(Global_Extend_Fn);
    resolve(SFM_Rule);
    resolve(Local_Lambda);
    resolve(Local_Parameter);
    resolve(Local_Variable);
    resolve(Expression_Invocation);
    resolve(Expression_Invocation_Arg);
    resolve(Expression_Member_Access);
    resolve(Statement_GoTo);
    resolve(Literal_Record);
    resolve(Literal_Range);
    resolve(Statement_If);
    resolve(Statement_For);
    resolve(Statement_Loop);
    resolve(Statement_While);
    resolve(Statement_GoTo_Label);
    resolve(Statement_Return);
    resolve(Statement_Break);
    resolve(Statement_Continue);
    resolve(Statement_Match);
    resolve(Statement_Match_Case);
    resolve(Expression_Ptr_Val);
    resolve(Operation_Cast_As);
    ignore(Import);
    ignore(Global_Alias_Type);
    ignore(Global_Alias_Module);
    ignore(Literal_Boolean);
    ignore(Literal_NullPtr);
    ignore(Literal_Integral);
    ignore(Literal_Fixed_Point);
    ignore(Literal_Floating_Point);
    ignore(Literal_Cune);
    ignore(Literal_Rune);
    ignore(Literal_Text_Pure);
  default: assert(false && "Unhandled node resolution");
  }

#undef resolve
}


size_t resolver::Symbol::start_resolver()
{
  size_t count = 0;
  resolve_Root(*CU.ast->get_file_root());

  ensure_types_symbols();

  return sym_resolved_count;
}


void resolver::Symbol::ensure_types_symbols()
{
  for (const auto& elem : CU.types->types) {
    const auto tyid = elem.id;

    if (const auto* array = tyid.as<type::Array>()) {
      if (array->size_expression) {
        resolve_node(array->size_expression);
        if (auto def = array->size_expression.def()) {

          auto n = def.node();

          if (auto* glo = n.as<ast::Global_Variable>()) {
            if (glo->kind != ast::EVariableKind::_const) {
              add_error_two_nodes(282, array->size_expression.get(), glo->header,
                                  "Illegal size definition from a runtime variable",
                                  "define a static size with a compiletime variable like: `const size = 100`");
            }
          } else if (auto* loc = n.as<ast::Local_Variable>()) {
            if (loc->kind != ast::EVariableKind::_const) {
              add_error_two_nodes(282, array->size_expression.get(), loc->header,
                                  "Illegal size definition from a runtime variable",
                                  "define a static size with a compiletime variable like: `const size = 100`");
            }
          }
        }
      }
    }
  }
}

void resolver::Symbol::resolve_Symbol_Id(ast::Symbol_Id& n)
{
  RESOLUTION_GUARD

  auto sym = resolve_id_sym(n.nodeid().scope(), n.nodeid(), n.name, false);
  if (!sym) return;

  add_resolution(n.nodeid(), sym);
}
void resolver::Symbol::resolve_Symbol_Qualified(ast::Symbol_Qualified& n)
{
  RESOLUTION_GUARD

  auto sym = resolve_path_sym(n.nodeid().module(), n.nodeid(), n.name, n.path, n.anchor);
  if (!sym) return;

  add_resolution(n.nodeid(), sym);
}
void resolver::Symbol::resolve_Symbol_Type(ast::Symbol_Type& n)
{
  RESOLUTION_GUARD

  resolve_node(n.name);
  add_resolution(n.nodeid(), n.name.def());
}


void resolver::Symbol::resolve_Root(ast::Root& n)
{
  for (auto elem : n.global_nodes) resolve_node(elem);
}
void resolver::Symbol::resolve_CodeBlock(ast::CodeBlock& n)
{
  for (auto elem : n.elements) resolve_node(elem);
}

void resolver::Symbol::resolve_Global_Function(ast::Global_Function& n)
{
  for (auto& elem : n.parameters) resolve_node(elem);
  if (n.contract) resolve_node(n.contract);
  if (n.codeblock) resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Global_Variable(ast::Global_Variable& n)
{
  if (n.expression) resolve_node(n.expression);
}
void resolver::Symbol::resolve_Call_Contract(ast::Call_Contract& n)
{
  RESOLUTION_GUARD

  resolve_node(n.pre);
  resolve_node(n.post);
}

void resolver::Symbol::resolve_Global_Extend_Fn(ast::Global_Extend_Fn& n)
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_SFM_Rule(ast::SFM_Rule& n)
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_Local_Variable(ast::Local_Variable& n)
{
  RESOLUTION_GUARD

  if (n.expression) resolve_node(n.expression);
}

void resolver::Symbol::resolve_Local_Lambda(ast::Local_Lambda& n)
{
  RESOLUTION_GUARD
}
void resolver::Symbol::resolve_Local_Parameter(ast::Local_Parameter& n)
{
  RESOLUTION_GUARD

  if (n.default_value) resolve_node(n.default_value);
}


void resolver::Symbol::resolve_Global_Export(ast::Global_Export& n)
{
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Global_Extern(ast::Global_Extern& n)
{
  resolve_node(n.codeblock);
}


void resolver::Symbol::resolve_Expression_Invocation(ast::Expression_Invocation& n)
{
  RESOLUTION_GUARD

  if (!ast::ENodeKind_is_symbol(n.callee.kind())) {
    add_error(196, n.header, "The callee is not an indentifier", "");
    return;
  }

  resolve_node(n.callee);
  auto defid = n.callee.def();

  if (!defid) return;

  if (n.arguments.empty()) {
    add_resolution(n.nodeid(), defid);
    return;
  }

  for (auto& arg : n.arguments) resolve_node(arg);

  if (defid.node().is<ast::Global_Function>() || defid.node().is<ast::SFM_Rule>()
      || defid.node().as<ast::Global_Extend_Fn>())
    n.invocation_kind = ast::EInvocationKind::fn_call;
  else if (defid.node().is<ast::Global_Enum>())
    n.invocation_kind = ast::EInvocationKind::enum_bind;


  add_resolution(n.nodeid(), defid);
}

void resolver::Symbol::resolve_Expression_Invocation_Arg(ast::Expression_Invocation_Arg& n)
{
  resolve_node(n.expression);
}

void resolver::Symbol::resolve_Expression_Member_Access(ast::Expression_Member_Access& n)
{
  RESOLUTION_GUARD
  resolve_node(n.left_expression);

  if (const auto* facet = n.left_expression.def().node().as<ast::SFM_Facet>()) {
    for (auto field_id : facet->fields) {
      assert(field_id.is<ast::SFM_Facet_Field>());
      const auto* field = field_id.as<ast::SFM_Facet_Field>();

      if (auto* id = n.right_identifier.as<ast::Symbol_Id>()) {
        if (field->name == id->name) {
          add_resolution(n.nodeid(), field_id.def());
          break;
        }
      }
    }
  }
}

void resolver::Symbol::resolve_Expression_Ptr_Val(ast::Expression_Ptr_Val& n)
{
  RESOLUTION_GUARD

  resolve_node(n.target);
}


void resolver::Symbol::resolve_Literal_Record(ast::Literal_Record& n)
{
  RESOLUTION_GUARD

  resolve_node(n.name);

  const auto& lit_def = n.name.def();
  add_resolution(n.nodeid(), lit_def);
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
      add_error_two_nodes(268, fid.get(), lit_def.node().get(),
                          std::format(R"(The field name "{}" for the facet "{}" dosen't exist.)", fname,
                                      lit_def.node().as<ast::SFM_Facet>()->name),
                          "");
      return;
    }

    add_resolution(fid, it->def());
  }
}

void resolver::Symbol::resolve_Literal_Range(ast::Literal_Range& n)
{
  if (n.start) resolve_node(n.start);
  if (n.end) resolve_node(n.end);
  if (n.step) resolve_node(n.step);
}

void resolver::Symbol::resolve_Statement_If(ast::Statement_If& n)
{
  RESOLUTION_GUARD
  resolve_node(n.evaluator);
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Statement_For(ast::Statement_For& n)
{
  RESOLUTION_GUARD
  resolve_node(n.expression);
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Statement_Loop(ast::Statement_Loop& n)
{
  RESOLUTION_GUARD
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Statement_While(ast::Statement_While& n)
{
  RESOLUTION_GUARD
  resolve_node(n.evaluator);
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Statement_GoTo(ast::Statement_GoTo& n)
{
  RESOLUTION_GUARD
  auto label = resolve_id_sym(n.nodeid().scope(), n.nodeid(), n.label);
  add_resolution(n.nodeid(), label);
}
void resolver::Symbol::resolve_Statement_GoTo_Label(ast::Statement_GoTo_Label& n)
{
  RESOLUTION_GUARD
  resolve_node(n.codeblock);
}
void resolver::Symbol::resolve_Statement_Return(ast::Statement_Return& n)
{
  RESOLUTION_GUARD
  resolve_node(n.value);
}
void resolver::Symbol::resolve_Statement_Break(ast::Statement_Break& n)
{
}
void resolver::Symbol::resolve_Statement_Continue(ast::Statement_Continue& n)
{
}
void resolver::Symbol::resolve_Statement_Match(ast::Statement_Match& n)
{
  RESOLUTION_GUARD
  resolve_node(n.base);
  for (auto _case : n.cases) resolve_node(_case);
  if (n.other_case) resolve_node(n.other_case);
}
void resolver::Symbol::resolve_Statement_Match_Case(ast::Statement_Match_Case& n)
{
  RESOLUTION_GUARD
  resolve_node(n.evaluator);
  resolve_node(n.codeblock);
}

void resolver::Symbol::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
  RESOLUTION_GUARD

  resolve_node(n.expression);
}