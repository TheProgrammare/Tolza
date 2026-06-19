
#include "resolver_inference.hpp"

#include <cassert>

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"

#include "nexus/forward.hpp"
#include "nexus/scope.hpp"
#include "nexus/type/type.hpp"
#include "nexus/inference.hpp"
#include "compiler/compilation_unit.hpp"

#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"


void resolver::Inference::Inference::add_inference(ast::Node& n, type::ID type) const
{
  assert(n.nodeid && "Invalid node");
  assert(type && "Invalid type");
  compiler::inference.add(n.nodeid, type);
}

void resolver::Inference::Inference::resolve_Node(ast::ID n, bool mandatory)
{
  if (!n) return;
  resolve_Node(*n.get(), mandatory);
}


void resolver::Inference::Inference::resolve_Node(ast::Node& n, bool mandatory)
{
#define resolve(_kind)                                                                                                 \
  case ast::ENodeKind::_kind: resolve_##_kind(*n.nodeid.as<ast::_kind>()); break

  const auto kind = n.kind();

  switch (kind) {
    resolve(Identifier);
    resolve(ID_Qualified);
    resolve(ID_Typed);
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
    resolve(Local_Parameter);
    resolve(Literal_Boolean);
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
    resolve(Literal_Iterator);
    resolve(Literal_Enum);
    resolve(Literal_Structured_Data);
    resolve(Literal_Form);
    resolve(Expression_If_Ternary);
    resolve(Expression_Member_Access);
    resolve(Expression_Self);
    resolve(Expression_Other);
    resolve(Expression_Call);
    resolve(Expression_Call_Argument);
    resolve(Expression_Call_Rule);
    resolve(Expression_Call_Pipe);
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
    resolve(Statement_For);
    resolve(Statement_Return);
    resolve(Operation_Is);
    resolve(Operation_In);
    resolve(Operation_Assignment);
    resolve(Operation_Binary);
    resolve(Operation_Unary);
    resolve(Operation_Interval);
  default: return; // assert(!mandatory);
  }

#undef resolve
}


void resolver::Inference::ensure_expression_resolution(ast::Node& p_expr, type::ID p_type_inference)
{
  // standalone expression inference
  if (!p_type_inference) {
    resolve_Node(p_expr);
    return;
  }

  if (ast::ENodeKind_is_literal(p_expr.kind()) && is_lazy_literal(p_expr)) {
    // primitive literal can be lazy type inference
    if (const auto* ty = p_type_inference.as<type::Primitive>()) {
      if (p_expr.kind() == ast::ENodeKind::Literal_Integral && !type::EPrimitiveTypeKind_is_integral(ty->primitive))
        goto bad_inference;
      if (p_expr.kind() == ast::ENodeKind::Literal_Floating_Point
          && !type::EPrimitiveTypeKind_is_floating(ty->primitive))
        goto bad_inference;

      if (p_expr.kind() == ast::ENodeKind::Literal_Fixed_Point && !type::EPrimitiveTypeKind_is_fixed(ty->primitive))
        goto bad_inference;
    } else if (auto* ty = p_type_inference.as<type::String>()) {
      if (p_expr.kind() != ast::ENodeKind::Literal_Text_Pure) goto bad_inference;
    } else
      goto bad_inference;

    add_inference(p_expr, p_type_inference);
  } else {
    resolve_Node(p_expr);
  }

  // no type to infer
  if (!p_type_inference) return;

  if (!p_expr.nodeid.is_inferred()) {
    add_error(234, p_expr, "Impossible to define the expression type.", "");
    return;
  }

bad_inference:
  if (!p_type_inference) return;

  // check if type inferred is compatible to the expected inference (or expected type)
  if (p_expr.nodeid.type() != p_type_inference) {
    if (type::ETypeKind_is_user_defined(p_type_inference.get().kind())) {
      const auto* n = p_type_inference.symbol().node().get();
      add_error_two_nodes(230, p_expr, *n, "Illegal type inference.", "");
    } else {
      add_error(230, p_expr,
                "Illegal type inference \"" + p_expr.nodeid.type().hex() + "\" as \"" + p_type_inference.hex() + "\".",
                "");
    }
  }
}

