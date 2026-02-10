#include "AST_Literal.hpp"

#include "Visitor/Symbol_Manager.hpp"

std::string AST::Literal::Table::debug_str() const { 
    if (element_definition.expired()) return "";
    std::string out;
    out = "table[" + std::to_string(resolved_size.size()) + ":";
    for (size_t i = 0; i < resolved_size.size(); i++) {
        out += std::to_string(resolved_size[i]) + "x";
    }
    if (auto locked = element_definition.lock())
        out += " -> " + locked->mangling + "]";
    return out;
}