#pragma once

#include <memory>

#include "AST_Base.hpp"
#include "AST_Type.hpp"
#include "AST_Evaluator.hpp"

namespace AST {
namespace Expression {

struct If_Ternary final : public AExpression {
    Evaluator evaluator;
    std::unique_ptr<Node> true_line;
    [[maybe_unused]]
    std::unique_ptr<Node> false_line;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "ternary if"; }
};

struct Enum final : public AExpression {
    std::string name;
    std::vector<std::unique_ptr<AExpression>> member_values;

    std::string debug_str() const override { return "literal enum \"" + name + "\""; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Member_Access final : public AExpression {
    std::unique_ptr<AExpression> left;
    std::unique_ptr<AExpression> right;

    std::string debug_str() const override {
        return left->debug_str() + "." + right->debug_str();
    }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Self final : public AExpression {
    SYM_DEFINITION self_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "self"; }
};

struct Other final : public AExpression {
    // can be primitive or other entity
    SYM_DEFINITION other_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "other"; }
};


// (10, a, param3 = b, param5 = c)
struct Call_Argument final : public AExpression {
    [[maybe_unused]]
    std::string name;
    std::unique_ptr<AExpression> expression;
    // resolved by superior node
    SYM_DEFINITION function_definition;
    SYM_DEFINITION parameter_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return name; }
};



struct Call : public AExpression {
    std::string name;
    std::vector<std::unique_ptr<AType>> gen_args;
    std::vector<std::unique_ptr<Call_Argument>> param_args;

    std::string debug_str() const override { return "call \"" + name + "\""; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }

    bool to_lit_enum(Enum& lit_enum) {
        lit_enum.name = name;
        lit_enum.member_values.reserve(param_args.size());
        for (auto& param : param_args) {
            lit_enum.member_values.push_back(std::move(param->expression));
        }
        return true;
    }
};

struct Call_System : public Call {
    std::unique_ptr<AExpression> target_entity;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "run"; }
};

struct Call_Pipe : public AExpression {
    std::string name;
    std::vector<std::unique_ptr<AType>> base_gen_args;
    std::vector<std::vector<std::unique_ptr<AType>>> gen_args;
    std::vector<std::vector<std::unique_ptr<Call_Argument>>> arguments;
    bool isMutable = false;
    std::vector<EBinOpType> mutableOperators;

    std::string debug_str() const override { return "pipecall \"" + name + "\""; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Table_Access : public AExpression {
    // most of time only one arg
    std::unique_ptr<Node> selector;

    std::string debug_str() const override { return "table access"; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


// my_ptr'at(i)
struct Ptr_At : public AExpression {
    std::unique_ptr<AExpression> target;

    std::unique_ptr<AExpression> index;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "ptr at"; }
};

// my_ptr'offset(i)
struct Ptr_Offset : public AExpression {
    std::unique_ptr<AExpression> target;

    std::unique_ptr<AExpression> offset;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "ptr offset"; }
};

// val'my_ptr
struct Ptr_Val : public AExpression {
    std::unique_ptr<AExpression> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "val'"; }
};

// addr'my_val
struct Addr_Of : public AExpression {
    std::unique_ptr<AExpression> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "addr'"; }
};

struct Size_Of : public AExpression {
    std::unique_ptr<AExpression> target;
    size_t size = 0;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<mem> size(" + std::to_string(size) + ")"; }
};

// target~[0..8] | target~[16..24]
struct GetBits : public AExpression {
    std::unique_ptr<AExpression> target;
    std::unique_ptr<AExpression> range;

    std::shared_ptr<AType> resolved_range_type;
    // 8, 16, 32, 64, 128
    enum EBitSize { _8, _16, _32, _64, _128 };
    EBitSize bit_size = _8;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "bits get"; }
};

// move'p
struct Move : public AExpression {
    std::unique_ptr<Node> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "move"; }
};

// new ptr'T(val)
struct New_Ptr : public AExpression {
    EPtrType pointer = EPtrType::raw_ptr;
    std::shared_ptr<AType> type;
    std::unique_ptr<Node> expression;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override {  return "new"; }
};

}
}