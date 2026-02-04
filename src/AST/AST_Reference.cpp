#include "AST_Reference.hpp"

#include "AST_Base.hpp"
#include "AST_Declaration.hpp"
#include "AST_Declaration_ECS.hpp"

std::string AST::Reference::Member_Access::debug_str() const
{ 
    return left->debug_str() + "." + right->debug_str();
}

std::shared_ptr<AST::ADeclaration> AST::Reference::Enum::get_symbol_resolution()
{ 
    return std::dynamic_pointer_cast<ADeclaration>(resolved_sym); 
}