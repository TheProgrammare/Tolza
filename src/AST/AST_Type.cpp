#include "AST_Type.hpp"

#include "AST_Declaration.hpp"
#include "AST_Declaration_Local.hpp"

std::string AST::Type::Function_Proto::mangle_type() const
{
    std::string retStr = "fn" + std::to_string(parameters.size());
    for (auto& param : parameters) {
        retStr += "_" + param->ty->mangle_type();
    }
    retStr += returnType ? "_" + returnType->mangle_type() : "";

    return retStr;
}

bool AST::Type::Function_Proto::operator==(const AType &other) const
{
    if (auto ptr = dynamic_cast<const Function_Proto*>(&other)) {
        if (isVariadic != ptr->isVariadic) return false;
        if (isVariadic && (!variadic_ty->operator==(*ptr->variadic_ty))) return false;
        if (parameters.size() != ptr->parameters.size()) return false;
        if (*returnType != *ptr->returnType) return false;
        for (size_t i = 0; i < parameters.size(); i++) {
            const Declaration::Local::Parameter& _local_param = *parameters[i];
            const Declaration::Local::Parameter& _other_param = *ptr->parameters[i];
            if (!_local_param.operator==(_other_param)) return false;
        }

        return true;
    }
    return false;
}