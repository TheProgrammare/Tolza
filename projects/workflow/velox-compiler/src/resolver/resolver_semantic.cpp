
#include "resolver_semantic.hpp"

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/module.hpp"
#include "nexus/type/type.hpp"

#include "compiler/compilation_unit.hpp"
#include "nexus/symbol.hpp"
#include "rules/rule_type.hpp"
#include <cstddef>
#include <vector>

bool resolver::Semantic::start_resolver()
{
#define resolve(kind)                                                                                                  \
  case ast::ENodeKind::kind: resolve_##kind(*node->nodeid.as<ast::kind>()); break;

  for (const auto& node : CU.nodes->nodes) {
    switch (node->kind()) {
      resolve(Global_Function);
      resolve(Operation_Binary);
      resolve(Operation_Cast_As);
      resolve(Expression_Call);
    default: break;
    }
  }

#undef resolve

  return true;
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
}


void resolver::Semantic::resolve_Operation_Binary(ast::Operation_Binary& n)
{
  if (n.left.type() != n.right.type()) {
    add_error(205, n,
              "Invalid binary operation on two differents types:\n  - left type: \""
                  + n.left.type().get().velox_codegen() + "\"\n  - right type: \""
                  + n.right.type().get().velox_codegen() + "\"",
              "");
    return;
  }

  if (const auto* prim = n.left.type().as<type::Primitive>()) {
    if (!rule::type::can_op_primitive(prim->primitive, n.op_ty)) {
      add_error(206, n,
                "Invalid operation \"" + std::string(EBinOpType_to_str(n.op_ty)) + "\" on " + prim->velox_codegen()
                    + " type.",
                "");
      return;
    }
  }
}

void resolver::Semantic::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
}

void resolver::Semantic::resolve_Expression_Call(ast::Expression_Call& n)
{
  const auto  decl_nodeid = n.nodeid.symbol().node();
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
