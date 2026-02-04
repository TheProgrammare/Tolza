#pragma once

#include "AST_Base.hpp"
#include "AST_Literal.hpp"
#include "AST_Type.hpp"

namespace AST {
namespace Declaration {
namespace ECS {

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

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "field[" + id.debug_str() + "]"; }
};

struct Component : public ADeclaration, AType {
    [[maybe_unused]]
    std::shared_ptr<Local::Generic_Parameter> gen_where;
    std::vector<std::shared_ptr<Component_Field>> fields;

    [[nodiscard]] std::string mangle_type() const override { return "comp"; }
    [[nodiscard]] std::string debug_str() const override { return "<ty> comp[" + id.debug_str() + "]"; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Role : public ADeclaration, AType {
    std::vector<std::unique_ptr<AReference>> components;

    [[nodiscard]] std::string mangle_type() const override { return "rl"; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<ty> role[" + id.debug_str() + "]"; }
};

struct Entity;

struct Entity_Cast : public ADeclaration {
    std::shared_ptr<Entity> entity_sym;

    std::unique_ptr<Node> source;
    std::unique_ptr<AType> target;

    bool isSourceSelf = false;

    std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<cast> entity"; }
};


struct Entity_Op : public ADeclaration {
    std::shared_ptr<Entity> entity_sym;

    EBinOpType operatorType = EBinOpType::Add;

    std::unique_ptr<AST::Declaration::Local::CodeBlock> codeblock;

    bool resolved_result_type_isRef = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return EBinOpType_to_str(operatorType); }
};

// the only non boolean operator and Iter operator who can return other type than the entity
struct Entity_OpIndex : public Entity_Op {
    // nullptr = usize by default
    // other non int type = map like 
    // Range = always return a Slice
    std::string parameter_name;

    std::unique_ptr<AType> return_type;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "[...]"; }
};

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
    [[nodiscard]] std::string mangle_type() const override { return "et"; }
    [[nodiscard]] std::string debug_str() const override { return "<ty> entity[" + id.debug_str() + "]"; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    bool contains_op(EBinOpType op, const AType* return_ty) const {
        for (auto& elem : operators) {
            if (elem->operatorType == op) {
                return true;
            }
        }
        return false;
    }
    [[nodiscard]] bool contains_cast(const AType& target_ty, bool isCastFrom) const {
        // difficult resolution:
        // entity have 2 cast way: cast self as T / cast T as self
        // generic have 2 cast way check: T cast to U / T cast from U

        if (isCastFrom) {
            for (auto& elem : casts) {
                if (!elem->isSourceSelf) {
                    if (auto id_ty_ptr = dynamic_cast<const AType*>(elem->source.get())) {
                        if (target_ty == *id_ty_ptr) {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        else {
            for (auto& elem : casts) {
                if (elem->isSourceSelf && *elem->target == target_ty) return true;
            }
            return false;
        }
    }
    [[nodiscard]] bool contains_comp(const Component& target_comp) const {
        for (auto& comp : comps) {
            if (comp->resolved_sym->id == target_comp.id) return true;
        }
        return false;
    }
};

struct System_Case : public ADeclaration {
    // resolved in def_system
    std::shared_ptr<System> sys_symbol;

    std::vector<std::shared_ptr<Local::Variable_Binding>> bindings;
    std::unique_ptr<Local::CodeBlock> codeblock;

    bool isReturn = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<def> with"; }

    [[nodiscard]] bool manage_entity(const Entity& entity) const;

    [[nodiscard]] bool manage_component(const Component& comp) const;
};



struct System : public ADeclaration, ICallable {
    std::shared_ptr<Type::Function_Proto> prototype;
    std::vector<std::shared_ptr<System_Case>> cases;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<def> system[" + id.debug_str() + "]"; }

    [[nodiscard]] bool manage_entity(const Entity& entity) const {
        for (auto& with : cases) {
            if (with->manage_entity(entity)) return true;
        }
        return false;
    }
    [[nodiscard]] bool manage_component(const Component& comp) const {
        for (auto& with : cases) {
            if (with->manage_component(comp)) return true;
        }
        return false;
    }
    [[nodiscard]] Type::Function_Proto* get_signature() override { return prototype.get(); };
};

}
}
}