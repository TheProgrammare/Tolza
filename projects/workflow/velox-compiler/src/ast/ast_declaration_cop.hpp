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
  bool                         isNoDefault = false;

  EBorrow borrow = EBorrow::None;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

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
    out += name + ": " + type->debug_str();
    if (default_value) out += " = " + default_value->debug_str();
    return out;
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Component;
  }

  void accept(Visitor_Base& v) override;
};

struct Component final : public ADeclaration, AType {
  [[maybe_unused]]
  std::shared_ptr<local::Generic_Parameter_Element> gen_where;
  std::vector<std::shared_ptr<Component_Field>>     fields;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override;
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Component;
  }

  std::string mangle_type() const override
  {
    return "cp_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Component*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override;
};

struct Role final : public ADeclaration, AType {
  std::vector<std::unique_ptr<AExpression>> components;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "declaration role \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Role;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Role*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  std::string mangle_type() const override
  {
    return "rl_" + mangle_id(name);
  }

  void accept(Visitor_Base& v) override;
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
  std::vector<std::shared_ptr<Entity_Cast>>         casts;

  bool isDestructible = true;
  bool isMoveable     = true;
  bool isCastable     = true;
  bool isExtCastable  = true;

  [[nodiscard]] bool contains_op(EBinOpType op, const AType* return_type) const;
  [[nodiscard]] bool contains_cast(const AType& target_type, bool isCastFrom) const;
  [[nodiscard]] bool contains_comp(const Component& target_comp) const;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "declaration entity \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity;
  }

  std::string mangle_type() const override
  {
    return "et_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override;

  void accept(Visitor_Base& v) override;
};

struct Entity_New final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override;
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_New;
  }
  void accept(Visitor_Base& v) override;
};

struct Entity_Del final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "del fn()";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Del;
  }
  void accept(Visitor_Base& v) override;
};

struct Entity_Cast final : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  std::unique_ptr<Node>  source;
  std::unique_ptr<AType> target;

  bool isSourceSelf = false;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "entity cast";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Cast;
  }
  void accept(Visitor_Base& v) override;
};

struct Entity_Op : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  EBinOpType operatorType = EBinOpType::Add;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "entity op " + EBinOpType_to_str(operatorType);
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Op;
  }
  void accept(Visitor_Base& v) override;
};

// the only non boolean operator and Iter operator who can return other type than the entity
struct Entity_OpIndex final : public Entity_Op {
  // nullptr = usize by default other non int type = map like Range = always return a Slice
  std::string parameter_name;

  std::unique_ptr<AType> return_type;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "entity op[index]";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_OpIndex;
  }
  void accept(Visitor_Base& v) override;
};

struct Entity_Transfert : public ACallable, ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  ETransfertType transfet;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "entity transfert " + ETransfertType_to_str(transfet);
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Transfert;
  }
  void accept(Visitor_Base& v) override;
};

struct System final : public ACallable, ADeclaration {
  std::vector<std::shared_ptr<System_Case>> cases;

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "declaration system \"" + name + "\"";
  }

  bool manage_entity(const Entity& entity) const;
  bool manage_component(const Component& comp) const;

  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::System;
  }

  void accept(Visitor_Base& v) override;
};

struct System_Case final : public Node, Trait_LLVM_Passage {
  // resolved in def_system
  std::shared_ptr<System> parent_system;

  std::vector<std::shared_ptr<local::Variable_Binding>> bindings;
  std::unique_ptr<local::CodeBlock>                     codeblock;

  bool isReturn  = false;
  bool isDefault = false;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "system case";
  }

  bool manage_entity(const Entity& entity) const;

  bool manage_component(const Component& comp) const;

  void accept(Visitor_Base& v) override;
};

} // namespace cop
  // COP
} // namespace declaration
  // Declaration
} // namespace ast
  // AST