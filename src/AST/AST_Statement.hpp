#pragma once

#include <memory>

#include "AST_Base.hpp"
#include "AST_Evaluator.hpp"
#include "AST_Forward.hpp"

namespace AST
{
namespace Statement
{

struct If final : public Node {
  Evaluator evaluator;

  std::unique_ptr<Declaration::Local::CodeBlock> codeblock;
  [[maybe_unused]]
  std::unique_ptr<If> alternative_statement;
  bool                isElseNoCondition = false;
  bool                isInline          = false;

  void        accept(Visitor_Base& v) override { v.visit(*this); }
  std::string debug_str() const override { return "IF"; }
};

// for i in range {}
struct For final : public Node {
  std::unique_ptr<AExpression> src;

  [[maybe_unused]]
  std::shared_ptr<Declaration::Local::Parameter> index;
  [[maybe_unused]]
  std::vector<std::shared_ptr<Declaration::Local::Parameter>> items;

  std::unique_ptr<Declaration::Local::CodeBlock> codeblock;
  bool                                           isReverse = false;

  SYM_DEFINITION type_item_definition;
  SYM_DEFINITION type_key_definition;

  void        accept(Visitor_Base& v) override { v.visit(*this); }
  std::string debug_str() const override;
};

// loop {...}
struct Loop final : public Node {
  std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

  void        accept(Visitor_Base& v) override { v.visit(*this); }
  std::string debug_str() const override { return "LOOP"; }
};

// while condition {...}
struct While final : public Node {
  bool                                           isDo = false;
  Evaluator                                      evaluator;
  std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

  void        accept(Visitor_Base& v) override { v.visit(*this); }
  std::string debug_str() const override { return "WHILE"; }
};

// goto azerty
struct GoTo final : public AExpression {
  std::string label;
  std::string debug_str() const override { return "GOTO \"" + label + "\""; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

// label azerty:
struct GoTo_Label final : public ADeclaration {
  void        accept(Visitor_Base& v) override { v.visit(*this); }
  std::string debug_str() const override { return "LABEL[" + name + "]"; }
  ESymbolType get_symbol_type() const override { return ESymbolType::Goto_Label; }
};

// return a, b, c;
struct Return final : public Node {
  [[maybe_unused]]
  std::unique_ptr<AExpression> value;

  std::string debug_str() const override { return "<inst> return"; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Break final : public Node {
  std::string debug_str() const override { return "break"; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Continue final : public Node {
  std::string debug_str() const override { return "continue"; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

// constant/comparison then {}
struct Match_Case final : public Node {
  Evaluator                                      evaluator;
  std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

  std::string debug_str() const override { return "CASE"; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

// match <base> { <const/comparison> => {...} _ => {...} }
struct Match final : public Node {
  std::shared_ptr<AExpression>             base;
  std::vector<std::unique_ptr<Match_Case>> cases;
  [[maybe_unused]]
  std::unique_ptr<Match_Case> other_case;

  std::string debug_str() const override { return "MATCH"; }

  void accept(Visitor_Base& v) override { v.visit(*this); }
};

} // namespace Statement
} // namespace AST