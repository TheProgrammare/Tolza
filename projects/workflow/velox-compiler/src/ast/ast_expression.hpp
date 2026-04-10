#pragma once


#include <memory>

#include "ast/ast_declaration_local.hpp"
#include "ast/ast_forward.hpp"
#include "ast/ast_type.hpp"
#include "ast_base.hpp"
#include "ast_evaluator.hpp"

namespace ast
{
namespace expression
{

struct If_Ternary final : public AExpression {
  Evaluator                    evaluator;
  std::unique_ptr<AExpression> true_line;
  [[maybe_unused]]
  std::unique_ptr<AExpression> false_line;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;


  std::string debug_str() const override
  {
    return "ternary if";
  }
};


struct Member_Access final : public AExpression {
  std::unique_ptr<AExpression> left;
  std::unique_ptr<AIdentifier> right;

  SET_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "access " + left->debug_str() + "." + right->debug_str();
  }
  void accept(Visitor_Base& v) override;
};

struct Self final : public AExpression {
  std::shared_ptr<declaration::cop::Entity> self_definition;

  SET_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "self";
  }
};

struct Other final : public AExpression {
  SET_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;


  std::string debug_str() const override
  {
    return "other";
  }
};

// (10, a, param3 = b, param5 = c)
struct Call_Argument final : public AExpression {
  [[maybe_unused]] std::string name;
  std::unique_ptr<AExpression> expression;
  // resolved by superior node
  ast::type::Function_Proto*   fn_type = nullptr;
  [[maybe_unused]]
  ast::declaration::local::Parameter* fn_param_type = nullptr;

  bool variadic_arg = false;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    std::string out;
    if (!name.empty()) out += name + " = ";

    return out + expression->debug_str();
  }
};

struct Call : public AExpression {
  std::unique_ptr<AExpression>                callee;
  std::vector<std::shared_ptr<AType>>         gen_args;
  std::vector<std::unique_ptr<Call_Argument>> param_args;

  SYM_REF                                    function_symbol;
  std::shared_ptr<ast::type::Function_Proto> function_proto;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override;

  void accept(Visitor_Base& v) override;
};

struct Call_System final : public Call {
  std::unique_ptr<AExpression> target_entity;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "run[" + target_entity->debug_str() + "::&gt;" + callee->debug_str() + "]";
  }
};

struct Call_Pipe final : public AExpression {
  std::unique_ptr<AIdentifier>                             callee;
  std::vector<std::shared_ptr<AType>>                      base_gen_args;
  std::vector<std::vector<std::shared_ptr<AType>>>         gen_args;
  std::vector<std::vector<std::unique_ptr<Call_Argument>>> arguments;
  bool                                                     isMutable = false;
  std::vector<EBinOpType>                                  mutableOperators;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "pipecall[" + callee->debug_str() + "]";
  }
  void accept(Visitor_Base& v) override;
};

// a[i]
struct Table_Access final : public AExpression {
  // most of time only one arg
  std::unique_ptr<AExpression> target;
  std::unique_ptr<AExpression> selector;

  SET_L_VAL


  bool bounded = false;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "table access";
  }
  void accept(Visitor_Base& v) override;
};

// my_ptr'at(i)
struct Ptr_At final : public AExpression {
  std::unique_ptr<AExpression> target;
  std::unique_ptr<AExpression> index;

  SET_L_VAL


  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "ptr at";
  }
};

// my_ptr'offset(i)
struct Ptr_Offset final : public AExpression {
  std::unique_ptr<AExpression> target;
  std::unique_ptr<AExpression> offset;

  SET_L_VAL


  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "ptr offset";
  }
};

// val'my_ptr
struct Ptr_Val final : public AExpression {
  std::unique_ptr<AExpression> target;

  SET_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "val of";
  }
};

// mut'my_val
struct Mut_Of final : public AExpression {
  std::unique_ptr<AExpression> target;

  SET_L_VAL
  SET_FORCED_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "mut of";
  }
};

// ref'my_val
struct Ref_Of final : public AExpression {
  std::unique_ptr<AExpression> target;

  SET_L_VAL
  SET_FORCED_L_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "ref of";
  }
};

// addr'my_val
struct Addr_Of final : public AExpression {
  std::unique_ptr<AExpression> target;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  SET_R_VAL

  std::string debug_str() const override
  {
    return "addr of";
  }
};

struct Size_Of final : public AExpression {
  std::unique_ptr<AExpression> target;
  size_t                       size = 0;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  SET_R_VAL

  std::string debug_str() const override
  {
    return "size of (" + std::to_string(size) + ")";
  }
};

// target~[0..8] | target~[16..24]
struct GetBits final : public AExpression {
  std::unique_ptr<AExpression> target;
  std::unique_ptr<AExpression> range;

  // 8, 16, 32, 64, 128
  enum EBitSize { _8, _16, _32, _64, _128 };
  EBitSize bit_size = _8;

  SET_R_VAL

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "bits get";
  }
};

// move'p
struct Move final : public AExpression {
  std::unique_ptr<Node> target;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  SET_L_VAL

  std::string debug_str() const override
  {
    return "move";
  }
};

// new ptr'T(val)
struct New_Ptr final : public AExpression {
  std::shared_ptr<AType> type;
  std::unique_ptr<Node>  expression;

  EPtrType pointer = EPtrType::raw_ptr;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  SET_L_VAL

  std::string debug_str() const override
  {
    return "new";
  }
};

} // namespace expression
  // Expression
} // namespace ast
  // AST