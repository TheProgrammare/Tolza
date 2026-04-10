#pragma once


#include <memory>

#include "ast_base.hpp"


namespace ast
{
namespace operation
{
struct Cast_As final : public AExpression {
  std::unique_ptr<AExpression> expression;
  std::shared_ptr<AType>       type;

  enum class ECastType { AS, AS_REINTERPRET, AS_SAFE };

  ECastType cast_type = ECastType::AS;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

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

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "is";
  }
};

struct In final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;

  In();

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "in";
  }
};

// a copy= b | a clone= b | a move= b | a ref= b | a mut= b
struct Assignment final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;
  ETransfertType               assignmentType = ETransfertType::Copy;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    switch (assignmentType) {
    case ETransfertType::Copy:         return "copy=";
    case ETransfertType::Clone:        return "clone=";
    case ETransfertType::MoveSemantic: return "move=";
    case ETransfertType::NONE:         return "=";
    }
  }
};

// a op b
struct Binary final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AExpression> right;
  EBinOpType                   op = EBinOpType::Add;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "op bin(" + EBinOpType_to_str(op) + ")";
  }
};

// !a
struct Unary final : public AExpression {
  std::unique_ptr<AExpression> base;
  EUnaryOpType                 unary_op     = EUnaryOpType::_not;
  // for
  // pre
  // increment/decrement
  // or
  // sign
  bool                         pre_operator = false;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "op unary(" + EUnaryOpType_to_str(unary_op) + ")";
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

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

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

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "ptr distance";
  }
};

} // namespace operation
  // Operation
} // namespace ast
  // AST