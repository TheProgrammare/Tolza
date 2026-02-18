#include "AST_Declaration_COP.hpp"

#include "AST_Declaration.hpp"

bool AST::Declaration::COP::System_Case::manage_entity(const Entity &entity) const { return false; }

bool AST::Declaration::COP::System_Case::manage_component(const Component &comp) const { return false; }

bool AST::Declaration::COP::Entity::contains_op(EBinOpType op, const AType *return_type) const
{
  for (auto &elem : operators) {
    if (elem->operatorType == op) {
      return true;
    }
  }
  return false;
}

bool AST::Declaration::COP::Entity::contains_cast(const AType &target_type, bool isCastFrom) const
{
  // difficult resolution:
  // entity have 2 cast way: cast self as T / cast T as self
  // generic have 2 cast way check: T cast to U / T cast from U

  if (isCastFrom) {
    for (auto &elem : casts) {
      if (!elem->isSourceSelf) {
        if (auto id_ty_ptr = dynamic_cast<const AType *>(elem->source.get())) {
          // if (target_type == *id_ty_ptr) return true;
        }
      }
    }
    return false;
  } else {
    for (auto &elem : casts) {
      // if (elem->isSourceSelf && *elem->target == target_type) return true;
    }
    return false;
  }
}

bool AST::Declaration::COP::Entity::contains_comp(const AST::Declaration::COP::Component &target_comp) const
{
  for (auto &comp : comps) {
    if (!comp) return false;
    if (comp->name->get_base_name() == target_comp.name) return true;
  }
  return false;
}

bool AST::Declaration::COP::System::manage_entity(const AST::Declaration::COP::Entity &entity) const
{
  for (auto &with : cases) {
    if (with->manage_entity(entity)) return true;
  }
  return false;
}

bool AST::Declaration::COP::System::manage_component(const AST::Declaration::COP::Component &comp) const
{
  for (auto &with : cases) {
    if (with->manage_component(comp)) return true;
  }
  return false;
}