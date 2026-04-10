
#include "visitor_type.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>

#include "ast/ast_data.hpp"
#include "ast/ast_inferred_type_singleton.hpp"
#include "rules/rule_type.hpp"
#include "visitor_default.hpp"
#include "symbol_manager.hpp"

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

std::shared_ptr<ast::AType> Visitor_Type::get_inferred_type(ast::Node& node, bool is_prototype_expected) const
{
  if (auto ptr = dynamic_cast<ast::AType*>(&node))
    return std::shared_ptr<ast::AType>(ptr, [](ast::AType*) {}); // no-op deleter
  else if (auto ptr = dynamic_cast<ast::AExpression*>(&node))
    return ptr->inferred_type;
  else if (auto ptr = dynamic_cast<ast::declaration::Global*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::Function*>(&node)) {
    if (is_prototype_expected) return ptr->prototype;
    return ptr->prototype->returnType;
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::local::Variable_Binding*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::cop::System*>(&node))
    return ptr->prototype->returnType;
  else if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(&node))
    return ptr->returnType;
  else if (auto ptr = dynamic_cast<ast::declaration::local::Parameter*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::cop::Component_Field*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::Type_Alias*>(&node)) {
    return ptr->type;
  } else {
    error_add(168, node, "Type inferrance impossible on [" + node.debug_str() + "](" + typeid(node).name() + ")", "");
    return nullptr;
  }
}

std::shared_ptr<ast::AType> Visitor_Type::get_symbol_type(const ast::Node& n, const ast::SYM_REF sym_data) const
{
  if (!sym_data) {
    error_add(163, n, "Unresolved symbol.", "");
    return nullptr;
  }
  if (!sym_data->symbol) {
    error_add(164, n, "Invalid symbol origin", "");
    return nullptr;
  }


  if (auto ty = get_inferred_type(*sym_data->symbol)) {
    return ty;
  } else {
    error_add(187, *sym_data->symbol, "Impossible to infer symbol type", "");
  }
}

bool Visitor_Type::is_same_type(const ast::AType& p_type_1, const ast::AType& p_type_2) const
{
  return p_type_1.is_same(p_type_2);
}

void Visitor_Type::visit(ast::Expr_ID& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_symbol_type(n, n.symbol)) {
    n.inferred_type = ty;
  } else {
    error_add(185, n, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Qualified& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_symbol_type(n, n.symbol)) {
    n.inferred_type = ty;
  } else {
    error_add(186, n, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Type& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_symbol_type(n, n.symbol)) {
    n.inferred_type       = ty;
    n.name->inferred_type = ty;
  } else {
    error_add(187, n, "Symbol inferred type not found", "");
  }
}

// expression inferred type;
void Visitor_Type::visit(ast::declaration::local::Pattern_Enum& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::Enum_Element>(n.name->inferred_type)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines(155, n, *ptr, "Inequal types binded on " + n.name->debug_str(), "");
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
  error_add(156, *n.name, "Unexpected reference encounted, expected Enum element", "");
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Tuple& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = std::dynamic_pointer_cast<ast::type::Tuple>(n.right->inferred_type)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines(157, n, *ptr, "Inequal types binded on " + ptr->debug_str(), "");
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
    error_add(158, *n.right, "Expected tuple type", "");
  }
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Entity& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::cop::Entity>(n.name->inferred_type)) {
    for (auto& pat_comp : n.mapping) {
      bool found = false;
      for (auto& et_comp : ptr->comps) {
        if (et_comp->name == pat_comp->name) {
          pat_comp->inferred_type = et_comp->inferred_type;
          found                   = true;
          break;
        }
      }

      if (!found) error_add(159, *pat_comp, "Component is not in the entity composition", "");
    }
  } else {
    error_add(160, *n.name, "Expected entity type", "");
  }
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Component& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::cop::Component>(n.name->inferred_type)) {
    for (auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (auto& comp_field : ptr->fields) {
        if (field_name == comp_field->name) {

          if (pattern->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern->node())) {
            map_ptr->type = comp_field->type;
          }

          found = true;
          break;
        }
      }

      if (!found) error_add(161, *n.name, "Field \"" + field_name + "\" not in the component definition", "");
    }
  } else {
    error_add(162, *n.name, "Expected component type", "");
  }
}


