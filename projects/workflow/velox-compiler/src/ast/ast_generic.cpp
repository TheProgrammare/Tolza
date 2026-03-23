#include "ast_generic.hpp"

#include "ast_declaration_cop.hpp"

#include "visitor/visitor_base.hpp"


void ast::generic::Is_Type::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::generic::Can_Cast::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::generic::Have_Op::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::generic::Have_Role::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::generic::Use_Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::generic::Compatible_System::accept(Visitor_Base& v)
{
  v.visit(*this);
}
bool ast::generic::Have_Op::type_isValid(const AType& type) const
{
  /*
  if (auto
  ptr =
  dynamic_cast<const
  declaration::cop::Entity*>(&type))
  { if
  (op_need_explict_retrun(operatorType))
          return ptr->contains_op(operatorType, explicit_return_type.get());
      else
          return ptr->contains_op(operatorType, nullptr);
  }*/
  return false;
}

bool ast::generic::Compatible_System::type_isValid(const AType& type) const
{
  if (auto ptr1 = dynamic_cast<const declaration::cop::Entity*>(&type)) {
    return resolved_system_sym->manage_entity(*ptr1);
  } else if (auto ptr2 = dynamic_cast<const declaration::cop::Component*>(&type)) {
    return resolved_system_sym->manage_component(*ptr2);
  }
  return false;
}

bool ast::generic::Use_Component::type_isValid(const AType& type) const
{
  if (auto ptr = dynamic_cast<const declaration::cop::Entity*>(&type)) {
    return ptr->contains_comp(*resolved_comp_sym);
  }
  return false;
}

bool ast::generic::Have_Role::type_isValid(const AType& type) const
{
  for (auto& comp : resolved_role_sym->components) {
    // if (comp->inferred_type == &type) return true;
  }
  return false;
}