bool resolver::Inference::is_lazy_literal(ast::Node& n) const
{
  if (const auto* ptr = n.nodeid.as<ast::Literal_Integral>()) return ptr->type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = n.nodeid.as<ast::Literal_Floating_Point>()) return ptr->type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = n.nodeid.as<ast::Literal_Fixed_Point>()) return ptr->raw_type == type::EPrimitiveTypeKind::NONE;
  if (const auto* ptr = n.nodeid.as<ast::Literal_Text_Pure>()) return ptr->text_type == type::ETextType::NONE;

  return false;
}


bool resolver::Inference::start_resolver()
{
  for (const auto& n : CU.nodes->nodes) {
    resolve_Node(*n);
  }

  return true;
}


void resolver::Inference::resolve_Identifier(ast::Identifier& n)
{
  if (n.nodeid.is_inferred()) return;
  auto n_sym = n.nodeid.symbol().node();
  resolve_Node(n_sym);
  assert(ast::ENodeKind_is_declaration(n_sym.get()->kind()) && "the node must be a declaration");
  add_inference(n, n_sym.type());
}
void resolver::Inference::resolve_ID_Qualified(ast::ID_Qualified& n)
{
  if (n.nodeid.is_inferred()) return;

  auto n_sym = n.nodeid.symbol().node();
  resolve_Node(n_sym);
  assert(ast::ENodeKind_is_declaration(n_sym.get()->kind()) && "the node must be a declaration");
  add_inference(n, n_sym.type());
}
void resolver::Inference::resolve_ID_Typed(ast::ID_Typed& n)
{
  if (n.nodeid.is_inferred()) return;

  auto n_sym = n.nodeid.symbol().node();
  resolve_Node(n_sym);
  assert(ast::ENodeKind_is_declaration(n_sym.get()->kind()) && "the node must be a declaration");
  add_inference(n, n_sym.type());
}