void Visitor_Type::visit(ast::declaration::Global& n)
{
  Visitor_Default::visit(n);

  if (!n.type) {
    if (auto ty = get_inferred_type(*n.expression)) {
      n.type = std::shared_ptr<ast::AType>(ty);
    } else {
      error_add(188, *n.expression, "Impossible to infer symbol type", "");
    }
  }
}

void Visitor_Type::visit(ast::declaration::Function& n)
{
  n.prototype->accept(*this);

  if (n.name == "main") {
    if (!n.prototype->returnType || n.prototype->returnType == ast::type::get_void_type()) {
      n.prototype->returnType = ast::type::get_i32_type();
    } else if (n.prototype->returnType != ast::type::get_i32_type()) {
      error_add(218, *n.prototype->returnType,
                "Violation of the main function convention, main must return i32 or u0 type.", "");
    }
  }
  Visitor_Default::visit(n);
}


void Visitor_Type::visit(ast::declaration::local::Variable& n)
{
  Visitor_Default::visit(n);

  if (!n.type) {
    if (auto ty = get_inferred_type(*n.expression)) {
      n.type = ty;
    } else {
      error_add(189, *n.expression, "Impossible to infer symbol type", "");
    }
  }
}
void Visitor_Type::visit(ast::declaration::local::Variable_Binding& n)
{
  Visitor_Default::visit(n);

  if (!n.type) {
    if (auto ty = get_inferred_type(*n.parent_pattern->right)) {
      n.type = ty;
    } else {
      error_add(190, *n.parent_pattern->right, "Impossible to infer symbol type", "");
    }
  }
}

void Visitor_Type::visit(ast::statement::Return& n)
{
  Visitor_Default::visit(n);

  // return void
  if (!n.value) {
    if (n.target_function->prototype->returnType != ast::type::get_void_type()) {
      error_two_lines(221, n, *n.target_function,
                      "Illegal return expression wihout expression.\n  - return type: "
                          + n.value->inferred_type->mangle_type()
                          + "\n  - fn type: " + n.target_function->prototype->returnType->mangle_type(),
                      "");
    }
    return;
  }

  if (n.target_function->prototype->returnType != n.value->inferred_type) {
    error_two_lines(219, n, *n.target_function,
                    "Illegal return expression type.\n  - return type: " + n.value->inferred_type->mangle_type()
                        + "\n  - fn type: " + n.target_function->prototype->returnType->mangle_type(),
                    "");
  }
}


void Visitor_Type::visit(ast::expression::If_Ternary& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = n.true_line->inferred_type;
}

