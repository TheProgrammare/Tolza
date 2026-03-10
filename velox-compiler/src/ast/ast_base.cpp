#include "ast_base.hpp"

#include <cstddef>
#include <llvm-19/llvm/IR/Value.h>

#include "error_output.hpp"

#include "ast_type.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"

std::string mangle_id(const std::string& inId)
{
  if (inId.empty()) return ""; // no name, must return empty string and no 0
  return std::to_string(inId.size()) + inId;
}

EPassMode ast::get_defaultParamPassmode(ast::AType& node)
{
  if (dynamic_cast<ast::type::Primitive*>(&node)) return EPassMode::Copy;
  // pass by ref
  return EPassMode::Ref;
}


void ast::Expr_ID::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::Expr_ID::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::Expr_ID_Qualified::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::Expr_ID_Qualified::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::Expr_ID_Type::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::Expr_ID_Type::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::Root::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::Root::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

std::string ast::Expr_ID_Qualified::debug_str() const
{
  std::string outStr;
  for (auto& seg : path) {
    outStr += seg + "::";
  }

  outStr += name;
  return outStr;
}

std::string ast::Expr_ID_Qualified::mangle_path() const
{
  std::string outStr;
  for (auto& seg : path) {
    outStr += mangle_id(seg);
  }
  return outStr;
}

std::string ast::Expr_ID_Qualified::mangle_local_name() const
{
  if (qualification_at_root_scope) {
    return mangle_path() + mangle_id(get_base_name());
  } else if (qualification_at_current_scope) {
    return mangle_scope() + mangle_path() + mangle_id(get_base_name());
  } else if (qualification_at_parent_scope) {
    // remove parent in loop
    std::string out;
    for (int i = 0; i < _scope.size() - 1; i++) {
      out += mangle_id(_scope[i]);
    }
    return out + mangle_path() + mangle_id(get_base_name());
  }
  // local level by default
  else if (is_qualified_id()) {
    return mangle_path() + mangle_id(get_base_name());
  }
  // local level by default
  else {
    return mangle_scope() + mangle_id(get_base_name());
  }
}

std::string ast::Expr_ID_Qualified::mangle_qualified_name() const
{
  if (is_qualified_id()) {
    return mangle_path() + mangle_id(get_base_name());
  }
  // local level by default
  else {
    return mangle_id(get_base_name());
  }
}

std::string ast::Node::mangle_scope() const
{
  std::string out;
  for (auto& seg : _scope) {
    out += mangle_id(seg);
  }
  return out;
}

bool ast::Node::is_visible_in(const std::span<const std::string>& other_scope) const
{
  if (_scope.empty() && other_scope.empty()) return true;
  if (other_scope.empty()) return true;

  if (_scope.size() > other_scope.size()) return false;

  for (size_t i = 1; i < _scope.size(); i++) {
    if (_scope[i] != other_scope[i]) return false;
  }

  return true;
}


std::string ast::Expr_ID_Type::mangle_types() const
{
  std::string out;
  for (auto& elem : gen_args) out += elem->mangle_scope();
  return out;
}

std::string ast::Expr_ID_Type::mangle_type() const
{
  std::string out;

  size_t count = 0;
  for (auto& ty : gen_args) {
    out += ty->mangle_type();
    if (count++ != gen_args.size() - 1) out += "_";
  }

  return out;
}

std::string ast::Expr_ID_Type::debug_str() const
{
  std::string out = name->debug_str();
  if (!gen_args.empty()) out += "&lt;";
  for (size_t i = 0; i < gen_args.size(); i++) {
    out += gen_args[i]->debug_str();
    if (i != gen_args.size() - 1) out += ", ";
  }
  if (!gen_args.empty()) out += "&gt;";
  return out;
}
