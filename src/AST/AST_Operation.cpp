#include "AST_Operation.hpp"

#include "AST_Inferred_Type_Singleton.hpp"

AST::Operation::Is::Is() {
    inferred_type = AST::Type::get_bool_type();
}

AST::Operation::In::In() {
    inferred_type = AST::Type::get_bool_type();
}

AST::Operation::Interval::Interval() {
    inferred_type = AST::Type::get_bool_type();
}