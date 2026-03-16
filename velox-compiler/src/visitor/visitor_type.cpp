
#include "visitor_type.hpp"

#include <cstddef>
#include <iostream>
#include <memory>

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

ast::AType* Visitor_Type::get_inferred_type(ast::Node& node, bool is_prototype_expected) const
{
  if (auto ptr = dynamic_cast<ast::AType*>(&node))
    return ptr;
  else if (auto ptr = dynamic_cast<ast::AExpression*>(&node))
    return ptr->inferred_type;
  else if (auto ptr = dynamic_cast<ast::declaration::Global*>(&node))
    return ptr->type.get();
  else if (auto ptr = dynamic_cast<ast::declaration::Function*>(&node)) {
    if (is_prototype_expected) return ptr->prototype.get();
    return ptr->prototype->returnType.get();
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(&node))
    return ptr->type.get();
  else if (auto ptr = dynamic_cast<ast::declaration::local::Variable_Binding*>(&node))
    return ptr->type;
  else if (auto ptr = dynamic_cast<ast::declaration::cop::System*>(&node))
    return ptr->prototype->returnType.get();
  else if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(&node))
    return ptr->returnType.get();
  else if (auto ptr = dynamic_cast<ast::declaration::local::Parameter*>(&node))
    return ptr->type.get();
  else if (auto ptr = dynamic_cast<ast::declaration::cop::Component_Field*>(&node))
    return ptr->type.get();
  else if (auto ptr = dynamic_cast<ast::declaration::Type_Alias*>(&node)) {
    return ptr->type.get();
  } else {
    error_add(168, node, "Type inferrance impossible on [" + node.debug_str() + "](" + typeid(node).name() + ")", "");
    return nullptr;
  }
}

ast::AType* Visitor_Type::get_symbol_type(const ast::Node& n, const ast::SYM_REF sym_data) const
{
  if (!sym_data) {
    error_add(163, n, "Unresolved symbol.", "");
    return nullptr;
  }
  if (!sym_data->symbol) {
    error_add(164, n, "Invalid symbol origin", "");
    return nullptr;
  }

  if (auto ty = get_inferred_type(*sym_data->symbol.get())) {
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
    error_add(185, *n.symbol->symbol, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Qualified& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_symbol_type(n, n.symbol)) {
    n.inferred_type = ty;
  } else {
    error_add(186, *n.symbol->symbol, "Symbol inferred type not found", "");
  }
}
void Visitor_Type::visit(ast::Expr_ID_Type& n)
{
  Visitor_Default::visit(n);

  if (auto ty = get_symbol_type(n, n.symbol)) {
    n.inferred_type       = ty;
    n.name->inferred_type = ty;
  } else {
    error_add(187, *n.symbol->symbol, "Symbol inferred type not found", "");
  }
}

// expression inferred type;
void Visitor_Type::visit(ast::declaration::local::Pattern_Enum& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = dynamic_cast<ast::declaration::Enum_Element*>(n.name->inferred_type)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines(155, n, *ptr, "Inequal types binded on " + n.name->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty.get();
        }
      }
    }
  }
  error_add(156, *n.name, "Unexpected reference encounted, expected Enum element", "");
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Tuple& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = dynamic_cast<ast::type::Tuple*>(n.right->inferred_type)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines(157, n, *ptr, "Inequal types binded on " + ptr->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty.get();
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

  if (auto ptr = dynamic_cast<ast::declaration::cop::Entity*>(n.name->inferred_type)) {
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

  if (auto ptr = dynamic_cast<ast::declaration::cop::Component*>(n.name->inferred_type)) {
    for (auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (auto& comp_field : ptr->fields) {
        if (field_name == comp_field->name) {

          if (pattern->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern->node())) {
            map_ptr->type = comp_field->type.get();
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
  if (!n.type) {
    if (auto ty = get_inferred_type(*n.expression)) {
      n.type = std::unique_ptr<ast::AType>(ty);
    } else {
      error_add(188, *n.expression, "Impossible to infer symbol type", "");
    }
  }
}

void Visitor_Type::visit(ast::declaration::local::Variable& n)
{
  if (!n.type) {
    if (auto ty = get_inferred_type(*n.expression)) {
      n.type = std::unique_ptr<ast::AType>(ty);
    } else {
      error_add(189, *n.expression, "Impossible to infer symbol type", "");
    }
  }
}
void Visitor_Type::visit(ast::declaration::local::Variable_Binding& n)
{
  if (!n.type) {
    if (auto ty = get_inferred_type(*n.parent_pattern->right)) {
      n.type = ty;
    } else {
      error_add(190, *n.parent_pattern->right, "Impossible to infer symbol type", "");
    }
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

  n.inferred_type = n.self_definition.get();
}
void Visitor_Type::visit(ast::expression::Other& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call_Argument& n)
{
  Visitor_Default::visit(n);
}
void Visitor_Type::visit(ast::expression::Call& n)
{
  Visitor_Default::visit(n);

  if (auto proto = get_inferred_type(*n.callee, true)) {
    n.function_proto = proto;

    if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(proto)) {
      n.inferred_type = ptr->returnType.get();
    } else {
      error_add(193, *n.callee, "Expected fuction type in inferred type", "");
    }
  } else {
    error_add(191, *n.callee, "Impossible to infer symbol type", "");
  }
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
    error_add(192, *n.callee, "Impossible to infer symbol type", "");
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