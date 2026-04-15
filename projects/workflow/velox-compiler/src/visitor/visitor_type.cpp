
#include "visitor_type.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>

#include "ast/ast_data.hpp"
#include "ast/ast_inferred_type_singleton.hpp"
#include "rules/rule_type.hpp"
#include "visitor_default.hpp"
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


namespace ast_loc = ast::declaration::local;

std::shared_ptr<ast::AType> Visitor_Type::get_inferred_type(ast::Node& n, bool p_is_prototype_expected) const
{
  if (auto ptr = dynamic_cast<ast::AType*>(&n))
    return ptr->resolve();
  else if (auto ptr = dynamic_cast<ast::AExpression*>(&n))
    return ptr->expression_inferred_type;
  else if (auto ptr = dynamic_cast<ast::declaration::Global*>(&n))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::Function*>(&n)) {
    if (p_is_prototype_expected) return ptr->prototype;
    return ptr->prototype->return_ty;
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(&n))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::local::Variable_Binding*>(&n))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::cop::System*>(&n))
    return ptr->prototype->return_ty;
  else if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(&n))
    return ptr->return_ty;
  else if (auto ptr = dynamic_cast<ast::declaration::local::Parameter*>(&n))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::cop::Component_Field*>(&n))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::Type_Alias*>(&n)) {
    return ptr->type;
  } else {
    add_error(168, n, "Type inferrance impossible on [" + n.debug_str() + "](" + typeid(n).name() + ")", "");
    return nullptr;
  }
}

std::shared_ptr<ast::AType> Visitor_Type::get_symbol_type(const ast::Node& n, const ast::SYM_REF p_sym_data) const
{
  if (!p_sym_data) {
    add_error(163, n, "Unresolved symbol.", "");
    return nullptr;
  }

  if (auto ty = get_inferred_type(*p_sym_data)) {
    return ty;
  } else {
    add_error(187, *p_sym_data, "Impossible to infer symbol type", "");
    return nullptr;
  }
}

bool Visitor_Type::is_same_type(const ast::AType& p_type_1, const ast::AType& p_type_2) const
{
  return p_type_1.is_same(p_type_2);
}

void Visitor_Type::ensure_expression_resolution(ast::AExpression& p_expr, std::shared_ptr<ast::AType> p_type_inferrance)
{
  if (!p_expr.expression_inferred_type && p_type_inferrance) {
    // primitive literal lazy type inferrance
    EPrimType inferred_ty = EPrimType::NONE;
    if (auto ty = dynamic_cast<ast::type::Primitive*>(p_type_inferrance->resolve().get())) inferred_ty = ty->type;

    if (auto ptr = dynamic_cast<ast::literal::Integral*>(&p_expr)) {
      if (EPrimType_is_integral(inferred_ty)) {
        p_expr.expression_inferred_type = p_type_inferrance;
        ptr->type                       = inferred_ty;
      }
    } else if (auto ptr = dynamic_cast<ast::literal::Floating_Point*>(&p_expr)) {
      if (EPrimType_is_floating(inferred_ty)) {
        p_expr.expression_inferred_type = p_type_inferrance;
        ptr->type                       = inferred_ty;
      }
    } else if (auto ptr = dynamic_cast<ast::literal::Fixed_Point*>(&p_expr)) {
      if (EPrimType_is_fixed(inferred_ty)) {
        p_expr.expression_inferred_type = p_type_inferrance;
        ptr->raw_type                   = inferred_ty;
      }
    } else if (auto ptr = dynamic_cast<ast::literal::Text_Pure*>(&p_expr)) {
      if (EPrimType_is_textual(inferred_ty)) {
        p_expr.expression_inferred_type = p_type_inferrance;
        ptr->text_type                  = inferred_ty;
      }
    }
  }

  // keep tracking nested nodes type
  p_expr.accept(*this);

  // no type to infer
  if (!p_type_inferrance) return;

  if (!p_expr.expression_inferred_type) {
    add_error(234, p_expr, "Impossible to define the expression type.", "");
    return;
  }

  // check if type inferred is compatible to the expected inferrance (or expected type)
  if (!p_expr.expression_inferred_type->is_same(*p_type_inferrance->resolve().get()))
    add_error_two_nodes(230, p_expr, *p_type_inferrance,
                        "Illegal type inferrance,\n  (expression) \"" + p_expr.expression_inferred_type->debug_str()
                            + "\" != \"" + p_type_inferrance->debug_str() + "\" (inferrance).",
                        "");
}


