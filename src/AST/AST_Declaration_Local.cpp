#include "AST_Declaration_Local.hpp"

std::string AST::Declaration::Local::Variable_Unpack::debug_str() const
{ 
    std::string out;
    out += "<def> ";
    switch (kind) {
        case EVariableKind::Const: out += "const"; break; 
        case EVariableKind::Let: out += "let"; break;
        case EVariableKind::Var: out += "var"; break;
    }
    out += "[";
    for (auto& elem : var_names) out += elem->id.debug_str() + ", ";
    return out + "]";
}