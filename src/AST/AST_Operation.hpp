#pragma once

#include "AST/AST_Type.hpp"
#include "AST_Base.hpp"
#include <memory>

namespace AST {
namespace Operation {


struct Cast_As : public AExpression {
    std::unique_ptr<AExpression> valueCasted;
    std::unique_ptr<AType> typeCasted;

    enum class ECastType { AS, AS_REINTERPRET, AS_SAFE };

    ECastType cast_type = ECastType::AS;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        switch (cast_type) {
            case ECastType::AS: return "as";
            case ECastType::AS_REINTERPRET: return "as!";
            case ECastType::AS_SAFE: return "as?";
        }
    }
};

struct Is : public AExpression {
    std::unique_ptr<AExpression> left;
    std::unique_ptr<AExpression> right;

    Is();

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "is"; }
};

struct In : public AExpression {
    std::unique_ptr<AExpression> left;
    std::unique_ptr<AExpression> right;

    In();

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "in"; }
};


// a copy= b | a clone= b | a move= b | a ref= b | a mut= b 
struct Assignment : public Node {
    std::unique_ptr<AExpression> left;
    std::unique_ptr<AExpression> right;
    EAssignmentType assignmentType = EAssignmentType::Copy;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        switch (assignmentType) {
        case EAssignmentType::Copy:			return "copy=";
        case EAssignmentType::Clone:	    return "clone=";
        case EAssignmentType::MoveSemantic:	return "move=";
        case EAssignmentType::NONE:         return "NO ASSIGNMENT TYPE";
        }
    }
};



// a op b
struct Binary : public AExpression {
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    EBinOpType op = EBinOpType::Add;

    SYM_DEFINITION type_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<op> bin(" + EBinOpType_to_str(op) + ")"; }
};

// ++a a++ --a a-- !a +a -a
struct Unary : public AExpression {
    std::unique_ptr<Node> base;
    EUnaryOpType unitaryOp = EUnaryOpType::Incr;
    // for pre increment/decrement or sign
    bool pre_operator = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<op> unary(" + EUnaryOpType_to_str(unitaryOp) + ")"; }
};

// a </<= b >/>= c
struct Interval : public AExpression {
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> center;
    std::unique_ptr<Node> right;
    EBinOpType left_comparator = EBinOpType::Low;
    EBinOpType right_comparator = EBinOpType::Low;

    Interval();

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "interval left[" + EBinOpType_to_str(left_comparator) + "] right[" + EBinOpType_to_str(right_comparator) + "]"; }
};

// p1 <-> p2
struct Ptr_Dist : public AExpression {
    std::unique_ptr<AExpression> left;
    std::unique_ptr<AExpression> right;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<mem> distance"; }
};

}
}