void Visitor_Type::visit(ast::Expr_ID& n)
{
  if (n.expression_inferred_type) return;

  if (auto ty = get_symbol_type(n, n.identifier_symbol)) {
    n.expression_inferred_type = ty;
  } else {
    add_error(185, n, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Qualified& n)
{
  if (n.expression_inferred_type) return;

  if (auto ty = get_symbol_type(n, n.identifier_symbol)) {
    n.expression_inferred_type = ty;
  } else {
    add_error(186, n, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Type& n)
{
  if (n.expression_inferred_type) return;

  if (auto ty = get_symbol_type(n, n.identifier_symbol)) {
    n.expression_inferred_type       = ty;
    n.name->expression_inferred_type = ty;
  } else {
    add_error(187, n, "Symbol inferred type not found", "");
  }
}

// expression inferred type;
void Visitor_Type::visit(ast::declaration::local::Pattern_Enum& n)
{
  Visitor_Default::visit(n);

  if (auto ptr =
          std::dynamic_pointer_cast<ast::declaration::Enum_Element>(n.name->expression_inferred_type->resolve())) {
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
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Tuple& n)
{
  Visitor_Default::visit(n);

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
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Entity& n)
{
  Visitor_Default::visit(n);

  if (auto ptr =
          std::dynamic_pointer_cast<ast::declaration::cop::Entity>(n.name->expression_inferred_type->resolve())) {
    for (auto& pat_comp : n.mapping) {
      bool found = false;
      for (auto& et_comp : ptr->comps) {
        if (et_comp->name == pat_comp->name) {
          pat_comp->expression_inferred_type = et_comp->expression_inferred_type->resolve();
          found                              = true;
          break;
        }
      }

      if (!found) add_error(159, *pat_comp, "Component is not in the entity composition", "");
    }
  } else {
    add_error(160, *n.name, "Expected entity type", "");
  }
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Component& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::cop::Component>(n.name->expression_inferred_type)) {
    for (auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (auto& comp_field : ptr->fields) {
        if (field_name == comp_field->declaration_name) {

          if (pattern->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern->node())) {
            map_ptr->type = comp_field->type;
          }

          found = true;
          break;
        }
      }

      if (!found) add_error(161, *n.name, "Field \"" + field_name + "\" not in the component definition", "");
    }
  } else {
    add_error(162, *n.name, "Expected component type", "");
  }
}


void Visitor_Type::visit(ast::declaration::Global& n)
{
  if (!n.type) {
    if (!n.expression->expression_inferred_type) n.expression->accept(*this);
    n.type = n.expression->expression_inferred_type->resolve();
  } else {
    add_error(188, *n.expression, "Impossible to infer symbol type", "");
  }

  Visitor_Default::visit(n);

  if (!n.type->is_same(*n.expression->expression_inferred_type)) {
    add_error_two_nodes(231, n, *n.expression,
                        "Incompatible type association,\n  (expression) \""
                            + n.expression->expression_inferred_type->debug_str() + "\" != \"" + n.type->debug_str()
                            + "\" (variable).",
                        "");
  }
}

void Visitor_Type::visit(ast::declaration::Function& n)
{
  n.prototype->accept(*this);

  if (n.declaration_name == "main") {
    if (!n.prototype->return_ty || n.prototype->return_ty == ast::type::get_void_type()) {
      n.prototype->return_ty = ast::type::get_i32_type();
    } else if (n.prototype->return_ty != ast::type::get_i32_type()) {
      add_error(218, *n.prototype->return_ty,
                "Violation of the main function convention, main must return i32 or u0 type.", "");
    }
  }

  // potential function return type inferrance
  if (n.prototype->return_ty == ast::type::get_void_type() && !n.prototype->is_explicit_return_type) {
    std::shared_ptr<ast::AType> return_ty;

    for (auto& elem : n.codeblock->elements) {
      if (auto ptr = dynamic_cast<ast::statement::Return*>(elem.node())) {
        if (!return_ty) {
          return_ty = ptr->value->expression_inferred_type->resolve();
          continue;
        }

        ensure_expression_resolution(*ptr->value, return_ty);
      }
    }

    n.prototype->return_ty = return_ty;
  }


  Visitor_Default::visit(n);
}

void Visitor_Type::visit(ast::declaration::local::Variable& n)
{
  // variable type inferrance
  if (!n.type) {
    n.expression->accept(*this);
    n.type = n.expression->expression_inferred_type->resolve();
  }
  // variable type explicit : expression type inferrance
  else {
    ensure_expression_resolution(*n.expression, n.type);
  }
}
void Visitor_Type::visit(ast::declaration::local::Variable_Binding& n)
{
  if (!n.type) {
    n.parent_pattern->accept(*this);
    n.type = n.parent_pattern->right->expression_inferred_type->resolve();
  }
}

void Visitor_Type::visit(ast::statement::Return& n)
{
  if (n.value) ensure_expression_resolution(*n.value, n.target_function->prototype->return_ty);

  Visitor_Default::visit(n);
}


void Visitor_Type::visit(ast::expression::If_Ternary& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.true_line->expression_inferred_type->resolve();
}

void Visitor_Type::visit(ast::expression::Member_Access& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.right->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::expression::Self& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.self_definition;
}
void Visitor_Type::visit(ast::expression::Other& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call_Argument& n)
{
  Visitor_Default::visit(n);

  if (!n.variadic_arg)
    ensure_expression_resolution(*n.expression, n.fn_param_type->type);
  else
    n.expression->accept(*this);

  n.expression_inferred_type = n.expression->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::expression::Call& n)
{
  n.callee->accept(*this);

  if (auto fn_ty = dynamic_cast<ast::declaration::Function*>(n.function_symbol.get())) {
    n.function_proto           = fn_ty->prototype;
    n.expression_inferred_type = fn_ty->prototype->return_ty->resolve();
  } else {
    add_error(193, *n.callee, "Expected fuction type in inferred type", "");
  }

  // resolve parameters
  size_t arg_count = 0;
  bool   name_mode = false;
  for (auto& arg : n.param_args) {
    // argument inferred type is equal to his expression inferred type
    // argument type on call is checked in semantic resolver

    // unamed argument encounted after a named argument
    if (!arg->name.empty() && name_mode) {
      add_error(212, *arg, "Unexpected unamed argument after a named argument.",
                "Named arguments are always the latest call arguments");
      continue;
    }

    // named argument encounted
    if (!arg->name.empty()) {
      name_mode = true;

      // find the corresponding parameter
      bool found = false;
      for (auto& param : n.function_proto->parameters) {
        if (param->declaration_name == arg->name) {
          arg->fn_param_type = param;
          arg->accept(*this);

          found = true;
          break;
        }
      }

      if (!found) {
        add_error_two_nodes(211, *arg, n, "The paramater name invoked \"" + arg->name + "\" dosen't exists.", "");
        continue;
      }
    }
    // unamed argument encounted : positional parameter
    else if (n.function_proto->parameters.size() > arg_count) {
      auto param         = n.function_proto->parameters[arg_count++];
      arg->fn_param_type = param;
      arg->accept(*this);
    }
    // unamed argument encounted out of param size but variadic function
    else if (n.function_proto->is_variadic) {
      arg->fn_param_type = nullptr;
      arg->variadic_arg  = true;
      arg->accept(*this);
    }
    // unamed argument encounted out of param size : not variadic function
    else {
      add_error_two_nodes(213, *arg, *n.function_symbol, "Too many arguments invoked.", "");
    }
  }
  for (auto& elem : n.gen_args) elem->accept(*this);

  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call_Pipe& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call_System& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Table_Access& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Ptr_At& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Ptr_Offset& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Ptr_Val& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Mut_Of& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.target->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::expression::Ref_Of& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.target->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::expression::Addr_Of& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Size_Of& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::GetBits& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Move& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::New_Ptr& n)
{
  Visitor_Default::visit(n);
}

void Visitor_Type::visit(ast::literal::Integral& n)
{
  if (n.expression_inferred_type) {
    if (auto ptr = dynamic_cast<ast::type::Primitive*>(n.expression_inferred_type->resolve().get())) {
      if (EPrimType_is_integral(ptr->type)) return;
    }

    add_error(235, n,
              "Illegal type inferrance (" + n.expression_inferred_type->debug_str() + ") on literal floating point",
              "make an explicit cast to a floating point or check the desired type inferred");
    return;
  }

  switch (n.type) {
  case EPrimType::iSize: n.expression_inferred_type = ast::type::get_isize_type(); break;
  case EPrimType::i8:    n.expression_inferred_type = ast::type::get_i8_type(); break;
  case EPrimType::i16:   n.expression_inferred_type = ast::type::get_i16_type(); break;
  case EPrimType::i32:   n.expression_inferred_type = ast::type::get_i32_type(); break;
  case EPrimType::i64:   n.expression_inferred_type = ast::type::get_i64_type(); break;
  case EPrimType::i128:  n.expression_inferred_type = ast::type::get_i128_type(); break;
  case EPrimType::uSize: n.expression_inferred_type = ast::type::get_usize_type(); break;
  case EPrimType::u8:    n.expression_inferred_type = ast::type::get_u8_type(); break;
  case EPrimType::u16:   n.expression_inferred_type = ast::type::get_u16_type(); break;
  case EPrimType::u32:   n.expression_inferred_type = ast::type::get_u32_type(); break;
  case EPrimType::u64:   n.expression_inferred_type = ast::type::get_u64_type(); break;
  case EPrimType::u128:  n.expression_inferred_type = ast::type::get_u128_type(); break;
  default:               n.expression_inferred_type = ast::type::get_isize_type(); n.type = EPrimType::iSize;
  }
}
void Visitor_Type::visit(ast::literal::Floating_Point& n)
{
  if (n.expression_inferred_type) {
    if (auto ptr = dynamic_cast<ast::type::Primitive*>(n.expression_inferred_type->resolve().get())) {
      if (EPrimType_is_floating(ptr->type)) return;
    }

    add_error(228, n,
              "Illegal type inferrance (" + n.expression_inferred_type->debug_str() + ") on literal floating point",
              "make an explicit cast to a floating point or check the desired type inferred");
    return;
  }

  switch (n.type) {
  case EPrimType::fSize: n.expression_inferred_type = ast::type::get_fsize_type(); break;
  case EPrimType::f16:   n.expression_inferred_type = ast::type::get_f16_type(); break;
  case EPrimType::f32:   n.expression_inferred_type = ast::type::get_f32_type(); break;
  case EPrimType::f64:   n.expression_inferred_type = ast::type::get_f64_type(); break;
  case EPrimType::f80:   n.expression_inferred_type = ast::type::get_f80_type(); break;
  case EPrimType::f128:  n.expression_inferred_type = ast::type::get_f128_type(); break;
  default:               n.expression_inferred_type = ast::type::get_fsize_type(); n.type = EPrimType::fSize;
  }
}

void Visitor_Type::visit(ast::literal::Table& n)
{
  Visitor_Default::visit(n);

  if (!n.values.empty()) n.element_type = n.values[0]->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::literal::Map& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Text_Interpolation& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Text_Pure& n)
{
  if (!n.expression_inferred_type) {
    switch (n.text_type) {
    case EPrimType::c_str: n.expression_inferred_type = ast::type::get_c_str_type(); break;
    case EPrimType::str:   n.expression_inferred_type = ast::type::get_str_type(); break;
    case EPrimType::text:  n.expression_inferred_type = ast::type::get_text_type(); break;
    default:               n.expression_inferred_type = ast::type::get_str_type(); n.text_type = EPrimType::str;
    }
  }
}
void Visitor_Type::visit(ast::literal::Textual_Format& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.values[0]->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::literal::Enum& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Tuple& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Range& n)
{
  ensure_expression_resolution(*n.start, n.end->expression_inferred_type);
  ensure_expression_resolution(*n.end, n.start->expression_inferred_type);

  n.expression_inferred_type = n.start->expression_inferred_type->resolve();
}
void Visitor_Type::visit(ast::literal::Structured_Data& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Entity& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Iterator& n)
{
  Visitor_Default::visit(n);
}


void Visitor_Type::visit(ast::statement::For& n)
{
  n.expression->accept(*this);

  if (n.index) n.index->type = n.expression->expression_inferred_type->resolve();
  for (auto& elem : n.items) elem->type = n.expression->expression_inferred_type->resolve();

  Visitor_Default::visit(n);
}


void Visitor_Type::visit(ast::operation::Cast_As& n)
{
  Visitor_Default::visit(n);

  n.expression_inferred_type = n.type->resolve();
}
void Visitor_Type::visit(ast::operation::Is& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::In& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Assignment& n)
{
  if (!n.left->expression_inferred_type) n.left->accept(*this);

  // variable type inferrance
  ensure_expression_resolution(*n.right, n.left->expression_inferred_type);
  if (n.expression_inferred_type) n.expression_inferred_type = n.left->expression_inferred_type->resolve();

  if (!n.right->expression_inferred_type->is_same(*n.left->expression_inferred_type)) {
    add_error_two_nodes(189, *n.left, *n.right,
                        "Incompatible type association,\n  (left) \"" + n.left->expression_inferred_type->debug_str()
                            + "\" != \"" + n.right->expression_inferred_type->debug_str() + "\" (right).",
                        "");
  }

  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Binary& n)
{
  if (EBinOpType_is_logical(n.op_ty)) {
    ensure_expression_resolution(*n.left, ast::type::get_bool_type());
    ensure_expression_resolution(*n.right, ast::type::get_bool_type());

    n.expression_inferred_type = ast::type::get_bool_type();
    Visitor_Default::visit(n);
    return;
  }


  ensure_expression_resolution(*n.left, n.right->expression_inferred_type);
  ensure_expression_resolution(*n.right, n.left->expression_inferred_type);


  if (EBinOpType_is_comparison(n.op_ty)) {
    n.expression_inferred_type = ast::type::get_bool_type();
    Visitor_Default::visit(n);
    return;
  }

  // div always return a floating point
  if (n.op_ty == EBinOpType::Div) {
    n.expression_inferred_type = ast::type::get_fsize_type();
    Visitor_Default::visit(n);
    return;
  }

  if (!n.left->expression_inferred_type->is_same(*n.right->expression_inferred_type)) {
    add_error_two_nodes(233, *n.left, *n.right,
                        "Incompatible type association,\n  (left) \"" + n.left->expression_inferred_type->debug_str()
                            + "\" " + EBinOpType_to_str(n.op_ty) + " \""
                            + n.right->expression_inferred_type->debug_str() + "\" (right).",
                        "");
  }

  n.expression_inferred_type = n.left->expression_inferred_type->resolve();

  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Unary& n)
{
  Visitor_Default::visit(n);

  if (n.unary_op == EUnaryOpType::_not) {
    n.expression_inferred_type = ast::type::get_bool_type();
    return;
  }

  ensure_expression_resolution(n, n.base->expression_inferred_type);
}
void Visitor_Type::visit(ast::operation::Interval& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Ptr_Dist& n)
{
  Visitor_Default::visit(n);
}