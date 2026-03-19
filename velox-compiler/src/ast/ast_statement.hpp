#pragma once

#include <llvm-19/llvm/IR/BasicBlock.h>
#include <llvm-19/llvm/IR/Value.h>
#include <memory>

#include "ast_base.hpp"
#include "ast_evaluator.hpp"


namespace ast
{
namespace statement
{

struct If final : public Node, Trait_LLVM_Passage {
  ~If();

  Evaluator evaluator;

  std::unique_ptr<declaration::local::CodeBlock> codeblock;
  [[maybe_unused]]
  std::unique_ptr<If> alternative_statement;
  bool                isElseNoCondition = false;
  bool                isInline          = false;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "IF";
  }
};

// for i in range {}
struct For final : public Node, Trait_LLVM_Passage {
  ~For();

  std::unique_ptr<AExpression> expression;

  [[maybe_unused]]
  std::shared_ptr<declaration::local::Variable_Binding> index;
  [[maybe_unused]]
  std::vector<std::shared_ptr<declaration::local::Variable_Binding>> items;

  std::unique_ptr<declaration::local::CodeBlock> codeblock;
  bool                                           isReverse = false;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override;
};

// loop {...}
struct Loop final : public Node, Trait_LLVM_Passage {
  ~Loop();

  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "LOOP";
  }
};

// while condition {...}
struct While final : public Node, Trait_LLVM_Passage {
  ~While();

  bool                                           isDo = false;
  Evaluator                                      evaluator;
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return "WHILE";
  }
};

struct GoTo_Label;

// goto azerty
struct GoTo final : public AExpression {
  std::string label;


  std::string debug_str() const override
  {
    return "GOTO \"" + label + "\"";
  }

  GoTo_Label* label_sym = nullptr;

  llvm::Value* codegen(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;
};

// label azerty {...}
struct GoTo_Label final : public ADeclaration {
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  llvm::BasicBlock* llvm_bb = nullptr;

  std::string debug_str() const override
  {
    return "LABEL[" + name + "]";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Goto_Label;
  }
};

// return a, b, c
struct Return final : public Node, Trait_LLVM_Passage {
  [[maybe_unused]]
  std::unique_ptr<AExpression> value;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "return";
  }

  void accept(Visitor_Base& v) override;
};

struct Break final : public Node, Trait_LLVM_Passage {
  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "break";
  }

  void accept(Visitor_Base& v) override;
};

struct Continue final : public Node, Trait_LLVM_Passage {
  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "continue";
  }

  void accept(Visitor_Base& v) override;
};

// constant/comparison => {}
struct Match_Case final : public Node, Trait_LLVM_Passage {
  ~Match_Case();

  Evaluator                                      evaluator;
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "CASE";
  }

  void accept(Visitor_Base& v) override;
};

// match <base> { <const/comparison> => {...} _ => {...} }
struct Match final : public Node, Trait_LLVM_Passage {
  std::shared_ptr<AExpression>             base;
  std::vector<std::unique_ptr<Match_Case>> cases;
  [[maybe_unused]]
  std::unique_ptr<Match_Case> other_case;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "MATCH";
  }

  void accept(Visitor_Base& v) override;
};

} // namespace statement
  // Statement
} // namespace ast
  // AST