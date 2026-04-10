#pragma once

#include <memory>

#include "ast/ast_declaration.hpp"
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
  bool                is_else = false;
  bool                is_elif = false;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  void         accept(Visitor_Base& v) override;

  std::string debug_str() const override
  {
    return is_else ? "else" : is_elif ? "elif" : "if";
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

// normally not an expression
struct GoTo final : public AExpression {
  std::string label;

  SET_R_VAL

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

  ACallable* target_function = nullptr;

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