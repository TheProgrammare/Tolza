
#include "resolver/semantic.hpp"

#include "ast/data.hpp"
#include "ast/dumper.hpp"
#include "ast/forward.hpp"
#include "ast/node/base.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/literal.hpp"
#include "ast/node/operation.hpp"
#include "ast/node/statement.hpp"
#include "ast/pool.hpp"
#include "ast/tool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "nexus/forward.hpp"
#include "pool/node_to_metadata.hpp"
#include "type/dumper.hpp"
#include "type/pool.hpp"
#include "type/rule.hpp"
#include "type/type.hpp"

#include <cassert>
#include <cstddef>
#include <format>
#include <vector>

bool resolver::Semantic::start_resolver()
{
  resolve_node(CU.ast->get_file_root()->nodeid());

  return true;
}

semantic::Metadata& resolver::Semantic::add_metadata(ast::ID nodeid, semantic::Metadata& metadata)
{
  assert(nodeid);
  assert(&metadata);
  auto& m = COMPILER.semantic_metadata.add(nodeid);
  m       = metadata;
  return m;
}
semantic::Metadata& resolver::Semantic::add_metadata(ast::ID nodeid, semantic::Metadata&& metadata)
{
  assert(nodeid);
  auto& m = COMPILER.semantic_metadata.add(nodeid);
  m       = metadata;
  return m;
}


void resolver::Semantic::resolve_node(ast::ID nodeid)
{
  const auto kind = nodeid.kind();

#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*nodeid.as<ast::_kind>()); break;
#define ignore(_kind)                                                                                                  \
  case ast::ENodeKind::_kind: break;

  switch (kind) {
    resolve(Root);
    resolve(CodeBlock);
    resolve(Global_Export);
    resolve(Global_Extern);
    resolve(Global_Function);
    resolve(Global_Variable);
    resolve(Local_Variable);
    resolve(Operation_Binary);
    resolve(Operation_Cast_As);
    resolve(Expression_Invocation);
    resolve(Expression_Invocation_Arg);
    resolve(Expression_Member_Access);
    resolve(Statement_If);
    resolve(Statement_For);
    resolve(Statement_Loop);
    resolve(Statement_While);
    resolve(Statement_GoTo);
    resolve(Statement_GoTo_Label);
    resolve(Statement_Return);
    resolve(Statement_Break);
    resolve(Statement_Continue);
    resolve(Statement_Match);
    resolve(Statement_Match_Case);
    resolve(Literal_Range);
    resolve(Literal_Table);

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
    ignore(Symbol_Id);
    ignore(Symbol_Qualified);
  default: assert(false && "Unhandled node resolution");
  }

#undef resolve
}

