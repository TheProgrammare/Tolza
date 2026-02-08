#pragma once

#include "AST_Base.hpp"

namespace AST {

namespace Generic {

struct IGenCond : public Node {
    virtual ~IGenCond() = default;
    // for semantic viewer
    [[nodiscard]] virtual bool type_isValid(const AType& ty) const = 0;
};



struct Is_Type : IGenCond {
    std::string srcTypename;
    std::vector<std::unique_ptr<AReference>> inType;

    void accept(Visitor_Base& v) override { v.visit(*this); }

    bool type_isValid(const AType& ty) const override {
        for (auto &_ty : inType) {
            if (auto ptr = dynamic_cast<AST::AType*>(_ty.get())) {
                if (*ptr == ty) return true;
            }
        }
        return false;
    }
    std::string debug_str() const override { return "gen is"; }
};

struct Can_Cast : IGenCond {
    std::string srcTypename;						// typename
    std::unique_ptr<AType> target;						// cast target
    bool isCastFrom = false;				// false = cast to | true = cast from

    void accept(Visitor_Base& v) override { v.visit(*this); }

    bool type_isValid(const AType& ty) const override {
        return *target == ty;
    }
    std::string debug_str() const override { return std::string("gen cast ") + (isCastFrom ? "from" : "to"); }
};

struct Have_Op : IGenCond {
    std::string targetGenSym;						// typename
    EBinOpType operatorType = EBinOpType::Add;	    // operator
    std::shared_ptr<AType> explicit_return_ty;		// for indexation/iterator

    void accept(Visitor_Base& v) override { v.visit(*this); }

    bool type_isValid(const AType& ty) const override;
    std::string debug_str() const override { return "gen op"; }
};

struct Have_Role : IGenCond {
    std::string targetGenSym;
    std::unique_ptr<AReference> role;

    void accept(Visitor_Base& v) override { v.visit(*this); }

    std::shared_ptr<Declaration::COP::Role> resolved_role_sym;

    bool type_isValid(const AType& ty) const override;
    std::string debug_str() const override { return "gen role"; }
};

struct Use_Component : IGenCond {
    std::string targetGenSym;
    std::unique_ptr<AReference> component;

    void accept(Visitor_Base& v) override { v.visit(*this); }

    std::shared_ptr<Declaration::COP::Component> resolved_comp_sym;

    bool type_isValid(const AType& ty) const override;
    std::string debug_str() const override { return "gen component"; }
};

struct Compatible_System : IGenCond {
    std::string targetGenSym;
    std::unique_ptr<AReference> system;

    void accept(Visitor_Base& v) override { v.visit(*this); }

    std::shared_ptr<Declaration::COP::System> resolved_system_sym;

    bool type_isValid(const AType& ty) const override;
    std::string debug_str() const override { return "gen system"; }
};

}
}