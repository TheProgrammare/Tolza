
#include "resolver_inference.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/scope.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"
#include "nexus/type/rule.hpp"
#include "nexus/type/type.hpp"

#include <cassert>

#define INFERENCE_GUARD                                                                                                \
  if (n.nodeid().is_inferred()) return;


compiler::EPhase resolver::Inference::current_EPhase() const
{
  return compiler::EPhase::resolver_type;
}


void resolver::Inference::Inference::add_inference(ast::ID nodeid, type::ID type)
{
  assert(nodeid && "Invalid node");
  assert(type && "Invalid type");

  const auto canon = type.canonical();
  compiler::inference.add(nodeid, canon);
  inference_count++;
}

void resolver::Inference::Inference::resolve_node(ast::ID nodeid, bool mandatory)
{
  if (!nodeid) return;

#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*nodeid.as<ast::_kind>()); break

  switch (nodeid.kind()) {
    resolve(Symbol_Id);
    resolve(Symbol_Qualified);
    resolve(Symbol_Type);
    resolve(Global_Variable);
    resolve(Global_Function);
    resolve(Global_Alias_Type);
    resolve(Local_Pattern_Enum);
    resolve(Local_Pattern_Tuple);
    resolve(Local_Pattern_Form);
    resolve(Local_Pattern_Rule_Facet);
    resolve(Local_Pattern_Facet);
    resolve(Local_Binding);
    resolve(Local_Variable);
    resolve(Local_Capability);
    resolve(Local_Parameter);
    resolve(Literal_Boolean);
    resolve(Literal_NullPtr);
    resolve(Literal_Integral);
    resolve(Literal_Fixed_Point);
    resolve(Literal_Floating_Point);
    resolve(Literal_Cune);
    resolve(Literal_Rune);
    resolve(Literal_Text_Pure);
    resolve(Literal_Text_Interpolation);
    resolve(Literal_Textual_Format);
    resolve(Literal_Table);
    resolve(Literal_Map);
    resolve(Literal_Tuple);
    resolve(Literal_Range);
    resolve(Literal_Record);
    resolve(Expression_If_Ternary);
    resolve(Expression_Member_Access);
    resolve(Expression_Self);
    resolve(Expression_Other);
    resolve(Expression_Invocation);
    resolve(Expression_Invocation_Arg);
    resolve(Expression_Invocation_Rule);
    resolve(Expression_Invocation_Extend);
    resolve(Expression_Table_Access);
    resolve(Expression_Ptr_Val);
    resolve(Expression_Mut_Of);
    resolve(Expression_Ref_Of);
    resolve(Expression_Move_Of);
    resolve(Expression_Copy_Of);
    resolve(Expression_Addr_Of);
    resolve(Expression_Size_Of);
    resolve(Expression_GetBits);
    resolve(Expression_New_Ptr);
    resolve(Expression_Get_Type);
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
    resolve(Operation_Is);
    resolve(Operation_In);
    resolve(Operation_Transfert);
    resolve(Operation_Binary);
    resolve(Operation_Unary);
    resolve(Operation_Interval);
    resolve(Operation_Cast_As);
    resolve(Global_Export);
    resolve(Global_Extern);
    resolve(Root);
    resolve(CodeBlock);
  case ast::ENodeKind::Import:
  case ast::ENodeKind::Path_Regex:
  case ast::ENodeKind::Global_Reexport: break;
  default:                              assert(false && "Unhandled node resolution");
  }

#undef resolve
#undef resolve_mut
}

