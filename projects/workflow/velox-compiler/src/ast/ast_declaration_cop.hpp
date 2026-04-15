#pragma once

#include "ast/ast_data.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_type.hpp"
#include "ast_base.hpp"
#include <memory>

namespace ast
{
namespace declaration
{
namespace cop
{

struct Component_Field final : public ADeclaration, AType {
  enum class EBorrow { None, ref, mut };

  std::shared_ptr<Component> parent_component;

  std::shared_ptr<AType>       type;
  std::unique_ptr<AExpression> default_value;
  bool                         is_no_default = false;

  EBorrow borrow = EBorrow::None;

  CODEGEN_PASS
  CODEGEN_TY

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override;

  std::string debug_str() const override
  {
    std::string out = "field ";
    switch (borrow) {
    case EBorrow::None: break;
    case EBorrow::ref:  out += "ref ";
    case EBorrow::mut:  out += "mut ";
    }
    out += declaration_name + ": " + type->debug_str();
    if (default_value) out += " = " + default_value->debug_str();
    return out;
  }

  VISTOR_ACCEPT
};

struct Component final : public ADeclaration, AType {
  [[maybe_unused]]
  std::shared_ptr<local::Generic_Parameter_Element> gen_where;
  std::vector<std::shared_ptr<Component_Field>>     fields;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override;

  std::string mangle_type() const override
  {
    return "cp." + declaration_name;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Component*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

struct Role final : public ADeclaration, AType {
  std::vector<std::unique_ptr<AExpression>> components;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "declaration role \"" + declaration_name + "\"";
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Role*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  std::string mangle_type() const override
  {
    return "rl." + declaration_name;
  }

  VISTOR_ACCEPT
};

struct Entity_New;
struct Entity_Del;
struct Entity_Op;
struct Entity_Cast;

struct Entity final : public ADeclaration, AType {
  ~Entity();

  std::vector<std::unique_ptr<literal::Structured_Data>> comps;

  // fn type, lines
  std::vector<std::shared_ptr<Entity_New>> news;
  std::shared_ptr<Entity_Del>              del;

  std::shared_ptr<local::Generic_Parameter_Element> gen_params;
  std::vector<std::shared_ptr<Entity_Op>>           operators;
  std::vector<std::shared_ptr<Entity_Access_Op>>    op_access;
  std::vector<std::shared_ptr<Entity_Cast>>         casts;

  bool isDestructible = true;
  bool isMoveable     = true;
  bool isCastable     = true;
  bool isExtCastable  = true;

  [[nodiscard]] bool contains_op(EBinOpType op, const AType* return_type) const;
  [[nodiscard]] bool contains_access_op(EAccessOpType op, const AType* return_type) const;
  [[nodiscard]] bool contains_cast(const AType& target_type, bool isCastFrom) const;
  [[nodiscard]] bool contains_comp(const Component& target_comp) const;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "declaration entity \"" + declaration_name + "\"";
  }

  std::string mangle_type() const override
  {
    return "et." + declaration_name;
  }
  bool compare_with(const AType& other) const override;

  VISTOR_ACCEPT
};

struct Entity_New final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override;
  VISTOR_ACCEPT
};

struct Entity_Del final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "del fn()";
  }
  VISTOR_ACCEPT
};

struct Entity_Cast final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  std::unique_ptr<Node>  source;
  std::shared_ptr<AType> target;

  bool isSourceSelf = false;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "entity cast";
  }
  VISTOR_ACCEPT
};

struct Entity_Op final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  EBinOpType op_ty = EBinOpType::Add;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "entity op " + EBinOpType_to_str(op_ty);
  }
  VISTOR_ACCEPT
};

// the only non boolean operator and Iter operator who can return other type than the entity
struct Entity_Access_Op final : ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  // nullptr = usize by default other non int type = map like Range = always return a Slice
  std::string parameter_name;

  EAccessOpType op_ty = EAccessOpType::Index;


  std::shared_ptr<AType> return_type;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "entity op[index]";
  }
  VISTOR_ACCEPT
};

struct Entity_Transfert : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  ETransfertType transfet;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "entity transfert " + ETransfertType_to_str(transfet);
  }
  VISTOR_ACCEPT
};

struct System final : public ACallable, ADeclaration {
  std::vector<std::shared_ptr<System_Case>> cases;

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override
  {
    return "declaration system \"" + declaration_name + "\"";
  }

  bool manage_entity(const Entity& entity) const;
  bool manage_component(const Component& comp) const;

  VISTOR_ACCEPT
};

struct System_Case final : public Node, Trait_LLVM_Passage {
  // resolved in def_system
  std::shared_ptr<System> parent_system;

  std::vector<std::shared_ptr<local::Variable_Binding>> bindings;
  std::unique_ptr<local::CodeBlock>                     codeblock;

  bool is_return  = false;
  bool is_default = false;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "system case";
  }

  bool manage_entity(const Entity& entity) const;

  bool manage_component(const Component& comp) const;

  VISTOR_ACCEPT
};

} // namespace cop
  // COP
} // namespace declaration
  // Declaration
} // namespace ast
  // AST