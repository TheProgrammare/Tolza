#pragma once

#include "AST_Base.hpp"
#include "AST_Literal.hpp"
#include "AST_Type.hpp"
#include "Compiler/Visitor/Symbol_Manager.hpp"
#include <memory>

namespace AST
{
namespace Declaration
{
namespace COP
{

struct Component_Field final : public ADeclaration {
  std::shared_ptr<Component> parent_component;

  std::unique_ptr<AType>       type;
  std::unique_ptr<AExpression> default_value;
  bool                         isNoDefault = false;

  std::string debug_str() const override
  {
    return "field \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Component;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Component final : public ADeclaration {
  [[maybe_unused]]
  std::shared_ptr<Local::Generic_Parameter>     gen_where;
  std::vector<std::shared_ptr<Component_Field>> fields;

  std::string debug_str() const override
  {
    return "declaration component \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Component;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Role final : public ADeclaration {
  std::vector<std::unique_ptr<AExpression>> components;

  std::string debug_str() const override
  {
    return "declaration role \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Role;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Entity_Op;
struct Entity_Cast;

struct Entity final : public ADeclaration {
  std::vector<std::unique_ptr<Literal::Component>> comps;

  // fn type, lines
  std::vector<std::tuple<std::shared_ptr<Type::Function_Proto>, std::unique_ptr<Local::CodeBlock>>> constructors;
  std::vector<std::unique_ptr<Node>>                                                                destructor;

  std::shared_ptr<Local::Generic_Parameter> gen_params;
  std::vector<std::shared_ptr<Entity_Op>>   operators;
  std::vector<std::shared_ptr<Entity_Cast>> casts;

  bool isDestructible = true;
  bool isMoveable     = true;
  bool isCastable     = true;
  bool isExtCastable  = true;

  [[nodiscard]] bool contains_op(EBinOpType op, const AType* return_type) const;
  [[nodiscard]] bool contains_cast(const AType& target_type, bool isCastFrom) const;
  [[nodiscard]] bool contains_comp(const Component& target_comp) const;

  std::string debug_str() const override
  {
    return "declaration entity \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Entity_Cast final : public ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  std::unique_ptr<Node>  source;
  std::unique_ptr<AType> target;

  bool isSourceSelf = false;

  std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

  std::string debug_str() const override
  {
    return "entity cast";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Cast;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Entity_Op : public ADeclaration {
  std::shared_ptr<Entity> parent_entity;

  EBinOpType operatorType = EBinOpType::Add;

  std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

  bool resolved_result_type_isRef = false;

  std::string debug_str() const override
  {
    return "entity op " + EBinOpType_to_str(operatorType);
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_Op;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

// the only non boolean operator and Iter operator who can return other type than the entity
struct Entity_OpIndex final : public Entity_Op {
  // nullptr = usize by default other non int type = map like Range = always return a Slice
  std::string parameter_name;

  std::unique_ptr<AType> return_type;

  std::string debug_str() const override
  {
    return "entity op[index]";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Entity_OpIndex;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct System final : public ADeclaration, ICallable {
  std::shared_ptr<Type::Function_Proto>     prototype;
  std::vector<std::shared_ptr<System_Case>> cases;

  std::string debug_str() const override
  {
    return "declaration system \"" + name + "\"";
  }

  bool                  manage_entity(const Entity& entity) const;
  bool                  manage_component(const Component& comp) const;
  Type::Function_Proto* get_signature() override
  {
    return prototype.get();
  };
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::System;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct System_Case final : public ADeclaration {
  // resolved in def_system
  std::shared_ptr<System> parent_system;

  std::vector<std::shared_ptr<Local::Variable_Binding>> bindings;
  std::unique_ptr<Local::CodeBlock>                     codeblock;

  bool isReturn  = false;
  bool isDefault = false;

  std::string debug_str() const override
  {
    return "system case";
  }

  bool manage_entity(const Entity& entity) const;

  bool        manage_component(const Component& comp) const;
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::System_Case;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

} // namespace
  // COP
} // namespace
  // Declaration
} // namespace
  // AST