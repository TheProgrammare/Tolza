#pragma once

#include <string>
#include <memory>
#include <vector>
#include <set>

#include "AST_Base.hpp"
#include "AST_Forward.hpp"

struct ModuleExportation;

namespace AST {
namespace Declaration {



struct Enum_Element : public Node {
    // if empty : it's a simple enum key element
    std::string name;
    std::vector<std::unique_ptr<AType>> types;
    size_t position = 0;

    std::string debug_str() const override { return "::" + name; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Enum : public ADeclaration {
    std::vector<std::unique_ptr<Enum_Element>> variants;

    bool isGlobal = true;
    size_t discriminant_max = 0;
    [[maybe_unused]] EPrimType discriminant_int_ty = EPrimType::u8;

    std::string debug_str() const override { return "<ty> enum[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Enum; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Flag : public ADeclaration {
    std::vector<std::string> fields;

    std::string debug_str() const override { return "<ty> flag[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Flag; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// e.g. mod name {}
struct Mod : public ADeclaration {
    std::vector<std::shared_ptr<Node>> elements;

    std::string debug_str() const override { return "<def> mod[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Module; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Export : public Mod {
    std::shared_ptr<ModuleExportation> mod_exp_sym;

    std::string debug_str() const override { return "<def> export[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Export; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};




struct Function : public ADeclaration, ICallable {
    std::shared_ptr<Type::Function_Proto> prototype;
    std::unique_ptr<Local::CodeBlock> codeblock;
    bool isDefinition = false;
    bool isConst = false; bool isPure = false; bool isCompileTime = false; 
    bool isExtern = false;
    std::string extern_call_convention;

    std::string debug_str() const override { return "fn[" + id.debug_str() + "]"; }
    Type::Function_Proto* get_signature() override { return prototype.get(); };
    ESymbolType get_symbol_type() const override { return ESymbolType::Function; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};



struct Type_Alias : public ADeclaration {
    std::unique_ptr<AType> ty;

    std::string debug_str() const override { return "alias[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Typealias; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// gen name<T, U,...> { condition }
struct Generic : public ADeclaration {
    std::unique_ptr<Type_Arguments> gen_args;
    std::set<std::string> targetGenericSymbols;		// generic typenames
    std::vector<std::unique_ptr<AST::Generic::IGenCond>> conditions;			// generic conditions

    std::string debug_str() const override { return "<ty> gen[" + id.debug_str() + "]"; }
    ESymbolType get_symbol_type() const override { return ESymbolType::Generic; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};



// let/var a: ptr'type#tableSize = expression;
struct Global : public ADeclaration {
    std::unique_ptr<AType> ty;											// infered if nullptr
    EAssignmentType assignment = EAssignmentType::MoveSemantic;		// assign type
    std::unique_ptr<Node> expression;									// affectation
    EVariableKind kind = EVariableKind::Const;

    bool isExtern = false;

    std::string debug_str() const override { 
        std::string out;
        out += "<global> ";
        switch (kind) {
            case EVariableKind::Const: out += "const "; break; 
            case EVariableKind::Let: out += "let "; break;
            case EVariableKind::Var: out += "var "; break;
            case EVariableKind::NONE: return "NO VAR KIND";
        }
        out += id.debug_str();
        return out;
    }
    ESymbolType get_symbol_type() const override { return ESymbolType::Global; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }

private:
    bool type_already_checked = false;
};


}
}