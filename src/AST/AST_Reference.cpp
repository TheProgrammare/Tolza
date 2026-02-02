#include "AST_Reference.hpp"

#include "AST_Base.hpp"
#include "AST_Declaration.hpp"
#include "AST_Declaration_ECS.hpp"

std::string AST::Reference::Member_Access::debug_str() const
{ 
    std::string out = id.debug_str();
    for (auto &elem : elements) {
        out += "." + elem->id.debug_str();
    }
    return out;
}

std::shared_ptr<AST::ADeclaration> AST::Reference::Enum::get_symbol_resolution()
{ 
    return std::dynamic_pointer_cast<ADeclaration>(resolved_sym); 
}