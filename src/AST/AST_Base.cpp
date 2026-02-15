#include "AST_Base.hpp"

#include "ErrorOutput.hpp"

#include "AST_Type.hpp"

EPassMode AST::get_defaultParamPassmode(AST::AType &node)
{
	if (dynamic_cast<AST::Type::Primitive*>(&node)) return EPassMode::Copy;
	// pass by ref 
	return EPassMode::Ref;
}

std::string AST::Expr_ID_Qualified::debug_str() const
{
    std::string outStr;
    for (auto& seg : path) {
        outStr += seg + "::";
    }
    
    outStr += name;
    return outStr;
}

std::string AST::Expr_ID_Qualified::mangle_path() const
{
    std::string outStr;
    for (auto& seg : path) {
        outStr += mangle_id(seg);
    }
    return outStr;
}

std::string AST::Expr_ID_Qualified::mangle_local_name() const
{
    if (qualification_at_root_scope) {
        return mangle_path() + mangle_name();
    }
    else if (qualification_at_current_scope) {
        return mangle_scope() + mangle_path() + mangle_name();
    }
    else if (qualification_at_parent_scope) {
        // remove parent in loop
        std::string out;
        for (int i = 0; i < _scope.size() - 1; i++) {
            out += mangle_id(_scope[i]);
        }
        return out + mangle_path() + mangle_name();
    }
    // local level by default
    else if (is_qualified_id()) {
        return mangle_path() + mangle_name();
    }
    // local level by default
    else {
        return mangle_scope() + mangle_name();
    }
}

std::string AST::Expr_ID_Qualified::mangle_absolute_name() const
{
    if (is_qualified_id()) {
        return mangle_path() + mangle_name();
    }
    // local level by default
    else {
        return mangle_name();
    }
}

std::string AST::Node::mangle_scope() const
{ 
    std::string out;
    for (auto &seg : _scope) {
        out += mangle_id(seg);
    }
    return out; 
}
