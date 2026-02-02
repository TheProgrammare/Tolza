#pragma once

#include <string>
#include <memory>
#include <vector>

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
};

struct Enum : public ADeclaration, AType {
    std::vector<std::unique_ptr<Enum_Element>> variants;

    bool isGlobal = true;
    size_t discriminant_max = 0;
    [[maybe_unused]] EPrimType discriminant_int_ty = EPrimType::u8;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<ty> enum[" + id.debug_str() + "]"; }
    [[nodiscard]] std::string mangle_type() const override { return "enum"; }
};

struct Flag : public ADeclaration, AType {
    std::vector<std::string> fields;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<ty> flag[" + id.debug_str() + "]"; }
    [[nodiscard]] std::string mangle_type() const override { return "typealias"; }
};

// e.g. mod name {}
struct Mod : public ADeclaration {
    std::vector<std::shared_ptr<Node>> elements;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<def> mod[" + id.debug_str() + "]"; }
};

struct Export : public Mod {
    std::shared_ptr<ModuleExportation> mod_exp_sym;

    [[nodiscard]] std::string debug_str() const override { return "<def> export[" + id.debug_str() + "]"; }
};




struct Function : public ADeclaration, ICallable {
    std::shared_ptr<Type::Function_Proto> prototype;
    std::unique_ptr<Local::CodeBlock> codeblock;
    bool isDefinition = false;
    bool isConst = false; bool isPure = false; bool isCompileTime = false; 
    bool isExtern = false;
    std::string extern_call_convention;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "fn[" + id.debug_str() + "]"; }
    Type::Function_Proto* get_signature() override { return prototype.get(); };
};



struct Type_Alias : public ADeclaration {
    std::unique_ptr<AType> ty;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "alias[" + id.debug_str() + "]"; }
};

// gen name<T, U,...> { condition }
struct Generic : public ADeclaration {
    std::unique_ptr<Type_Arguments> gen_args;
    std::set<std::string> targetGenericSymbols;		// generic typenames
    std::vector<std::unique_ptr<AST::Generic::IGenCond>> conditions;			// generic conditions

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<ty> gen[" + id.debug_str() + "]"; }
};



// let/var a: ptr'type#tableSize = expression;
struct Global : public ADeclaration {
    std::unique_ptr<AType> ty;											// infered if nullptr
    EAssignmentType assignment = EAssignmentType::MoveSemantic;		// assign type
    std::unique_ptr<Node> expression;									// affectation
    EVariableKind kind = EVariableKind::Const;

    bool isExtern = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { 
        std::string out;
        out += "<global> ";
        switch (kind) {
            case EVariableKind::Const: out += "const "; break; 
            case EVariableKind::Let: out += "let "; break;
            case EVariableKind::Var: out += "var "; break;
        }
        out += id.debug_str();
        return out;
    }

private:
    bool type_already_checked = false;
};


}
}