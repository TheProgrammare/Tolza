
#include "resolver_semantic.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_statement.hpp"
#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/module.hpp"
#include "nexus/type/rule.hpp"
#include "nexus/type/type.hpp"

#include "compiler/compilation_unit.hpp"
#include "nexus/definition.hpp"
#include <cstddef>
#include <vector>

bool resolver::Semantic::start_resolver()
{
  resolve_node(*CU.nodes->get_file_root());

  return true;
}

void resolver::Semantic::resolve_node(ast::ID nodeid)
{

  const auto kind = nodeid.kind();

#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*nodeid.as<ast::_kind>()); break;

  switch (kind) {
    resolve(Global_Function);
    resolve(Operation_Binary);
    resolve(Operation_Cast_As);
    resolve(Expression_Invocation);
  default: break;
  }

#undef resolve
}

void resolver::Semantic::resolve_node(ast::Node& n)
{
  resolve_node(n.nodeid);
}

void resolver::Semantic::resolve_Root(ast::Root& n)
{
  for (auto& elem : n.global_nodes) resolve_node(elem);
}
void resolver::Semantic::resolve_CodeBlock(ast::CodeBlock& n)
{
  for (auto& elem : n.elements) resolve_node(elem);
}

void resolver::Semantic::resolve_Statement_If(ast::Statement_If& n)
{
  resolve_node(n.evaluator);
  resolve_node(n.codeblock);
  resolve_node(n.alternative_statement);
}
void resolver::Semantic::resolve_Statement_For(ast::Statement_For& n)
{
  resolve_node(n.expression);
  resolve_node(n.index);
  resolve_node(n.codeblock);
}
void resolver::Semantic::resolve_Statement_Loop(ast::Statement_Loop& n)
{
  resolve_node(n.codeblock);
}
void resolver::Semantic::resolve_Statement_While(ast::Statement_While& n)
{
  resolve_node(n.evaluator);
  resolve_node(n.codeblock);
}
void resolver::Semantic::resolve_Statement_GoTo(ast::Statement_GoTo& n)
{
}
void resolver::Semantic::resolve_Statement_GoTo_Label(ast::Statement_GoTo_Label& n)
{
  resolve_node(n.codeblock);
}
void resolver::Semantic::resolve_Statement_Return(ast::Statement_Return& n)
{
  if (n.value) resolve_node(n.value);
}
void resolver::Semantic::resolve_Statement_Break(ast::Statement_Break& n)
{
}
void resolver::Semantic::resolve_Statement_Continue(ast::Statement_Continue& n)
{
}
void resolver::Semantic::resolve_Statement_Match(ast::Statement_Match& n)
{
  resolve_node(n.base);
  for (auto& elem : n.cases) resolve_node(elem);
  resolve_node(n.other_case);
}
void resolver::Semantic::resolve_Statement_Match_Case(ast::Statement_Match_Case& n)
{
  resolve_node(n.codeblock);
}


void resolver::Semantic::resolve_Global_Function(ast::Global_Function& n)
{
  if (n.name == "main") {
    if (n.nodeid.module() != CU.modules->get_file_root().modid)
      add_error(215, n, "Illegal function reserved name 'main'. Or your main function musn't be scoped.", "");
    if (n.nodeid.module().get().visibility == EVisibility::Cross_File_Scope)
      add_error(216, n, "Illegal function reserved name 'main'. Or your main function musn't be exported.", "");
    if (!n.extern_abi.empty())
      add_error(217, n, "Illegal function reserved name 'main'. Or your main function musn't be external.", "");
  }

  resolve_node(n.codeblock);
}


void resolver::Semantic::resolve_Operation_Binary(ast::Operation_Binary& n)
{
  resolve_node(n.left);
  resolve_node(n.right);

  if (n.left.type() != n.right.type()) {
    add_error(205, n,
              "Invalid binary operation on two differents types:\n  - left type: \"" + n.left.type().dump()
                  + "\"\n  - right type: \"" + n.right.type().dump() + "\"",
              "");
    return;
  }

  if (const auto* prim = n.left.type().as<type::Primitive>()) {
    if (!type::rule::can_op_primitive(prim->primitive, n.op_ty)) {
      add_error(206, n,
                "Invalid operation \"" + std::string(EOp_Bin_to_str(n.op_ty)) + "\" on " + n.left.type().dump()
                    + " type.",
                "");
      return;
    }
  }
}

void resolver::Semantic::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
}

void resolver::Semantic::resolve_Expression_Invocation(ast::Expression_Invocation& n)
{
  const auto  decl_nodeid = n.nodeid.def().node();
  const auto  params      = ast::get_parameters(decl_nodeid);
  const auto* proto       = type::get_prototype(decl_nodeid).as<type::Prototype>();

  size_t count = 0;
  for (auto paramid : params) {
    // no more type verification in variadic
    if (proto->is_variadic && count >= n.arguments.size()) break;

    const auto* param = paramid.as<ast::Local_Parameter>();

    // more parameters than arguments check if next params are optionals
    if (count >= n.arguments.size()) {
      auto it = params.begin() + count;
      for (; it < params.end(); ++it) {
        const auto* next_param = it->as<ast::Local_Parameter>();
        if (!next_param->default_value)
          add_error_two_nodes(
              212, n, *next_param,
              "Mandatory parameter ignored, not enough arguments passed on the call compared to the signature.", "");
      }

      break;
    }

    const auto argid = n.arguments[count++];

    if (argid.type() != param->type) add_error_two_nodes(214, *argid.get(), *param, "Invalid argument type", "");
  }
}