void resolver::Inference::ensure_primitive_literal(ast::ID lit, type::ID ty_inference)
{
  if (const auto* inf_primitive = ty_inference.as<type::Primitive>()) {
    if (auto* ptr = lit.as<ast::Literal_Integral>()) {
      ptr->type = inf_primitive->primitive;
    } else if (auto* ptr = lit.as<ast::Literal_Floating_Point>()) {
      ptr->type = inf_primitive->primitive;
    } else if (auto* ptr = lit.as<ast::Literal_Fixed_Point>()) {
      ptr->raw_type = inf_primitive->primitive;
    } else if (lit.as<ast::Literal_Cune>() || lit.as<ast::Literal_Rune>()) {
      // ignore
    } else if (auto* ptr = lit.as<ast::Literal_Text_Pure>()) {
      switch (inf_primitive->primitive) {
      case type::EPrimitiveTypeKind::_cune: ptr->text_type = type::ETextType::_cune; break;
      case type::EPrimitiveTypeKind::_rune: ptr->text_type = type::ETextType::_rune; break;
      default:                              assert(false && "Illegal type inference reached");
      }
    } else {
      assert(false && "Illegal type inference reached");
    }
  } else if (const auto* inf_text = ty_inference.as<type::String>()) {
    if (auto* ptr = lit.as<ast::Literal_Text_Pure>()) {
      assert(inf_text->kind != type::ETextType::NONE);
      ptr->text_type = inf_text->kind;
    } else {
      assert(false && "Illegal type inference reached");
    }

  } else if (const auto* inf_tbl = ty_inference.as<type::Array>()) {
    if (auto* ptr = lit.as<ast::Literal_Table>()) {
      ptr->type = inf_tbl->tyid();
    } else
      assert(false && "Illegal type inference reached");
  } else if (const auto* inf_tbl = ty_inference.as<type::Buffer>()) {
    if (auto* ptr = lit.as<ast::Literal_Table>()) {
      ptr->type = inf_tbl->tyid();
    } else
      assert(false && "Illegal type inference reached");
  } else if (const auto* inf_tbl = ty_inference.as<type::Slice>()) {
    if (auto* ptr = lit.as<ast::Literal_Table>()) {
      ptr->type = inf_tbl->tyid();
    } else
      assert(false && "Illegal type inference reached");
  } else {
    assert(false && "Illegal type inference reached");
  }
}


void resolver::Inference::ensure_expression_resolution(ast::ID expr_nodeid, type::ID ty_inference, bool silent_error)
{
  // standalone expression inference
  if (!ty_inference) {
    resolve_node(expr_nodeid);
    return;
  }

  ty_inference = ty_inference.canonical();

  if (ast::ENodeKind_is_literal(expr_nodeid.kind()) && is_lazy_literal(expr_nodeid)) {
    // primitive literal can be lazy type inference
    if (const auto* ty = ty_inference.as<type::Primitive>()) {
      const bool bad_integral = expr_nodeid.kind() == ast::ENodeKind::Literal_Integral
                                && !type::EPrimitiveTypeKind_is_integral(ty->primitive);
      const bool bad_floating = expr_nodeid.kind() == ast::ENodeKind::Literal_Floating_Point
                                && !type::EPrimitiveTypeKind_is_floating(ty->primitive);
      const bool bad_fixed = expr_nodeid.kind() == ast::ENodeKind::Literal_Fixed_Point
                             && !type::EPrimitiveTypeKind_is_fixed(ty->primitive);
      const bool bad_cune =
          expr_nodeid.kind() == ast::ENodeKind::Literal_Cune && ty->primitive != type::EPrimitiveTypeKind::_cune;
      const bool bad_rune =
          expr_nodeid.kind() == ast::ENodeKind::Literal_Rune && ty->primitive != type::EPrimitiveTypeKind::_rune;

      if (bad_integral && bad_floating && bad_fixed && bad_cune && bad_rune) goto bad_inference;

    } else if (auto* ty = ty_inference.as<type::String>()) {
      if (expr_nodeid.kind() != ast::ENodeKind::Literal_Text_Pure) goto bad_inference;
    } else if (auto* ty = ty_inference.as<type::Array>()) {
      if (expr_nodeid.kind() != ast::ENodeKind::Literal_Table) goto bad_inference;
    } else if (auto* ty = ty_inference.as<type::Buffer>()) {
      if (expr_nodeid.kind() != ast::ENodeKind::Literal_Table) goto bad_inference;
    } else if (auto* ty = ty_inference.as<type::Slice>()) {
      if (expr_nodeid.kind() != ast::ENodeKind::Literal_Table) goto bad_inference;
    } else {
      goto bad_inference;
    }

    add_inference(expr_nodeid, ty_inference);
    ensure_primitive_literal(expr_nodeid, ty_inference);
  } else {
    resolve_node(expr_nodeid);
  }

  // no type to infer
  if (!ty_inference) return;

  if (!expr_nodeid.is_inferred() && !silent_error) {
    add_error(234, expr_nodeid.get(), "Impossible to define the expression type.", "");
    return;
  }

  {
  bad_inference:
    if (!ty_inference) return;

    // check if type inferred is compatible to the expected inference (or expected type
    if (expr_nodeid.type() != ty_inference && !silent_error) {
      if (type::ETypeKind_is_user_defined(ty_inference.kind())) {
        const auto n = ty_inference.def().node().get();
        add_error_two_nodes(230, expr_nodeid.get(), n, "Illegal type inference.", "");
      } else {
        if (!expr_nodeid.type()) {
          add_error(230, expr_nodeid.get(),
                    std::format("Expression type undefined, inference tried on \"{}\".", ty_inference.dump()), "");
        } else if (!type::rule::can_implicit_cast(expr_nodeid.type(), ty_inference)) {
          add_error(230, expr_nodeid.get(),
                    std::format("Illegal type inference \"{}\" as \"", expr_nodeid.type().dump()) + ty_inference.dump()
                        + "\".",
                    "");
        }
      }
    }
  }
}

