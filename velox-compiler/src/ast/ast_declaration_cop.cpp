#include "ast_declaration_cop.hpp"

#include "ast_expression.hpp"
#include "ast_declaration_local.hpp"
#include "ast_literal.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"

void ast::declaration::cop::Component_Field::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Component_Field::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Component::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Role::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Role::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Entity::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Entity::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Entity_Cast::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Entity_Cast::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Entity_Op::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Entity_Op::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::Entity_OpIndex::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::Entity_OpIndex::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::System::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::System::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::cop::System_Case::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::cop::System_Case::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

bool ast::declaration::cop::System_Case::manage_entity(const Entity& entity) const
{
  return false;
}

bool ast::declaration::cop::System_Case::manage_component(const Component& comp) const
{
  return false;
}

bool ast::declaration::cop::Entity::contains_op(EBinOpType op, const AType* return_type) const
{
  for (auto& elem : operators) {
    if (elem->operatorType == op) {
      return true;
    }
  }
  return false;
}

bool ast::declaration::cop::Entity::contains_cast(const AType& target_type, bool isCastFrom) const
{
  // difficult resolution:
  // entity have 2 cast way:
  // cast self as T / cast T as self
  // generic have 2 cast way check:
  // T cast to U / T cast from U

  if (isCastFrom) {
    for (auto& elem : casts) {
      if (!elem->isSourceSelf) {
        if (auto id_ty_ptr = dynamic_cast<const AType*>(elem->source.get())) {
          // if (target_type == *id_ty_ptr) return true;
        }
      }
    }
    return false;
  } else {
    for (auto& elem : casts) {
      // if (elem->isSourceSelf && *elem->target == target_type) return true;
    }
    return false;
  }
}

bool ast::declaration::cop::Entity::contains_comp(const ast::declaration::cop::Component& target_comp) const
{
  for (auto& comp : comps) {
    if (!comp) return false;
    if (comp->name->get_base_name() == target_comp.name) return true;
  }
  return false;
}

bool ast::declaration::cop::Entity::compare_with(const AType& other) const
{
  if (auto ptr = dynamic_cast<const Entity*>(&other)) {
    return name == ptr->name;
  }
  return false;
}

bool ast::declaration::cop::System::manage_entity(const ast::declaration::cop::Entity& entity) const
{
  for (auto& with : cases) {
    if (with->manage_entity(entity)) return true;
  }
  return false;
}

bool ast::declaration::cop::System::manage_component(const ast::declaration::cop::Component& comp) const
{
  for (auto& with : cases) {
    if (with->manage_component(comp)) return true;
  }
  return false;
}

std::string ast::declaration::cop::Component::debug_str() const
{
  std::string out;
  out += "comp " + name;
  if (gen_where) out += gen_where->debug_str();
  return out;
}


ast::declaration::cop::Entity::~Entity() = default;
