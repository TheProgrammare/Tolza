# pragma once

#include "AST_Base.hpp"

struct Visitor_Base;

namespace AST {
namespace Type {

struct Ptr final : public AType {
    EPtrType pointer_type = EPtrType::raw_ptr;

    std::unique_ptr<AType> inner;

    bool operator==(const AType& other) const override {
        if (auto ptr = dynamic_cast<const Ptr*>(&other)) {
            return AType::operator==(other) && pointer_type == ptr->pointer_type && *inner == *ptr->inner;
        }
        return false;
    }

    std::string mangle_type() const override { return EPtrType_to_mangle(pointer_type); }
    std::string debug_str() const override { return EPtrType_to_str(pointer_type); }
    EPrimType get_type() const override { return inner->get_type(); }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Table final : public AType {
    std::optional<size_t> tableSize; // nullopt = dynamic
    std::unique_ptr<Node> sizeSymbol;

    std::unique_ptr<AType> inner;

    bool operator==(const AType& other) const override {
        if (auto ptr = dynamic_cast<const Table*>(&other)) {
            return AType::operator==(other) && tableSize == ptr->tableSize && *inner == *ptr->inner;
        }
        return false;
    }

    std::string mangle_type() const override {
        if (tableSize.has_value()) return "arr" + std::to_string(tableSize.value()) + "_" + inner->mangle_type();
        else return "list_" + inner->mangle_type();
    }
    std::string debug_str() const override { return "<ty> table"; }
    EPrimType get_type() const override { return inner->get_type(); }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Primitive final : public AType {
    EPrimType _type = EPrimType::u8;

    bool operator==(const AType& other) const override {
        if (auto ptr = dynamic_cast<const Primitive*>(&other)) {
            return AType::operator==(other) && _type == ptr->_type;
        }
        return false;
    }

    std::string mangle_type() const override { return EPrimTy_to_mangle(_type); }
    std::string debug_str() const override { return EPrimTy_to_str(_type); }
    EPrimType get_type() const override { return _type; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Tuple final : public AType {
    std::vector<std::unique_ptr<AType>> types;
    std::vector<std::string> name_fields;

    bool operator==(const AType& other) const override {
        if (auto ptr = dynamic_cast<const Tuple*>(&other)) {
            if (types.size() != ptr->types.size()) return false;
            for (size_t i = 0; i < types.size(); i++) {
                const AType& _local_ty = *types[i];
                const AType& _other_ty = *ptr->types[i];
                if (!_local_ty.operator==(_other_ty)) return false;
            }
            return true;
        }
        return false;
    }

    bool operator!=(const AType& other) const {
        return !(*this == other);
    }

    std::string mangle_type() const override {
        std::string retStr = "tu" + std::to_string(types.size());
        for (auto& ty : types) {
            retStr += "_" + ty->mangle_type();
        }
        return retStr;
    }
    std::string debug_str() const override { 
        if (name_fields.empty()) return "<ty> tuple(" + std::to_string(types.size()) + ")";
        return "<ty> named tuple(" + std::to_string(types.size()) + ")";
    }
    EPrimType get_type() const override { return EPrimType::tuple; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Function_Proto final : public AType {
    std::vector<std::shared_ptr<Declaration::Local::Parameter>> parameters;
    std::vector<std::unique_ptr<Declaration::Local::Generic_Parameter>> gen_parameters;
    std::unique_ptr<Tuple> returnType;
    
    bool isVariadic = false;
    std::shared_ptr<AType> variadic_ty;
    size_t Variadic_start_pos = 0;

    bool operator==(const AType& other) const override;

    std::string mangle_type() const override;
    std::string debug_str() const override { return "<ty> fn"; }
    EPrimType get_type() const override { return EPrimType::Fn_Proto; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Get_Expr_Type final : public AType {
    std::unique_ptr<Node> target;

    SYM_TYPE<AType> resolved_ty;

    bool operator==(const AType& other) const override {
        if (!resolved_ty.resolved) return false;
        return AType::operator==(other) && *resolved_ty.ptr == other;
    }
    
    std::string mangle_type() const override { return resolved_ty.resolved ? resolved_ty.ptr->mangle_type() : ""; };
    std::string debug_str() const override { return "<ty> comptime"; };
    EPrimType get_type() const override { return resolved_ty.resolved ? resolved_ty.ptr->get_type() : EPrimType::NONE; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

}
}