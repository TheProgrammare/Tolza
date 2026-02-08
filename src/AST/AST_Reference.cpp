#include "AST_Reference.hpp"

#include "AST_Base.hpp"
#include "AST_Declaration.hpp"
#include "AST_Declaration_COP.hpp"

std::string AST::Reference::Member_Access::debug_str() const
{ 
    return left->debug_str() + "." + right->debug_str();
}
