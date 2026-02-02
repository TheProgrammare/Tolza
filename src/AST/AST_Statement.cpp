#include "AST_Statement.hpp"

#include "AST_Declaration_Local.hpp"

std::string AST::Statement::For::debug_str() const
{ 
    std::string str_index = index ? "index: " + index->debug_str() : "";
    std::string str_items;
    for (auto &item : items) {
        str_items += ", " + item->debug_str();
    }

    return "FOR[" + str_index + str_items + "]";
}