// expression inferred type;
void resolver::Inference::resolve_Local_Pattern_Enum(ast::Local_Pattern_Enum& n)
{
  /*
    if (n.id.is_inferred()) return;

  resolve_Node(n.expression);

  auto& expr_ty = get_type(n.expression);

  if (expr_ty.kind != type::ETypeKind::Enum) {
    add_error(156, get_node(n.expression), "Expected enum type.", "");
    return;
  }

  auto& enum_data = std::get<type::EnumData>(expr_ty.data);
  auto& enum_sym  = expr_ty.sym.get();
  auto  enum_decl = enum_sym.nodeid.as<ast::Global_Enum>();

  auto enum_target = compiler::COMPILER.resolved.get_symbol(make_id(get_node(n.name)));


  if (enum_sym.id != enum_target) {
    add_error_two_nodes(245, *enum_decl, get_node(n.name),
                        "Different enum used in pattern between name and expression type.", "");
    return;
  }

  size_t max_payload = 0;
  size_t min_payload = -1;

  for (auto elem : enum_data.variants) {
    max_payload = elem.size() > max_payload ? max_payload = elem.size() : max_payload;
    min_payload = elem.size() < min_payload ? min_payload = elem.size() : min_payload;
  }

  if (max_payload < n.pattern_elements.size())
    add_error_two_nodes(243, get_node(n.expression), *enum_decl,
                        "Too much pattern elements specified for any enum variants.", "");
  else if (min_payload > n.pattern_elements.size())
    add_error_two_nodes(244, n, *enum_decl, "Too few pattern elements specified for any enum variants.", "");

  size_t count = 0;
  for (auto elem : n.pattern_elements) {
    resolve_Node(elem);

    auto n_elem = elem.as<ast::Local_Pattern_Element>();
    assert(!n_elem);

    switch (n_elem->kind) {
    case ast::Local_Pattern_Element::Kind::Ignore:  count++; continue;
    case ast::Local_Pattern_Element::Kind::Binding: {
      resolve_Node(n_elem->bind);
      ensure_expression_resolution(get_node(n_elem->bind, enum_data.variants[0]),
    }
    case ast::Local_Pattern_Element::Kind::Literal: break;
    }
    if (n_elem->kind) auto& elem_ty = get_type(elem);

    if (elem_ty !=) }

  if (auto ptr = std::dynamic_pointer_cast<ast::Enum_Element>(n.name->expression_inferred_type->resolve())) {
    if (n.mapping.size() != ptr->types.size()) {
      add_error_two_nodes(155, n, *ptr, "Inequal types binded on " + n.name->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty;
        }
      }
    }
  }
  add_error(156, *n.name, "Unexpected reference encounted, expected Enum element", "");
  */
}
void resolver::Inference::resolve_Local_Pattern_Tuple(ast::Local_Pattern_Tuple& n)
{
  /*
    if (n.id.is_inferred()) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::type::Tuple>(n.right->expression_inferred_type->resolve())) {
    if (n.mapping.size() != ptr->types.size()) {
      add_error_two_nodes(157, n, *ptr, "Inequal types binded on " + ptr->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty;
        }
      }
    }
  } else {
    add_error(158, *n.right, "Expected tuple type", "");
  }
    */
}
void resolver::Inference::resolve_Local_Pattern_Form(ast::Local_Pattern_Form& n)
{
  /*
    if (n.id.is_inferred()) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::sfm::Form>(n.name->expression_inferred_type->resolve())) {
    for (const auto& pat_facet : n.mapping) {
      bool found = false;
      for (const auto& et_facet : ptr->facets) {
        if (et_facet->name == pat_facet->name) {
          pat_facet->expression_inferred_type = et_facet->expression_inferred_type->resolve();
          found                              = true;
          break;
        }
      }

      if (!found) add_error(159, *pat_facet, "Facet is not in the form composition", "");
    }
  } else {
    add_error(160, *n.name, "Expected form type", "");
  }
    */
}
void resolver::Inference::resolve_Local_Pattern_Facet(ast::Local_Pattern_Facet& n)
{
  /*
    if (n.id.is_inferred()) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::sfm::Facet>(n.name->expression_inferred_type)) {
    for (const auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (const auto& facet_field : ptr->fields) {
        if (field_name == facet_field->declaration_name) {

          if (pattern->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern->node())) {
            map_ptr->type = facet_field->type;
          }

          found = true;
          break;
        }
      }

      if (!found) add_error(161, *n.name, "Field \"" + field_name + "\" not in the facet definition", "");
    }
  } else {
    add_error(162, *n.name, "Expected facet type", "");
  }
    */
}

void resolver::Inference::resolve_Local_Pattern_Rule_Facet(ast::Local_Pattern_Rule_Facet& n)
{
}


void resolver::Inference::resolve_Global_Variable(ast::Global_Variable& n)
{
  if (n.nodeid.is_inferred()) return;

  if (!n.type) {
    assert(n.expression && "Expression must be valid to infer type");
    resolve_Node(n.expression);
    n.type = n.expression.type();
  } else if (n.expression) {
    ensure_expression_resolution(*n.expression.get(), n.type);
  }

  add_inference(n, n.type);
}

void resolver::Inference::resolve_Global_Function(ast::Global_Function& n)
{
  if (n.nodeid.is_inferred()) return;

  auto* proto = n.prototype.as<type::Prototype>();

  if (n.name == "main") {
    if (!proto->ret || proto->ret == type::TYPEID_u0) {
      proto->ret = type::TYPEID_u0;
    } else if (proto->ret != type::TYPEID_s32) {
      add_error(218, n, "Violation of the main function convention, main must return i32 or u0 type.", "");
    }
  }

  // potential function return type inference
  if (proto->ret == type::TYPEID_u0 && !n.is_explicit_ret_type) {
    type::ID return_ty;

    const auto* cb = n.codeblock.as<ast::CodeBlock>();

    for (const auto& elem : cb->elements) {
      if (elem.as<ast::Statement_Return>()) {
        resolve_Node(elem);
        return_ty = elem.type();
        continue;
      }
    }

    proto->ret = return_ty;
  }

  add_inference(n, proto->ret);
}

