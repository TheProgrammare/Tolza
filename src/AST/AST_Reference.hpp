#pragma once

#include "AST_Base.hpp"
#include "AST_Type.hpp"
#include <memory>

namespace AST {
namespace Reference {

struct Enum : public AReference {
    using AReference::AReference;
    std::vector<std::unique_ptr<Node>> member_values;

    std::string debug_str() const override { return "<Lit> enum[" + id.debug_str() + "]"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Member_Access : public AReference {
    std::unique_ptr<AReference> left;
    std::unique_ptr<AReference> right;

    std::string debug_str() const override;

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Self : public Node {
    SYM_DEFINITION self_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "self"; }
};

struct Other : public Node {
    // can be primitive or other entity
    SYM_DEFINITION other_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "other"; }
};


// (10, a, param3 = b, param5 = c)
struct Call_Argument : public Node {
    std::string name;
    std::unique_ptr<Node> val;
    // resolved by superior node
    SYM_DEFINITION function_definition;
    SYM_DEFINITION parameter_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return name; }
};



struct Call : public AReference {
    std::vector<std::unique_ptr<AType>> gen_args;
    std::vector<std::unique_ptr<Call_Argument>> param_args;

    std::string debug_str() const override { return "call[" + id.debug_str() + "]"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }

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
    std::vector<std::unique_ptr<AType>> base_gen_args;
    std::vector<std::vector<std::unique_ptr<AType>>> gen_args;
    std::vector<std::vector<std::unique_ptr<Call_Argument>>> arguments;
    bool isMutable = false;
    std::vector<EBinOpType> mutableOperators;

    std::string debug_str() const override { return "pipecall[" + id.debug_str() + "]"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Table_Access : public AReference {
    // most of time only one arg
    std::unique_ptr<Node> selector;

    std::string debug_str() const override { return "table access"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

}
}