bool resolver::Inference::is_lazy_literal(ast::ID nodeid) const
{
  if (const auto* ptr = nodeid.as<ast::Literal_Integral>()) return ptr->type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = nodeid.as<ast::Literal_Floating_Point>()) return ptr->type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = nodeid.as<ast::Literal_Fixed_Point>()) return ptr->raw_type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = nodeid.as<ast::Literal_Text_Pure>()) return ptr->text_type == type::ETextType::NONE;
  if (const auto* ptr = nodeid.as<ast::Literal_Table>()) return !ptr->type;

  return false;
}


size_t resolver::Inference::start_resolver()
{
  resolve_node(CU.ast->get_file_root()->nodeid());

  return inference_count;
}

void resolver::Inference::resolve_Root(const ast::Root& n)
{
  for (auto& elem : n.global_nodes) resolve_node(elem);
}
void resolver::Inference::resolve_CodeBlock(const ast::CodeBlock& n)
{
  for (auto& elem : n.elements) resolve_node(elem);
}

void resolver::Inference::resolve_Global_Export(const ast::Global_Export& n)
{
  resolve_node(n.codeblock);
}
void resolver::Inference::resolve_Global_Extern(const ast::Global_Extern& n)
{
  resolve_node(n.codeblock);
}

void resolver::Inference::resolve_Symbol_Id(const ast::Symbol_Id& n)
{
  INFERENCE_GUARD
  auto def_node = n.nodeid().def().node();
  resolve_node(def_node);
  assert(ast::ENodeKind_is_declaration(def_node.kind()) && "the node must be a declaration");
  add_inference(n.nodeid(), def_node.type());
}
void resolver::Inference::resolve_Symbol_Qualified(const ast::Symbol_Qualified& n)
{
  INFERENCE_GUARD

  auto def_node = n.nodeid().def().node();
  resolve_node(def_node);
  assert(ast::ENodeKind_is_declaration(def_node.kind()) && "the node must be a declaration");
  add_inference(n.nodeid(), def_node.type());
}
void resolver::Inference::resolve_Symbol_Type(const ast::Symbol_Type& n)
{
  INFERENCE_GUARD

  auto def_node = n.nodeid().def().node();
  resolve_node(def_node);
  assert(ast::ENodeKind_is_declaration(def_node.kind()) && "the node must be a declaration");
  add_inference(n.nodeid(), def_node.type());
}

// expression inferred type;
void resolver::Inference::resolve_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n)
{
}
void resolver::Inference::resolve_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n)
{
}
void resolver::Inference::resolve_Local_Pattern_Form(const ast::Local_Pattern_Form& n)
{
}
void resolver::Inference::resolve_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n)
{
}

void resolver::Inference::resolve_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n)
{
}


void resolver::Inference::resolve_Global_Variable(const ast::Global_Variable& n)
{
  INFERENCE_GUARD

  if (!n.type) {
    assert(n.expression && "Expression must be valid to infer type");
    resolve_node(n.expression);
    add_inference(n.nodeid(), n.expression.type());
    return;
  }

  if (n.expression) ensure_expression_resolution(n.expression, n.type);

  add_inference(n.nodeid(), n.type);
}

