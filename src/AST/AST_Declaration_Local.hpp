#pragma once

#include <memory>
#include <vector>
#include <string>

#include "AST_Base.hpp"
#include "AST_CodeBlock_Instruction.hpp"


namespace AST {
namespace Declaration {
namespace Local {

struct CodeBlock : public Node {
    std::vector<CodeBlock_instruction> elements;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "codeblock"; }
};


// inside of Enum/Tuple pattern
// e.g. Some(a) = value
// e.g. Player { CId.name: name, CId.age: age } = value
// e.g. Player { CId { name: name, age: age } } = value
// e.g. (a, b, c) = triple
// e.g. match val { Some(a) => ... }
// e.g. sys name() { Component(c) => ... }
struct Variable_Binding : public ALocal {
    std::string name;
    ECapability capability = ECapability::NONE;

    std::shared_ptr<AReference> reference;

    std::string debug_str() const override { return "bind[" + name + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Bind; };
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Pattern_Element {
    enum class Kind { Ignore, Binding, Literal };

    Kind kind;
    std::shared_ptr<Variable_Binding> bind;
    std::unique_ptr<ALiteral> literal;

    Pattern_Element(std::shared_ptr<Variable_Binding> _bind)
        : kind(Kind::Binding)
        , bind(_bind) {}
    Pattern_Element(std::unique_ptr<ALiteral> _literal)
        : kind(Kind::Literal)
        , literal(std::move(_literal)) {}
    Pattern_Element() 
        : kind(Kind::Ignore) {}

    Node *node() {
        switch (kind) {
        case Kind::Binding: return static_cast<Node*>(bind.get());
        case Kind::Literal: return static_cast<Node*>(literal.get());
        case Kind::Ignore:  return nullptr;
        }
    }
};

struct Pattern : public Node {
    ECapability capability = ECapability::Ref;
    // shared because can be from a match case base reference (so a reference mirror)
    std::shared_ptr<AReference> reference;

    [[maybe_unused]]
    std::shared_ptr<Node> additive_evaluator;
};

// e.g. [if/elif/while] let Some(a) = value {...}
struct Pattern_Enum : public Pattern {
    ID enum_id;
    std::vector<Pattern_Element> mapping;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "enum pattern[" + enum_id.debug_str() + "]"; }
};

// e.g. [if/while/for] let (a, b, 10) in triple_collection {...}
struct Pattern_Tuple : public Pattern {
    std::vector<Pattern_Element> mapping;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "tuple pattern"; }
};

// e.g. [if/while] let Player{ CId.name: name, CId.id: 10 } 
// e.g. [if/while] let Player{ CId{ name: name, id: 10 } } 
struct Pattern_Entity : public Pattern {
    ID entity_id;

    // component identifier, field_name, pattern_element
    std::vector<std::tuple<ID, std::string, Pattern_Element>> mapping;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "entity pattern"; }
};

// e.g. [if/while] let CId{ name: name, id: 10 } 
struct Pattern_Component : public Pattern {
    ID component_id;

    // field_name, pattern_element
    std::vector<std::tuple<std::string, Pattern_Element>> mapping;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "component pattern"; }
};

// var (a, b, _, d) = call();
// var (a, _, c, d) = tupleVariable;
struct Variable_Unpack : public ALocal {
    std::vector<std::shared_ptr<Variable_Binding>> elements;
    std::unique_ptr<AReference> reference;
    EVariableKind kind = EVariableKind::Const;

    bool isStatic = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override;
    ESymbolType get_symbol_type() const override { return ESymbolType::Local; };
};


struct Lambda : public ALocal, ICallable {
    std::shared_ptr<Type::Function_Proto> prototype;
    std::unique_ptr<Local::CodeBlock> codeblock;

    bool isConst = false; bool isPure = false; bool isMutable = false; 
    bool isNoexcept = false; bool isLambdaConstexpr = false; bool isConstexpr = false;
    std::unique_ptr<Lambda_Capture> capture;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "lam[" + id.debug_str() + "]"; };
    Type::Function_Proto* get_signature() override { return prototype.get(); };
    ESymbolType get_symbol_type() const override { return ESymbolType::Lambda; };
};



// let/var a: ptr'type?$ = expression;
struct Variable : public ALocal {
    [[maybe_unused]] std::shared_ptr<AType> type;				// infered if nullptr
    EAssignmentType assignment = EAssignmentType::Copy;		// assign type
    [[maybe_unused]] std::optional<std::unique_ptr<Node>> expression;		// affectation
    EVariableKind kind = EVariableKind::Const;

    bool isStatic = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        std::string out;
        out += "<def> ";
        switch (kind) {
            case EVariableKind::Const: out += "const"; break; 
            case EVariableKind::Let: out += "let"; break;
            case EVariableKind::Var: out += "var"; break;
            case EVariableKind::NONE: return "NO VAR KIND";
        }
        out += "[" + id.debug_str() + "]";
        return out;
    }
    ESymbolType get_symbol_type() const override { return ESymbolType::Local; };
};

struct Capability : public ALocal {
    std::string name;

    std::shared_ptr<AReference> reference;

    ECapability kind = ECapability::NONE;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        std::string str_kind = kind == ECapability::Mut ? "mut " : "ref ";
        return str_kind + name;
    }
    ESymbolType get_symbol_type() const override { return ESymbolType::Local; };
};



struct Capture_Member : public AReference {
    ECapability capability;

    std::string debug_str() const override { return "capture " + id.debug_str(); }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Lambda_Capture : public Node {
    std::vector<std::unique_ptr<Capture_Member>> elements;
    bool isAllRef = false; bool isCaptureSelf = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "<def> capture"; }
};

// (a: str, copy b: i32 = 10, mut c: f32 = nullptr, args: ...)
struct Parameter : public ALocal {
    EPassMode passMode = EPassMode::Copy;
    std::shared_ptr<AType> type;
    [[maybe_unused]] std::unique_ptr<Node> defaultValue;
    bool isVariadic = false;

    SYM_DEFINITION parent_function;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        if (isVariadic) return EPassMode_to_str(passMode) + " " + id.debug_str() + "...";
        else			return EPassMode_to_str(passMode) + " " + id.debug_str();
    }
    ESymbolType get_symbol_type() const override { return ESymbolType::Parameter; } 
};

struct Generic_Parameter : public ALocal {
    std::string name;
    std::vector<std::unique_ptr<AType>> generic_references;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { 
        std::string out = generic_references.empty() ? name : name + ": ";
        for (size_t i = 0; i < generic_references.size(); i++)
        {
            out += generic_references[i]->debug_str();
            if (i != generic_references.size() - 1) out += " + ";
        }
        return out;
    }
    ESymbolType get_symbol_type() const override { return ESymbolType::Generic_Parameter; };
};

}
}
}