void Visitor_Type::visit(ast::expression::Member_Access& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = n.right->inferred_type;
}
void Visitor_Type::visit(ast::expression::Self& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = n.self_definition;
}
void Visitor_Type::visit(ast::expression::Other& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call_Argument& n)
{
  Visitor_Default::visit(n);

  n.expression->accept(*this);
  n.inferred_type = n.expression->inferred_type;
}
void Visitor_Type::visit(ast::expression::Call& n)
{
  n.callee->accept(*this);

  if (auto proto = get_inferred_type(*n.function_symbol->symbol, true)) {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Function_Proto>(proto)) {
      n.function_proto = ptr;
      n.inferred_type  = ptr->returnType;
    } else {
      error_add(193, *n.callee, "Expected fuction type in inferred type", "");
    }
  } else {
    error_add(191, *n.callee, "Impossible to infer symbol type", "");
  }

  // resolve parameters
  size_t arg_count = 0;
  bool   name_mode = false;
  for (auto& arg : n.param_args) {
    // argument inferred type is equal to his expression inferred type
    // argument type on call is checked in semantic resolver
    arg->accept(*this);

    // unamed argument encounted after a named argument
    if (!arg->name.empty() && name_mode) {
      error_add(212, *arg, "Unexpected unamed argument after a named argument.",
                "Named arguments are always the latest call arguments");
      continue;
    }

    // named argument encounted
    if (!arg->name.empty()) {
      name_mode = true;

      // find the corresponding parameter
      bool found = false;
      for (auto& param : n.function_proto->parameters) {
        if (param->name == arg->name) {
          arg->fn_param_type = param.get();
          found              = true;
          break;
        }
      }

      if (!found) {
        error_two_lines(211, *arg, n, "The paramater name invoked \"" + arg->name + "\" dosen't exists.", "");
        continue;
      }
    }
    // unamed argument encounted : positional parameter
    else if (n.function_proto->parameters.size() < arg_count) {
      auto param = n.function_proto->parameters[arg_count++];

      arg->fn_param_type = param.get();
    }
    // unamed argument encounted out of param size but variadic function
    else if (n.function_proto->isVariadic) {
      arg->fn_param_type = nullptr;
      arg->variadic_arg  = true;
    }
    // unamed argument encounted out of param size : not variadic function
    else {
      error_two_lines(213, *arg, n, "Too many arguments invoked.", "");
    }
  }
  for (auto& elem : n.gen_args) elem->accept(*this);
}
void Visitor_Type::visit(ast::expression::Call_Pipe& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_inferred_type(*n.callee)) {
    n.inferred_type = ty;
  } else {
    error_add(192, *n.callee, "Impossible to infer symbol type", "");
  }
}
void Visitor_Type::visit(ast::expression::Call_System& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_inferred_type(*n.callee)) {
    n.inferred_type = ty;
  } else {
    error_add(194, *n.callee, "Impossible to infer symbol type", "");
  }
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

  n.inferred_type = n.target->inferred_type;
}
void Visitor_Type::visit(ast::expression::Ref_Of& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = n.target->inferred_type;
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
  Visitor_Default::visit(n);

  switch (n.type) {
  case EPrimType::iSize: n.inferred_type = ast::type::get_isize_type(); break;
  case EPrimType::i8:    n.inferred_type = ast::type::get_i8_type(); break;
  case EPrimType::i16:   n.inferred_type = ast::type::get_i16_type(); break;
  case EPrimType::i32:   n.inferred_type = ast::type::get_i32_type(); break;
  case EPrimType::i64:   n.inferred_type = ast::type::get_i64_type(); break;
  case EPrimType::i128:  n.inferred_type = ast::type::get_i128_type(); break;
  case EPrimType::uSize: n.inferred_type = ast::type::get_usize_type(); break;
  case EPrimType::u8:    n.inferred_type = ast::type::get_u8_type(); break;
  case EPrimType::u16:   n.inferred_type = ast::type::get_u16_type(); break;
  case EPrimType::u32:   n.inferred_type = ast::type::get_u32_type(); break;
  case EPrimType::u64:   n.inferred_type = ast::type::get_u64_type(); break;
  case EPrimType::u128:  n.inferred_type = ast::type::get_u128_type(); break;
  default:               n.inferred_type = ast::type::get_isize_type();
  }
}
void Visitor_Type::visit(ast::literal::Floating& n)
{
  Visitor_Default::visit(n);

  switch (n.type) {
  case EPrimType::fSize: n.inferred_type = ast::type::get_fsize_type(); break;
  case EPrimType::f16:   n.inferred_type = ast::type::get_f16_type(); break;
  case EPrimType::f32:   n.inferred_type = ast::type::get_f32_type(); break;
  case EPrimType::f64:   n.inferred_type = ast::type::get_f64_type(); break;
  case EPrimType::f80:   n.inferred_type = ast::type::get_f80_type(); break;
  case EPrimType::f128:  n.inferred_type = ast::type::get_f128_type(); break;
  default:               n.inferred_type = ast::type::get_fsize_type();
  }
}

void Visitor_Type::visit(ast::literal::Table& n)
{
  Visitor_Default::visit(n);

  if (!n.values.empty()) n.element_type = n.values[0]->inferred_type;
}
void Visitor_Type::visit(ast::literal::Map& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::literal::Text_Interpolation& n)
{
  Visitor_Default::visit(n);
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
  Visitor_Default::visit(n);
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


void Visitor_Type::visit(ast::operation::Cast_As& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = n.type;
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
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Binary& n)
{
  Visitor_Default::visit(n);

  if (std::find(k_boolean_op.begin(), k_boolean_op.end(), n.op) != k_boolean_op.end()) {
    n.inferred_type = ast::type::get_bool_type();
    return;
  }

  // div always return a floating point
  if (n.op == EBinOpType::Div) {
    n.inferred_type = ast::type::get_fsize_type();
    return;
  }

  n.inferred_type = n.left->inferred_type;
}
void Visitor_Type::visit(ast::operation::Unary& n)
{
  Visitor_Default::visit(n);

  if (n.unary_op == EUnaryOpType::_not) {
    n.inferred_type = ast::type::get_bool_type();
    return;
  }

  n.inferred_type = n.base->inferred_type;
}
void Visitor_Type::visit(ast::operation::Interval& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::operation::Ptr_Dist& n)
{
  Visitor_Default::visit(n);
}