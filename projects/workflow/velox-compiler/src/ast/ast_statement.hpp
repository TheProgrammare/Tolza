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

  CODEGEN_PASS
  VISTOR_ACCEPT

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
  bool                                           is_reverse = false;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

// loop {...}
struct Loop final : public Node, Trait_LLVM_Passage {
  ~Loop();

  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "LOOP";
  }
};

// while condition {...}
struct While final : public Node, Trait_LLVM_Passage {
  ~While();

  bool                                           is_do = false;
  Evaluator                                      evaluator;
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  CODEGEN_PASS
  VISTOR_ACCEPT

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

  CODEGEN_VALUE
  VISTOR_ACCEPT
};

// label azerty {...}
struct GoTo_Label final : public ADeclaration {
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  CODEGEN_PASS
  VISTOR_ACCEPT

  llvm::BasicBlock* llvm_bb = nullptr;

  std::string debug_str() const override
  {
    return "LABEL[" + declaration_name + "]";
  }
};

// return a, b, c
struct Return final : public Node, Trait_LLVM_Passage {
  [[maybe_unused]]
  std::unique_ptr<AExpression> value;

  ACallable* target_function = nullptr;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "return";
  }

  VISTOR_ACCEPT
};

struct Break final : public Node, Trait_LLVM_Passage {
  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "break";
  }

  VISTOR_ACCEPT
};

struct Continue final : public Node, Trait_LLVM_Passage {
  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "continue";
  }

  VISTOR_ACCEPT
};

// constant/comparison => {}
struct Match_Case final : public Node, Trait_LLVM_Passage {
  ~Match_Case();

  Evaluator                                      evaluator;
  std::unique_ptr<declaration::local::CodeBlock> codeblock;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "CASE";
  }

  VISTOR_ACCEPT
};

// match <base> { <const/comparison> => {...} _ => {...} }
struct Match final : public Node, Trait_LLVM_Passage {
  std::shared_ptr<AExpression>             base;
  std::vector<std::unique_ptr<Match_Case>> cases;
  [[maybe_unused]]
  std::unique_ptr<Match_Case> other_case;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "MATCH";
  }

  VISTOR_ACCEPT
};

} // namespace statement
  // Statement
} // namespace ast
  // AST