void resolver::Semantic::resolve_Root(ast::Root& n)
{
  for (auto& elem : n.global_nodes) resolve_node(elem);
}
void resolver::Semantic::resolve_CodeBlock(ast::CodeBlock& n)
{
  for (auto& elem : n.elements) resolve_node(elem);
}
void resolver::Semantic::resolve_Global_Export(ast::Global_Export& n)
{
  resolve_node(n.codeblock);
}
void resolver::Semantic::resolve_Global_Extern(ast::Global_Extern& n)
{
  resolve_node(n.codeblock);
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

void resolver::Semantic::resolve_Literal_Range(ast::Literal_Range& n)
{
  if (n.start) resolve_node(n.start);
  if (n.end) resolve_node(n.end);
}

void resolver::Semantic::resolve_Literal_Table(ast::Literal_Table& n)
{
  if (ast::is_table_population(n.nodeid())) {
  }
}

void resolver::Semantic::resolve_Global_Function(ast::Global_Function& n)
{
  if (n.name == "main") {
    if (n.nodeid().module() != CU.modules->get_file_root().modid)
      add_error(215, n.header, "Illegal function reserved name 'main'. Or your main function musn't be scoped.", "");
    if (n.nodeid().module().get().visibility == ast::EVisibility::Cross_File_Scope)
      add_error(216, n.header, "Illegal function reserved name 'main'. Or your main function musn't be exported.", "");
    if (!n.extern_abi.empty())
      add_error(217, n.header, "Illegal function reserved name 'main'. Or your main function musn't be external.", "");
  }

  if (n.codeblock) resolve_node(n.codeblock);
  if (n.contract) resolve_node(n.contract);
}

void resolver::Semantic::resolve_Call_Contract(ast::Call_Contract& n)
{
  resolve_node(n.pre);
  resolve_node(n.post);
}


void resolver::Semantic::resolve_Global_Variable(ast::Global_Variable& n)
{
  if (n.expression) resolve_node(n.expression);
}
void resolver::Semantic::resolve_Local_Variable(ast::Local_Variable& n)
{
  if (n.expression) resolve_node(n.expression);
}


void resolver::Semantic::resolve_Operation_Binary(ast::Operation_Binary& n)
{
  resolve_node(n.left);
  resolve_node(n.right);

  if (n.left.type() != n.right.type()) {
    add_error(
        205, n.header,
        std::format("Invalid binary operation on two differents types:\n  - left type: \"{}\"\n  - right type: \"{}\"",
                    dump(n.left.type()), dump(n.right.type())),
        "");
    return;
  }

  if (const auto* prim = n.left.type().as<type::Primitive>()) {
    if (!type::rule::can_op_primitive(prim->primitive, n.op_ty)) {
      add_error(206, n.header,
                std::format(R"(Invalid operation "{}" on "{}" type.)", EOp_Bin_to_str(n.op_ty), dump(n.left.type())),
                "");
      return;
    }
  }
}

void resolver::Semantic::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
  resolve_node(n.expression);
}

void resolver::Semantic::resolve_Expression_Invocation(ast::Expression_Invocation& n)
{
  const auto  decl   = n.nodeid().def().node();
  const auto  params = ast::get_parameters(decl);
  const auto* proto  = ast::get_prototype(decl);
  assert(proto && "Invalid type id");

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
              212, n.header, next_param->header,
              "Mandatory parameter ignored, not enough arguments passed on the call compared to the signature.", "");
      }

      break;
    }

    const auto argid = n.arguments[count++];

    if (argid.type() != param->type) add_error_two_nodes(214, argid.get(), param->header, "Invalid argument type", "");

    resolve_node(argid);
    add_metadata(argid, semantic::Metadata{.param_def = paramid});
  }

  if (proto->is_variadic) {
    for (size_t i = params.size(); i < n.arguments.size(); i++) {
      auto argid = n.arguments[i];
      resolve_node(argid);
      add_metadata(argid, semantic::Metadata{.variadic_arg = true});
    }
  } else if (params.size() < n.arguments.size()) {
    add_error_two_nodes(215, n.header, n.arguments[params.size()].get(), "Too many arguments specified", "");
  }
}

void resolver::Semantic::resolve_Expression_Invocation_Arg(ast::Expression_Invocation_Arg& n)
{
  resolve_node(n.expression);
}


void resolver::Semantic::resolve_Expression_Member_Access(ast::Expression_Member_Access& n)
{
  resolve_node(n.left_expression);

  auto defid = n.left_expression.def().node();

  if (auto* def = defid.as<ast::SFM_Facet>()) {
    if (auto* id = n.right_identifier.as<ast::Symbol_Id>()) {
      size_t pos = 0;
      for (auto fid : def->fields) {
        auto* f = fid.as<ast::SFM_Facet_Field>();
        if (id->name == f->name) {
          add_metadata(n.right_identifier, semantic::Metadata{.member_position = pos});
          break;
        }
        pos++;
      }
    }
  } else if (auto* arr = defid.type().as<type::Array>()) {
    if (n.right_identifier.def().node() == ast::NODEID_BUILTIN_Member_Access_Data)
      add_metadata(n.right_identifier, semantic::Metadata{.member_position = 0});
    else if (n.right_identifier.def().node() == ast::NODEID_BUILTIN_Member_Access_Len)
      add_metadata(n.right_identifier, semantic::Metadata{.member_position = 1});
    else if (n.right_identifier.def().node() == ast::NODEID_BUILTIN_Member_Access_Capa)
      add_metadata(n.right_identifier, semantic::Metadata{.member_position = 2});
  }
}
