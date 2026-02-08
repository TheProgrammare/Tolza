#pragma once

#include "AST_Base.hpp"
#include "AST_Literal.hpp"
#include "AST_Type.hpp"

namespace AST {
namespace Declaration {
namespace COP {

struct Component_Field : public ADeclaration {
    ID id;
    std::unique_ptr<AType> ty;
    std::unique_ptr<Node> default_value;
    bool isNoDefault = false;

    bool operator==(const Component_Field& other) const {
        return
                id == other.id &&
            *ty == *other.ty &&
            isNoDefault == other.isNoDefault;
    }

    std::string debug_str() const override { return "field[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Component; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Component : public ADeclaration, AType {
    [[maybe_unused]]
    std::shared_ptr<Local::Generic_Parameter> gen_where;
    std::vector<std::shared_ptr<Component_Field>> fields;

    std::string mangle_type() const override { return "comp"; }
    std::string debug_str() const override { return "<ty> comp[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Component; }
    EPrimType get_type() const override { return EPrimType::Component; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Role : public ADeclaration, AType {
    std::vector<std::unique_ptr<AReference>> components;

    std::string mangle_type() const override { return "rl"; }
    std::string debug_str() const override { return "<ty> role[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Role; }
    EPrimType get_type() const override { return EPrimType::Role; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Entity_Op;
struct Entity_Cast;

struct Entity : public ADeclaration, AType {
    std::vector<std::unique_ptr<Literal::Component>> comps;

    // fn type, lines 
    std::vector<std::tuple<std::shared_ptr<Type::Function_Proto>, std::unique_ptr<Local::CodeBlock>>> constructors;
    std::vector<std::unique_ptr<Node>> destructor;

    std::shared_ptr<Local::Generic_Parameter> gen_params;
    std::vector<std::shared_ptr<Entity_Op>> operators;
    std::vector<std::shared_ptr<Entity_Cast>> casts;

    bool isDestructible = true;
    bool isMoveable = true;
    bool isCastable = true;
    bool isExtCastable = true;

    // faire une injection de nomenclature
    std::string mangle_type() const override { return "et"; }
    std::string debug_str() const override { return "<ty> entity[" + id.debug_str() + "]"; }
    [[nodiscard]] bool contains_op(EBinOpType op, const AType* return_ty) const;
    [[nodiscard]] bool contains_cast(const AType& target_ty, bool isCastFrom) const;
    [[nodiscard]] bool contains_comp(const Component& target_comp) const;
    ESymbolType get_symbol_type() const override { return ESymbolType::Entity; }
    EPrimType get_type() const override { return EPrimType::Entity; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Entity_Cast : public ADeclaration {
    SYM_DECL<Entity> entity_sym;

    std::unique_ptr<Node> source;
    std::unique_ptr<AType> target;

    bool isSourceSelf = false;

    std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<cast> entity"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Entity_Cast; }
};


struct Entity_Op : public ADeclaration {
    SYM_DECL<Entity> entity_sym;

    EBinOpType operatorType = EBinOpType::Add;

    std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

    bool resolved_result_type_isRef = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return EBinOpType_to_str(operatorType); }
    ESymbolType get_symbol_type() const override { return ESymbolType::Entity_Op; }
};

// the only non boolean operator and Iter operator who can return other type than the entity
struct Entity_OpIndex : public Entity_Op {
    // nullptr = usize by default
    // other non int type = map like 
    // Range = always return a Slice
    std::string parameter_name;

    std::unique_ptr<AType> return_type;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "[...]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Entity_OpIndex; }
};

struct System : public ADeclaration, ICallable {
    std::shared_ptr<Type::Function_Proto> prototype;
    std::vector<std::shared_ptr<System_Case>> cases;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<def> system[" + id.debug_str() + "]"; }

    bool manage_entity(const Entity& entity) const;
    bool manage_component(const Component& comp) const;
    Type::Function_Proto* get_signature() override { return prototype.get(); };
    ESymbolType get_symbol_type() const override { return ESymbolType::System; }
};


struct System_Case : public ADeclaration {
    // resolved in def_system
    SYM_DECL<System> sys_symbol;

    std::vector<std::shared_ptr<Local::Variable_Binding>> bindings;
    std::unique_ptr<Local::CodeBlock> codeblock;

    bool isReturn = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<def> with"; }

    bool manage_entity(const Entity& entity) const;

    bool manage_component(const Component& comp) const;
    ESymbolType get_symbol_type() const override { return ESymbolType::System_Case; }
};


}
}
}