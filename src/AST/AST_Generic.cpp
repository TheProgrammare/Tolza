#include "AST_Generic.hpp"

#include "AST_Declaration_COP.hpp"

bool AST::Generic::Have_Op::type_isValid(const AType &type) const
{
  /*
  if (auto ptr = dynamic_cast<const Declaration::COP::Entity*>(&type)) {
      if (op_need_explict_retrun(operatorType))
          return ptr->contains_op(operatorType, explicit_return_type.get());
      else
          return ptr->contains_op(operatorType, nullptr);
  }*/
  return false;
}

bool AST::Generic::Compatible_System::type_isValid(const AType &type) const
{
  if (auto ptr1 = dynamic_cast<const Declaration::COP::Entity *>(&type)) {
    return resolved_system_sym->manage_entity(*ptr1);
  } else if (auto ptr2 = dynamic_cast<const Declaration::COP::Component *>(&type)) {
    return resolved_system_sym->manage_component(*ptr2);
  }
  return false;
}

bool AST::Generic::Use_Component::type_isValid(const AType &type) const
{
  if (auto ptr = dynamic_cast<const Declaration::COP::Entity *>(&type)) {
    return ptr->contains_comp(*resolved_comp_sym);
  }
  return false;
}

bool AST::Generic::Have_Role::type_isValid(const AType &type) const
{
  for (auto &comp : resolved_role_sym->components) {
    if (comp->inferred_type == &type) return true;
  }
  return false;
}