void resolver::Inference::resolve_Global_Function(const ast::Global_Function& n)
{
  INFERENCE_GUARD

  const auto* proto = n.prototype.as<type::Prototype>();

  for (const auto& param : n.parameters) resolve_node(param);

  if (n.name == "main") {
    if (!proto->ret || (proto->ret != type::TYPEID_s32 && proto->ret != type::TYPEID_u0)) {
      add_error(218, n.header, "Violation of the main function convention, main must return i32 or u0 type.", "");
    }
  }

  if (proto->ret)
    add_inference(n.nodeid(), proto->ret);
  else
    add_inference(n.nodeid(), type::TYPEID_u0);

  resolve_node(n.codeblock);
}

void resolver::Inference::resolve_Global_Alias_Type(const ast::Global_Alias_Type& n)
{
  add_inference(n.nodeid(), n.type);
}


void resolver::Inference::resolve_Local_Variable(const ast::Local_Variable& n)
{
  INFERENCE_GUARD

  if (!n.type) {
    resolve_node(n.expression);
    add_inference(n.nodeid(), n.expression.type());
    return;
  }

  if (n.expression) ensure_expression_resolution(n.expression, n.type);

  add_inference(n.nodeid(), n.type);
}
void resolver::Inference::resolve_Local_Capability(const ast::Local_Capability& n)
{
  INFERENCE_GUARD

  if (!n.type) {
    resolve_node(n.expression);
    add_inference(n.nodeid(), n.expression.type());
    return;
  }

  if (n.expression) ensure_expression_resolution(n.expression, n.type);

  add_inference(n.nodeid(), n.type);
}

void resolver::Inference::resolve_Local_Parameter(const ast::Local_Parameter& n)
{
  INFERENCE_GUARD

  if (n.default_value) ensure_expression_resolution(n.default_value, n.type);

  add_inference(n.nodeid(), n.type);
}
void resolver::Inference::resolve_Local_Binding(const ast::Local_Binding& n)
{
  INFERENCE_GUARD

  if (!n.type) {
    resolve_node(n.expression);
  } else if (n.expression) {
    ensure_expression_resolution(n.expression, n.type);
    add_inference(n.nodeid(), n.type);
  }
}


void resolver::Inference::resolve_Expression_If_Ternary(const ast::Expression_If_Ternary& n)
{
  INFERENCE_GUARD

  resolve_node(n.evaluator);
  resolve_node(n.statement_true);
  resolve_node(n.statement_false);

  ensure_expression_resolution(n.left, n.statement_true.type());
  if (n.statement_false) ensure_expression_resolution(n.left, n.statement_false.type());

  add_inference(n.nodeid(), n.statement_true.type());
}

void resolver::Inference::resolve_Expression_Member_Access(const ast::Expression_Member_Access& n)
{
  INFERENCE_GUARD

  resolve_node(n.left_expression);
  resolve_node(n.right_identifier);

  add_inference(n.nodeid(), n.right_identifier.type());
}
void resolver::Inference::resolve_Expression_Self(const ast::Expression_Self& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);

  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Other(const ast::Expression_Other& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);

  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n)
{
  INFERENCE_GUARD

  resolve_node(n.expression);

  add_inference(n.nodeid(), n.expression.type());
}
void resolver::Inference::resolve_Expression_Invocation(const ast::Expression_Invocation& n)
{
  INFERENCE_GUARD

  resolve_node(n.callee);
  assert(n.callee.type());
  const auto* proto = ast::get_prototype(n.callee.def().node());
  assert(proto);

  for (size_t i = 0; i < proto->params.size(); i++) {
    if (n.arguments.size() <= i) break;
    const auto& param = proto->params[i];
    const auto* arg   = n.arguments[i].as<ast::Expression_Invocation_Arg>();
    assert(arg);
    ensure_expression_resolution(arg->expression, param.type);
  }

  if (proto->is_variadic && proto->params.size() < n.arguments.size()) {
    for (size_t i = proto->params.size(); i < n.arguments.size(); i++) {
      resolve_node(n.arguments[i]);
    }
  }

  add_inference(n.nodeid(), n.callee.type());
}