void resolver::Inference::resolve_Global_Alias_Type(ast::Global_Alias_Type& n)
{
  if (n.type) {
    add_inference(n, n.type);
  }
  // it's a opaque type !
  else {
    // u0 is a opaque type by convention
    add_inference(n, type::TYPEID_u0);
  }
}


void resolver::Inference::resolve_Local_Variable(ast::Local_Variable& n)
{
  if (n.nodeid.is_inferred()) return;

  if (!n.type) {
    resolve_Node(n.expression);
    n.type = n.expression.type();
  } else {
    ensure_expression_resolution(*n.expression.get(), n.type);
  }

  add_inference(n, n.type);
}
void resolver::Inference::resolve_Local_Parameter(ast::Local_Parameter& n)
{
  if (n.nodeid.is_inferred()) return;

  add_inference(n, n.type);
}
void resolver::Inference::resolve_Local_Binding(ast::Local_Binding& n)
{
  if (n.nodeid.is_inferred()) return;

  if (!n.type) {
    resolve_Node(n.expression);
    n.type = n.expression.type();
  } else {
    ensure_expression_resolution(*n.expression.get(), n.type);
  }

  add_inference(n, n.type);
}

void resolver::Inference::resolve_Statement_Return(ast::Statement_Return& n)
{
  if (n.nodeid.is_inferred()) return;

  auto check_ret = [&](ast::ID nodeid) {
    resolve_Node(*nodeid.get());

    if (!n.value && nodeid.type() == type::TYPEID_u0) return;
    if (!n.value && nodeid.type() != type::TYPEID_u0) {
      add_error_two_nodes(246, n, *nodeid.get(), "Invalid void return, expression needed, the function returns values.",
                          "");
      return;
    }

    ensure_expression_resolution(*n.value.get(), nodeid.type());
    add_inference(n, nodeid.type());
  };

  if (auto fn_id = scope::get_scope_node(ast::ENodeKind::Global_Function, n.scpid)) {
    check_ret(fn_id);
  } else if (auto lam_id = scope::get_scope_node(ast::ENodeKind::Local_Lambda, n.scpid)) {
    check_ret(lam_id);
  } else if (auto rule_id = scope::get_scope_node(ast::ENodeKind::SFM_Rule, n.scpid)) {
    check_ret(rule_id);
  } else
    assert(false);
}


void resolver::Inference::resolve_Expression_If_Ternary(ast::Expression_If_Ternary& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.evaluator);
  resolve_Node(n.statement_true);
  resolve_Node(n.statement_false);

  ensure_expression_resolution(*n.left.get(), n.statement_true.type());
  if (n.statement_false) ensure_expression_resolution(*n.left.get(), n.statement_false.type());

  add_inference(n, n.statement_true.type());
}

