#pragma once

#include <memory>

#include "ast_base.hpp"


namespace ast
{
namespace operation
{
struct Cast_As final : public AExpression {
  std::unique_ptr<AExpression> valueCasted;
  std::unique_ptr<AType>       typeCasted;

  enum class ECastType { AS, AS_REINTERPRET, AS_SAFE };

  ECastType cast_type = ECastType::AS;

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    switch (cast_type) {
    case ECastType::AS:             return "cast as";
    case ECastType::AS_REINTERPRET: return "cast as! unsafe";
    case ECastType::AS_SAFE:        return "cast as? safe";
    }
  }
};

struct Is final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;

  Is();

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "is";
  }
};

struct In final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;

  In();

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "in";
  }
};

// a copy= b | a clone= b | a move= b | a ref= b | a mut= b
struct Assignment final : public Node {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;
  EAssignmentType              assignmentType = EAssignmentType::Copy;

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    switch (assignmentType) {
    case EAssignmentType::Copy:         return "copy=";
    case EAssignmentType::Clone:        return "clone=";
    case EAssignmentType::MoveSemantic: return "move=";
    case EAssignmentType::NONE:         return "=";
    }
  }
};

// a op b
struct Binary final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;
  EBinOpType                   op = EBinOpType::Add;

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "op bin(" + EBinOpType_to_str(op) + ")";
  }
};

// !a
struct Unary final : public AExpression {
  std::unique_ptr<AExpression> base;
  EUnaryOpType                 unitaryOp    = EUnaryOpType::_not;
  // for
  // pre
  // increment/decrement
  // or
  // sign
  bool                         pre_operator = false;

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "op unary(" + EUnaryOpType_to_str(unitaryOp) + ")";
  }
};

// a </<=
// b >/>=
// c
struct Interval final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> center;
  std::unique_ptr<AExpression> right;
  EBinOpType                   left_comparator  = EBinOpType::Low;
  EBinOpType                   right_comparator = EBinOpType::Low;

  Interval();

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "interval left[" + EBinOpType_to_str(left_comparator) + "] right[" + EBinOpType_to_str(right_comparator)
           + "]";
  }
};

// p1 <->
// p2
struct Ptr_Dist final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;

  Ptr_Dist();

  void accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "ptr distance";
  }
};

} // namespace operation
  // Operation
} // namespace ast
  // AST