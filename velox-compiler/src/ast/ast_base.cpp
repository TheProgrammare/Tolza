#include "ast_base.hpp"

#include "error_output.hpp"

#include "ast_type.hpp"

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

std::string ast::Expr_ID_Generic::mangle_types() const
{
  std::string out;
  for (auto& elem : gen_args) out += elem->mangle_scope();
  return out;
}

std::string ast::Expr_ID_Generic::mangle_type() const
{
  std::string out = mangle_qualified_name();

  size_t count = 0;
  for (auto& ty : gen_args) {
    out += ty->mangle_type();
    if (count++ != gen_args.size() - 1) out += "_";
  }

  return out;
}
