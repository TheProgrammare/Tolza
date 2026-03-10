
#include "visitor_type.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"
#include "visitor/visitor_default.hpp"
#include <cstddef>

namespace ast_loc = ast::declaration::local;


bool Visitor_Type::is_same_type(const ast::AType& p_type_1, const ast::AType& p_type_2)
{
  return p_type_1.is_same(p_type_2);
}

void Visitor_Type::visit(ast::Expr_ID& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = get_symbol<ast::AType>(n, n.symbol);
}
void Visitor_Type::visit(ast::Expr_ID_Qualified& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = get_symbol<ast::AType>(n, n.symbol);
}
void Visitor_Type::visit(ast::Expr_ID_Type& n)
{
  Visitor_Default::visit(n);

  n.inferred_type = get_symbol<ast::AType>(n, n.symbol);
}

// expression inferred type;
void Visitor_Type::visit(ast::declaration::local::Pattern_Enum& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = get_symbol<ast::declaration::Enum_Element>(*n.name, n.name->symbol)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines<155>(n, *ptr, "Inequal types binded on " + n.name->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map.kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map.node())) {
          map_ptr->type = ty.get();
        }
      }
    }
  }
  error_add<156>(*n.name, "Unexpected reference encounted, expected Enum element", "");
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Tuple& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = get_inferred_type<ast::type::Tuple>(n.right->inferred_type)) {
    if (n.mapping.size() != ptr->types.size()) {
      error_two_lines<157>(n, *ptr, "Inequal types binded on " + ptr->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map.kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map.node())) {
          map_ptr->type = ty.get();
        }
      }
    }
  } else {
    error_add<158>(*n.right, "Expected tuple type", "");
  }
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Entity& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = dynamic_cast<ast::declaration::cop::Entity*>(n.name->symbol->symbol.get())) {
    for (auto& pat_comp : n.mapping) {
      bool found = false;
      for (auto& et_comp : ptr->comps) {
        if (et_comp->name == pat_comp->name) {
          pat_comp->inferred_type = et_comp->inferred_type;
          found                   = true;
          break;
        }
      }

      if (!found) error_add<159>(*pat_comp, "Component is not in the entity composition", "");
    }
  } else {
    error_add<160>(*n.name, "Expected entity type", "");
  }
}
void Visitor_Type::visit(ast::declaration::local::Pattern_Component& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = dynamic_cast<ast::declaration::cop::Component*>(n.name->symbol->symbol.get())) {
    for (auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (auto& comp_field : ptr->fields) {
        if (field_name == comp_field->name) {

          if (pattern.kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern.node())) {
            map_ptr->type = comp_field->type.get();
          }

          found = true;
          break;
        }
      }

      if (!found) error_add<161>(*n.name, "Field \"" + field_name + "\" not in the component definition", "");
    }
  } else {
    error_add<162>(*n.name, "Expected component type", "");
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

  if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(n.callee->inferred_type)) {
    n.inferred_type = ptr->returnType.get();
  } else {
    error_add<165>(n, "Unexpected inferred type of [" + n.debug_str() + "]", "It's must be a function prototype.");
  }
}
void Visitor_Type::visit(ast::expression::Call_Pipe& n)
{
  Visitor_Default::visit(n);

  if (auto ptr = dynamic_cast<ast::type::Function_Proto*>(n.callee->inferred_type)) {
    n.inferred_type = ptr->returnType.get();
  } else {
    error_add<166>(*n.callee, "Unexpected inferred type (" + n.callee->inferred_type->debug_str() + ")",
                   "It's must be a function prototype.");
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
void Visitor_Type::visit(ast::literal::Component& n)
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