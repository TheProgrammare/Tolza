#include "ast_declaration_cop.hpp"

#include "ast/ast_data.hpp"
#include "ast_expression.hpp"
#include "ast_declaration_local.hpp"
#include "ast_literal.hpp"

#include "visitor/visitor_base.hpp"

#include "codegen/visitor_codegen.hpp"


void ast::declaration::cop::Component_Field::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Role::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_New::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_Del::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_Cast::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_Op::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_Access_Op::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::Entity_Transfert::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::System::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::cop::System_Case::accept(Visitor_Base& v)
{
  v.visit(*this);
}
bool ast::declaration::cop::System_Case::manage_entity(const Entity& entity) const
{
  return false;
}

bool ast::declaration::cop::System_Case::manage_component(const Component& comp) const
{
  return false;
}

std::string ast::declaration::cop::Component_Field::mangle_type() const
{
  return type->mangle_type();
}

bool ast::declaration::cop::Component_Field::compare_with(const AType& other) const
{
  if (auto ptr = dynamic_cast<const Component_Field*>(&other)) {
    return type == ptr->type;
  }
  return type->compare_with(other);
}


bool ast::declaration::cop::Entity::contains_op(EBinOpType op, const AType* return_type) const
{
  for (auto& elem : operators) {
    if (elem->op_ty == op) {
      return true;
    }
  }
  return false;
}

bool ast::declaration::cop::Entity::contains_access_op(EAccessOpType op, const AType* return_type) const
{
  for (auto& elem : op_access) {
    if (elem->op_ty == op) {
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
    if (*comp->name.get() == target_comp.declaration_name) return true;
  }
  return false;
}

bool ast::declaration::cop::Entity::compare_with(const AType& other) const
{
  if (auto ptr = dynamic_cast<const Entity*>(&other)) {
    return declaration_name == ptr->declaration_name;
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
  out += "comp " + declaration_name;
  if (gen_where) out += gen_where->debug_str();
  return out;
}

std::string ast::declaration::cop::Entity_New::debug_str() const
{
  return "new " + prototype->debug_str();
}


ast::declaration::cop::Entity::~Entity() = default;


llvm::Function* ast::declaration::cop::Entity_New::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Function* ast::declaration::cop::Entity_Del::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Function* ast::declaration::cop::Entity_Cast::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Function* ast::declaration::cop::Entity_Op::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Function* ast::declaration::cop::Entity_Access_Op::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Function* ast::declaration::cop::Entity_Transfert::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


llvm::Value* ast::declaration::cop::Entity_New::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::cop::Entity_Del::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::cop::Entity_Cast::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::cop::Entity_Op::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::cop::Entity_Access_Op::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::cop::Entity_Transfert::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}


llvm::Function* ast::declaration::cop::System::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


llvm::Type* ast::declaration::cop::Component::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::declaration::cop::Component_Field::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Type* ast::declaration::cop::Role::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Type* ast::declaration::cop::Entity::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}


llvm::Value* ast::declaration::cop::System::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::cop::System_Case::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}


llvm::Value* ast::declaration::cop::Component::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::cop::Component_Field::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}

llvm::Value* ast::declaration::cop::Role::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}

llvm::Value* ast::declaration::cop::Entity::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}