void resolver::Inference::resolve_Expression_Member_Access(ast::Expression_Member_Access& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.left_expression);
  resolve_Node(n.right_identifier);

  add_inference(n, n.right_identifier.type());
}
void resolver::Inference::resolve_Expression_Self(ast::Expression_Self& n)
{
  if (n.nodeid.is_inferred()) return;

  auto form_id = scope::get_scope_node(ast::ENodeKind::SFM_Form, n.scpid);
  assert(form_id);

  auto* form = form_id.as<ast::SFM_Form>();
  assert(!form && "Must be form");

  resolve_Node(*form);

  add_inference(n, form_id.type());
}
void resolver::Inference::resolve_Expression_Other(ast::Expression_Other& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Call_Argument(ast::Expression_Call_Argument& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Call(ast::Expression_Call& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.callee);

  // to build
}
void resolver::Inference::resolve_Expression_Call_Pipe(ast::Expression_Call_Pipe& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Call_Rule(ast::Expression_Call_Rule& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Table_Access(ast::Expression_Table_Access& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Ptr_Val(ast::Expression_Ptr_Val& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Expression_Mut_Of(ast::Expression_Mut_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Ref_Of(ast::Expression_Ref_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Addr_Of(ast::Expression_Addr_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Size_Of(ast::Expression_Size_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_GetBits(ast::Expression_GetBits& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Move_Of(ast::Expression_Move_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Copy_Of(ast::Expression_Copy_Of& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_Get_Type(ast::Expression_Get_Type& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.target);
  add_inference(n, n.target.type());
}
void resolver::Inference::resolve_Expression_New_Ptr(ast::Expression_New_Ptr& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Boolean(ast::Literal_Boolean& n)
{
  if (n.nodeid.is_inferred()) return;

  add_inference(n, type::TYPEID_bool);
}

void resolver::Inference::resolve_Literal_Integral(ast::Literal_Integral& n)
{
  if (n.nodeid.is_inferred()) return;

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::_ssize: ensure_expression_resolution(n, type::TYPEID_ssize); break;
  case type::EPrimitiveTypeKind::_s8:    ensure_expression_resolution(n, type::TYPEID_s8); break;
  case type::EPrimitiveTypeKind::_s16:   ensure_expression_resolution(n, type::TYPEID_s16); break;
  case type::EPrimitiveTypeKind::_s32:   ensure_expression_resolution(n, type::TYPEID_s32); break;
  case type::EPrimitiveTypeKind::_s64:   ensure_expression_resolution(n, type::TYPEID_s64); break;
  case type::EPrimitiveTypeKind::_s128:  ensure_expression_resolution(n, type::TYPEID_s128); break;
  case type::EPrimitiveTypeKind::_usize: ensure_expression_resolution(n, type::TYPEID_usize); break;
  case type::EPrimitiveTypeKind::_u8:    ensure_expression_resolution(n, type::TYPEID_u8); break;
  case type::EPrimitiveTypeKind::_u16:   ensure_expression_resolution(n, type::TYPEID_u16); break;
  case type::EPrimitiveTypeKind::_u32:   ensure_expression_resolution(n, type::TYPEID_u32); break;
  case type::EPrimitiveTypeKind::_u64:   ensure_expression_resolution(n, type::TYPEID_u64); break;
  case type::EPrimitiveTypeKind::_u128:  ensure_expression_resolution(n, type::TYPEID_u128); break;
  default:                               assert(false);
  }
}
void resolver::Inference::resolve_Literal_Floating_Point(ast::Literal_Floating_Point& n)
{
  if (n.nodeid.is_inferred()) return;

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::_fsize: ensure_expression_resolution(n, type::TYPEID_fsize); break;
  case type::EPrimitiveTypeKind::_f16:   ensure_expression_resolution(n, type::TYPEID_f16); break;
  case type::EPrimitiveTypeKind::_f32:   ensure_expression_resolution(n, type::TYPEID_f32); break;
  case type::EPrimitiveTypeKind::_f64:   ensure_expression_resolution(n, type::TYPEID_f64); break;
  case type::EPrimitiveTypeKind::_f80:   ensure_expression_resolution(n, type::TYPEID_f80); break;
  case type::EPrimitiveTypeKind::_f128:  ensure_expression_resolution(n, type::TYPEID_f128); break;
  default:                               assert(false);
  }
}
void resolver::Inference::resolve_Literal_Fixed_Point(ast::Literal_Fixed_Point& n)
{
  if (n.nodeid.is_inferred()) return;

  switch (n.raw_type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::_dsize:  ensure_expression_resolution(n, type::TYPEID_dsize); break;
  case type::EPrimitiveTypeKind::_d32:    ensure_expression_resolution(n, type::TYPEID_d32); break;
  case type::EPrimitiveTypeKind::_d64:    ensure_expression_resolution(n, type::TYPEID_d64); break;
  case type::EPrimitiveTypeKind::_d128:   ensure_expression_resolution(n, type::TYPEID_d128); break;
  case type::EPrimitiveTypeKind::_udsize: ensure_expression_resolution(n, type::TYPEID_udsize); break;
  case type::EPrimitiveTypeKind::_ud32:   ensure_expression_resolution(n, type::TYPEID_ud32); break;
  case type::EPrimitiveTypeKind::_ud64:   ensure_expression_resolution(n, type::TYPEID_ud64); break;
  case type::EPrimitiveTypeKind::_ud128:  ensure_expression_resolution(n, type::TYPEID_ud128); break;
  default:                                assert(false);
  }
}
void resolver::Inference::resolve_Literal_Cune(ast::Literal_Cune& n)
{
  if (n.nodeid.is_inferred()) return;

  add_inference(n, type::TYPEID_cune);
}
void resolver::Inference::resolve_Literal_Rune(ast::Literal_Rune& n)
{
  if (n.nodeid.is_inferred()) return;

  add_inference(n, type::TYPEID_rune);
}

void resolver::Inference::resolve_Literal_Table(ast::Literal_Table& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Map(ast::Literal_Map& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Text_Interpolation(ast::Literal_Text_Interpolation& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Text_Pure(ast::Literal_Text_Pure& n)
{
  if (n.nodeid.is_inferred()) return;

  switch (n.text_type) {
  case type::ETextType::_c_str: ensure_expression_resolution(n, type::TYPEID_cune); break;
  case type::ETextType::_str:   ensure_expression_resolution(n, type::TYPEID_str); break;
  case type::ETextType::_text:  ensure_expression_resolution(n, type::TYPEID_text); break;
  default:                      assert(false);
  }
}
void resolver::Inference::resolve_Literal_Textual_Format(ast::Literal_Textual_Format& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Enum(ast::Literal_Enum& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Tuple(ast::Literal_Tuple& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Range(ast::Literal_Range& n)
{
  if (n.nodeid.is_inferred()) return;

  ensure_expression_resolution(*n.start.get(), n.end.type());
  ensure_expression_resolution(*n.end.get(), n.start.type());
}
void resolver::Inference::resolve_Literal_Structured_Data(ast::Literal_Structured_Data& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Form(ast::Literal_Form& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Literal_Iterator(ast::Literal_Iterator& n)
{
  if (n.nodeid.is_inferred()) return;
}


void resolver::Inference::resolve_Statement_For(ast::Statement_For& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.expression);

  if (n.index) ensure_expression_resolution(*n.index.get(), n.expression.type());
  for (auto& elem : n.items) ensure_expression_resolution(*elem.get(), n.expression.type());
}


void resolver::Inference::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
  if (n.nodeid.is_inferred()) return;

  assert(n.type);

  ensure_expression_resolution(*n.expression.get(), n.type);
}
void resolver::Inference::resolve_Operation_Is(ast::Operation_Is& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Operation_In(ast::Operation_In& n)
{
  if (n.nodeid.is_inferred()) return;
}
void resolver::Inference::resolve_Operation_Assignment(ast::Operation_Assignment& n)
{
  if (n.nodeid.is_inferred()) return;

  resolve_Node(n.left);

  // variable type inference
  ensure_expression_resolution(*n.right.get(), n.left.type());
  ensure_expression_resolution(n, n.left.type());
}
void resolver::Inference::resolve_Operation_Binary(ast::Operation_Binary& n)
{
  if (n.nodeid.is_inferred()) return;

  if (EBinOpType_is_logical(n.op_ty)) {
    ensure_expression_resolution(*n.left.get(), type::TYPEID_bool);
    ensure_expression_resolution(*n.right.get(), type::TYPEID_bool);

    add_inference(n, type::TYPEID_bool);
    return;
  }


  ensure_expression_resolution(*n.left.get(), n.right.type());
  ensure_expression_resolution(*n.right.get(), n.left.type());


  if (EBinOpType_is_comparison(n.op_ty)) {
    add_inference(n, type::TYPEID_bool);
    return;
  }

  if (n.op_ty == ast::EBinOpType::_div) {
    // div always return a floating point
    add_inference(n, type::TYPEID_fsize);
    return;
  }

  add_inference(n, n.left.type());
}
void resolver::Inference::resolve_Operation_Unary(ast::Operation_Unary& n)
{
  if (n.nodeid.is_inferred()) return;

  if (n.unary_op == ast::EUnaryOpType::_not) {
    ensure_expression_resolution(*n.base.get(), type::TYPEID_bool);
    add_inference(n, type::TYPEID_bool);
    return;
  }

  resolve_Node(n.base);
  ensure_expression_resolution(n, n.base.type());
}
void resolver::Inference::resolve_Operation_Interval(ast::Operation_Interval& n)
{
  if (n.nodeid.is_inferred()) return;
}
