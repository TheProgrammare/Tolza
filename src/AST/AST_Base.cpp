#include "AST_Base.hpp"

#include "../ScriptInfo.hpp"
#include "../ErrorOutput.hpp"

#include "AST_Type.hpp"

std::string AST::make_error(const Node &n, const ScriptInfo &script, const std::string &code, const std::string &err, const std::string &hint)
{
    Error_Text out_error(
        n._token.span.line,
        n._token.span.col, 
        n._token.span.size, 
        script.src_lines[n._token.span.line - 1],
        code, 
        err,
        hint,
        script.file_path
    );

    return out_error.print_error();
}

EPassMode AST::get_defaultParamPassmode(AST::AType &node)
{
	if (dynamic_cast<AST::Type::Primitive*>(&node)) return EPassMode::Copy;
	// pass by ref 
	return EPassMode::Ref;
}

std::string AST::ID::debug_str() const
{
    std::string outStr;
    for (auto& seg : path) {
        outStr += seg + "::";
    }
    
    outStr += name;
    return outStr;
}

std::string AST::ID::mangle_path() const
{
    std::string outStr;
    for (auto& seg : path) {
        outStr += mangle_id(seg);
    }
    return outStr;
}

std::string AST::ID::mangle_local_name() const
{
    if (!parent) return "";
    if (qualification_at_root_scope) {
        return mangle_path() + mangle_name();
    }
    else if (qualification_at_current_scope) {
        return parent->mangle_scope() + mangle_path() + mangle_name();
    }
    else if (qualification_at_parent_scope) {
        // remove parent in loop
        std::string out;
        for (int i = 0; i < parent->_scope.size() - 1; i++) {
            out += mangle_id(parent->_scope[i]);
        }
        return out + mangle_path() + mangle_name();
    }
    // local level by default
    else if (is_qualified_id()) {
        return parent->mangle_scope() + mangle_path() + mangle_name();
    }
    // local level by default
    else {
        return parent->mangle_scope() + mangle_name();
    }
}

std::string AST::ID::mangle_absolute_name() const
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

bool AST::AType::operator==(const AType &other) const
{
    return
        type_isOptional == other.type_isOptional &&
        type_isConst == other.type_isConst &&
        type_isVolatile == other.type_isVolatile;
}