#pragma once

#include "AST_Base.hpp"
#include "AST_Type.hpp"

namespace AST {
namespace Reference {

struct Enum : public AReference {
    using AReference::AReference;
    std::vector<std::unique_ptr<Node>> member_values;

    SYM_DECL<Declaration::Enum> resolved_sym;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<Lit> enum[" + id.debug_str() + "]"; }
    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return std::static_pointer_cast<ADeclaration>(resolved_sym.ptr); }
};

struct Member_Access : public AReference {
    SYM_DECL<ADeclaration> resolved_sym;

    std::unique_ptr<AReference> left;
    std::unique_ptr<AReference> right;

    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym.ptr; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override;
};

struct Self : public Node {
    SYM_DECL<Declaration::COP::Entity> source_sym;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "self"; }
};

struct Other : public Node {
    // can be primitive or other entity
    SYM_TYPE<AType> resolved_type;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "other"; }
};


// (10, a, param3 = b, param5 = c)
struct Call_Argument : public Node {
    std::string name;
    std::unique_ptr<Node> val;
    // resolved by superior node
    SYM_DECL<ADeclaration> resolved_symbol;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return name; }
};



struct Call : public AReference {
    std::unique_ptr<Type_Arguments> gen_args;
    std::vector<std::unique_ptr<Call_Argument>> param_args;

    // symbol resolution
    // function/lambda/system/enum
    SYM_DECL<ADeclaration> resolved_sym;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "call[" + id.debug_str() + "]"; }
    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym.ptr; }

    bool to_lit_enum(Enum& lit_enum) {
        lit_enum.id = ID(id.path, id.name);
        lit_enum.member_values.reserve(param_args.size());
        for (auto& param : param_args) {
            lit_enum.member_values.push_back(std::move(param->val));
        }
        return true;
    }
};

struct Call_System : public Call {
    std::unique_ptr<AReference> target_entity;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "run"; }
};

struct Call_Pipe : public AReference {
    std::unique_ptr<Type_Arguments> base_gen_args;
    std::vector<std::unique_ptr<Type_Arguments>> gen_args;
    std::vector<std::vector<std::unique_ptr<Call_Argument>>> arguments;
    bool isMutable = false;
    std::vector<EBinOpType> mutableOperators;

    // symbol resolution
    SYM_DECL<ADeclaration> resolved_sym;

    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym.ptr; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "pipecall[" + id.debug_str() + "]"; }
};

struct Table_Access : public AReference {
    // most of time only one arg
    std::unique_ptr<Node> selector;

    SYM_TYPE<AType> result_type_resolution;

    SYM_DECL<ADeclaration> resolved_sym;

    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym.ptr; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "table access"; }
};

}
}