void resolver::Inference::resolve_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Expression_Table_Access(const ast::Expression_Table_Access& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  resolve_node(n.selector);

  assert(n.target.type());

  type::ID tyid;

  if (auto* ptr = n.target.type().as<type::Ptr>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Array>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Buffer>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Slice>()) {
    tyid = ptr->inner;
  }

  assert(tyid);

  add_inference(n.nodeid(), tyid);
}
void resolver::Inference::resolve_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);

  type::ID tyid;

  if (auto* ptr = n.target.type().as<type::Ptr>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Array>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Buffer>()) {
    tyid = ptr->inner;
  } else if (auto* ptr = n.target.type().as<type::Slice>()) {
    tyid = ptr->inner;
  }

  assert(tyid);

  add_inference(n.nodeid(), tyid);
}
void resolver::Inference::resolve_Expression_Mut_Of(const ast::Expression_Mut_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Ref_Of(const ast::Expression_Ref_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Addr_Of(const ast::Expression_Addr_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  auto tyid = CU.types->factory.make_ptr(n.target.type());
  add_inference(n.nodeid(), tyid);
}
void resolver::Inference::resolve_Expression_Size_Of(const ast::Expression_Size_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_GetBits(const ast::Expression_GetBits& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Move_Of(const ast::Expression_Move_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Copy_Of(const ast::Expression_Copy_Of& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_Get_Type(const ast::Expression_Get_Type& n)
{
  INFERENCE_GUARD

  resolve_node(n.target);
  add_inference(n.nodeid(), n.target.type());
}
void resolver::Inference::resolve_Expression_New_Ptr(const ast::Expression_New_Ptr& n)
{
  INFERENCE_GUARD
}

void resolver::Inference::resolve_Statement_If(const ast::Statement_If& n)
{
  if (!n.is_else) ensure_expression_resolution(n.evaluator, type::TYPEID_bool);
  resolve_node(n.codeblock);
  if (!n.is_else) resolve_node(n.alternative_statement);
}
void resolver::Inference::resolve_Statement_For(const ast::Statement_For& n)
{
  resolve_node(n.expression);

  if (const auto* ptr = n.expression.as<ast::Literal_Range>()) {
    add_inference(n.index, ptr->start.type());
  } else if (!type::ETypeKind_is_iterable(n.expression.type().kind()))
    add_error(272, n.expression.get(), std::format("The type \"{}\" is not iterable.", n.expression.type().dump()), "");
  else {
    if (n.index) add_inference(n.index, type::get_inner(n.expression.type()));
    for (auto it : n.items) add_inference(it, type::get_inner(n.expression.type()));
  }
  resolve_node(n.codeblock);
}
void resolver::Inference::resolve_Statement_Loop(const ast::Statement_Loop& n)
{
  resolve_node(n.codeblock);
}
void resolver::Inference::resolve_Statement_While(const ast::Statement_While& n)
{
  ensure_expression_resolution(n.evaluator, type::TYPEID_bool);
  resolve_node(n.codeblock);
}
void resolver::Inference::resolve_Statement_GoTo(const ast::Statement_GoTo& n)
{
}
void resolver::Inference::resolve_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n)
{
  resolve_node(n.codeblock);
}
void resolver::Inference::resolve_Statement_Return(const ast::Statement_Return& n)
{
  INFERENCE_GUARD

  resolve_node(n.returnable);

  // return void
  if (!n.value) {
    if (n.returnable.type() != type::TYPEID_u0)
      add_error_two_nodes(246, n.header, n.returnable.get(),
                          "Invalid void return, expression needed, the function returns values.", "");

    add_inference(n.nodeid(), n.returnable.type());
    return;
  }

  ensure_expression_resolution(n.value, n.returnable.type());

  add_inference(n.nodeid(), n.returnable.type());
}
void resolver::Inference::resolve_Statement_Break(const ast::Statement_Break& n)
{
}
void resolver::Inference::resolve_Statement_Continue(const ast::Statement_Continue& n)
{
}
void resolver::Inference::resolve_Statement_Match(const ast::Statement_Match& n)
{
  ensure_expression_resolution(n.base, type::TYPEID_bool);

  for (auto& elem : n.cases) resolve_node(elem);
  resolve_node(n.other_case);
}
void resolver::Inference::resolve_Statement_Match_Case(const ast::Statement_Match_Case& n)
{
  resolve_node(n.codeblock);
}

void resolver::Inference::resolve_Literal_Boolean(const ast::Literal_Boolean& n)
{
  INFERENCE_GUARD

  add_inference(n.nodeid(), type::TYPEID_bool);
}

void resolver::Inference::resolve_Literal_NullPtr(const ast::Literal_NullPtr& n)
{
  INFERENCE_GUARD

  add_inference(n.nodeid(), type::TYPEID_opaque);
}

void resolver::Inference::resolve_Literal_Integral(ast::Literal_Integral& n)
{
  INFERENCE_GUARD

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:   n.type = type::EPrimitiveTypeKind::_ssize;
  case type::EPrimitiveTypeKind::_ssize: add_inference(n.nodeid(), type::TYPEID_ssize); break;
  case type::EPrimitiveTypeKind::_s8:    add_inference(n.nodeid(), type::TYPEID_s8); break;
  case type::EPrimitiveTypeKind::_s16:   add_inference(n.nodeid(), type::TYPEID_s16); break;
  case type::EPrimitiveTypeKind::_s32:   add_inference(n.nodeid(), type::TYPEID_s32); break;
  case type::EPrimitiveTypeKind::_s64:   add_inference(n.nodeid(), type::TYPEID_s64); break;
  case type::EPrimitiveTypeKind::_s128:  add_inference(n.nodeid(), type::TYPEID_s128); break;
  case type::EPrimitiveTypeKind::_usize: add_inference(n.nodeid(), type::TYPEID_usize); break;
  case type::EPrimitiveTypeKind::_u8:    add_inference(n.nodeid(), type::TYPEID_u8); break;
  case type::EPrimitiveTypeKind::_u16:   add_inference(n.nodeid(), type::TYPEID_u16); break;
  case type::EPrimitiveTypeKind::_u32:   add_inference(n.nodeid(), type::TYPEID_u32); break;
  case type::EPrimitiveTypeKind::_u64:   add_inference(n.nodeid(), type::TYPEID_u64); break;
  case type::EPrimitiveTypeKind::_u128:  add_inference(n.nodeid(), type::TYPEID_u128); break;
  default:                               assert(false);
  }
}
void resolver::Inference::resolve_Literal_Floating_Point(ast::Literal_Floating_Point& n)
{
  INFERENCE_GUARD

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:   n.type = type::EPrimitiveTypeKind::_fsize;
  case type::EPrimitiveTypeKind::_fsize: add_inference(n.nodeid(), type::TYPEID_fsize); break;
  case type::EPrimitiveTypeKind::_f16:   add_inference(n.nodeid(), type::TYPEID_f16); break;
  case type::EPrimitiveTypeKind::_f32:   add_inference(n.nodeid(), type::TYPEID_f32); break;
  case type::EPrimitiveTypeKind::_f64:   add_inference(n.nodeid(), type::TYPEID_f64); break;
  case type::EPrimitiveTypeKind::_f80:   add_inference(n.nodeid(), type::TYPEID_f80); break;
  case type::EPrimitiveTypeKind::_f128:  add_inference(n.nodeid(), type::TYPEID_f128); break;
  default:                               assert(false);
  }
}
void resolver::Inference::resolve_Literal_Fixed_Point(ast::Literal_Fixed_Point& n)
{
  INFERENCE_GUARD

  switch (n.raw_type) {
  case type::EPrimitiveTypeKind::NONE:    n.raw_type = type::EPrimitiveTypeKind::_dsize;
  case type::EPrimitiveTypeKind::_dsize:  add_inference(n.nodeid(), type::TYPEID_dsize); break;
  case type::EPrimitiveTypeKind::_d32:    add_inference(n.nodeid(), type::TYPEID_d32); break;
  case type::EPrimitiveTypeKind::_d64:    add_inference(n.nodeid(), type::TYPEID_d64); break;
  case type::EPrimitiveTypeKind::_d128:   add_inference(n.nodeid(), type::TYPEID_d128); break;
  case type::EPrimitiveTypeKind::_udsize: add_inference(n.nodeid(), type::TYPEID_udsize); break;
  case type::EPrimitiveTypeKind::_ud32:   add_inference(n.nodeid(), type::TYPEID_ud32); break;
  case type::EPrimitiveTypeKind::_ud64:   add_inference(n.nodeid(), type::TYPEID_ud64); break;
  case type::EPrimitiveTypeKind::_ud128:  add_inference(n.nodeid(), type::TYPEID_ud128); break;
  default:                                assert(false);
  }
}
void resolver::Inference::resolve_Literal_Cune(const ast::Literal_Cune& n)
{
  INFERENCE_GUARD

  add_inference(n.nodeid(), type::TYPEID_cune);
}
void resolver::Inference::resolve_Literal_Rune(const ast::Literal_Rune& n)
{
  INFERENCE_GUARD

  add_inference(n.nodeid(), type::TYPEID_rune);
}

void resolver::Inference::resolve_Literal_Table(const ast::Literal_Table& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Literal_Map(const ast::Literal_Map& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Literal_Text_Pure(const ast::Literal_Text_Pure& n)
{
  INFERENCE_GUARD

  switch (n.text_type) {
  case type::ETextType::_cstr: add_inference(n.nodeid(), type::TYPEID_cstr); return;
  case type::ETextType::NONE:
  case type::ETextType::_cune:
  case type::ETextType::_str:  add_inference(n.nodeid(), type::TYPEID_str); return;
  case type::ETextType::_rune:
  case type::ETextType::_text: add_inference(n.nodeid(), type::TYPEID_text); return;
  }
}
void resolver::Inference::resolve_Literal_Textual_Format(const ast::Literal_Textual_Format& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Literal_Tuple(const ast::Literal_Tuple& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Literal_Range(const ast::Literal_Range& n)
{
  INFERENCE_GUARD

  resolve_node(n.end);
  ensure_expression_resolution(n.start, n.end.type());

  add_inference(n.nodeid(), n.end.type());
}
void resolver::Inference::resolve_Literal_Record(const ast::Literal_Record& n)
{
  INFERENCE_GUARD
}


void resolver::Inference::resolve_Operation_Cast_As(const ast::Operation_Cast_As& n)
{
  INFERENCE_GUARD

  assert(n.type);

  ensure_expression_resolution(n.expression, n.type, true);
  if (!n.expression.is_inferred()) resolve_node(n.expression); // must have a inference even if silent error

  switch (n.cast_type) {
  case ast::Operation_Cast_As::ECastType::AS: {

    if (!type::rule::can_explicit_cast(n.expression.type(), n.type.canonical())) {
      add_error(267, n.header,
                std::format("Invalid explicit cast `{}` as `{}`", n.expression.type().dump(), n.type.dump()), "");
    }
    break;
  }
  case ast::Operation_Cast_As::ECastType::AS_REINTERPRET:
  case ast::Operation_Cast_As::ECastType::AS_SAFE:        break;
  }


  add_inference(n.nodeid(), n.type);
}
void resolver::Inference::resolve_Operation_Is(const ast::Operation_Is& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Operation_In(const ast::Operation_In& n)
{
  INFERENCE_GUARD
}
void resolver::Inference::resolve_Operation_Transfert(const ast::Operation_Transfert& n)
{
  INFERENCE_GUARD

  resolve_node(n.left);

  // variable type inference
  ensure_expression_resolution(n.right, n.left.type());
  add_inference(n.nodeid(), n.left.type());
}
void resolver::Inference::resolve_Operation_Binary(const ast::Operation_Binary& n)
{
  INFERENCE_GUARD

  if (EOp_Bin_is_logical(n.op_ty)) {
    ensure_expression_resolution(n.left, type::TYPEID_bool);
    ensure_expression_resolution(n.right, type::TYPEID_bool);

    add_inference(n.nodeid(), type::TYPEID_bool);
    return;
  }


  ensure_expression_resolution(n.left, n.right.type());
  ensure_expression_resolution(n.right, n.left.type());


  if (EOp_Bin_is_comparison(n.op_ty)) {
    add_inference(n.nodeid(), type::TYPEID_bool);
    return;
  }

  if (n.op_ty == ast::EOp_Bin::_div) {
    // div always return a floating point
    add_inference(n.nodeid(), type::TYPEID_fsize);
    return;
  }

  add_inference(n.nodeid(), n.left.type());
}
void resolver::Inference::resolve_Operation_Unary(const ast::Operation_Unary& n)
{
  INFERENCE_GUARD

  if (n.unary_op == ast::EOp_Unary::_not) {
    ensure_expression_resolution(n.base, type::TYPEID_bool);
    add_inference(n.nodeid(), type::TYPEID_bool);
    return;
  }

  resolve_node(n.base);
  add_inference(n.nodeid(), n.base.type());
}
void resolver::Inference::resolve_Operation_Interval(const ast::Operation_Interval& n)
{
  INFERENCE_GUARD
}
