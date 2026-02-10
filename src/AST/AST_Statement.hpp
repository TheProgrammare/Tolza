#pragma once

#include <memory>
#include <variant>

#include "AST_Base.hpp"
#include "AST_Forward.hpp"
#include "AST_Evaluator.hpp"

namespace AST {
namespace Statement {
    
struct If : public Node {
    Evaluator evaluator;
    
    std::unique_ptr<Declaration::Local::CodeBlock> codeblock;
    [[maybe_unused]]
    std::unique_ptr<If> alternative_statement;
    bool isElseNoCondition = false;
    bool isInline = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "IF"; }
};

struct If_Ternary : public Node {
    Evaluator evaluator;
    std::unique_ptr<Node> true_line;
    [[maybe_unused]]
    std::unique_ptr<Node> false_line;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "TER IF"; }
};


// for i in range {}
struct For : public Node {
    std::unique_ptr<Node> src;

    [[maybe_unused]]
    std::shared_ptr<Declaration::Local::Parameter> index;
    [[maybe_unused]]
    std::vector<std::shared_ptr<Declaration::Local::Parameter>> items;

    std::unique_ptr<Declaration::Local::CodeBlock> codeblock;
    bool isReverse = false;

    SYM_DEFINITION type_item_definition;
    SYM_DEFINITION type_key_definition;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override;
};

// loop {...}
struct Loop : public Node {
    std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "LOOP"; }
};

// while condition {...}
struct While : public Node {
    bool isDo = false;
    Evaluator evaluator;
    std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "WHILE"; }
};

// goto azerty
struct GoTo : public AReference {
    std::string debug_str() const override { return "GOTO[" + id.debug_str() + "]"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// label azerty:
struct GoTo_Label : public ADeclaration {
    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "LABEL[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Goto_Label; }
};


// return a, b, c;
struct Return : public Node {
    [[maybe_unused]]
    std::unique_ptr<Node> value;

    std::string debug_str() const override { return "<inst> return"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Break : public Node {
    std::string debug_str() const override { return "break"; }
 
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Continue : public Node {
    std::string debug_str() const override { return "continue"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};


// constant/comparison then {}
struct Match_Case : public Node {
    Evaluator evaluator;
    std::unique_ptr<Declaration::Local::CodeBlock> codeblock;

    std::string debug_str() const override { return "CASE"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// match <base> { <const/comparison> => {...} _ => {...} }
struct Match : public Node {
    std::shared_ptr<AReference> base;
    std::vector<std::unique_ptr<Match_Case>> cases;
    [[maybe_unused]]
    std::unique_ptr<Match_Case> other_case;

    std::string debug_str() const override { return "MATCH"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};


}
}