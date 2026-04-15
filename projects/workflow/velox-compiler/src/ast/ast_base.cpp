#include "ast_base.hpp"

#include <cstddef>

#include "misc/error_output.hpp"

#include "ast_type.hpp"

#include "misc/module_manager.hpp"

#include "visitor/visitor_base.hpp"
#include "codegen/visitor_codegen.hpp"


std::string ast::mangle_path(const std::vector<std::string>& p_in_path)
{
  std::string out;
  for (auto elem : p_in_path) out += elem + ".";
  return out.substr(0, out.size() - 1);
}

EPassMode ast::get_defaultParamPassmode(ast::AType& node)
{
  if (dynamic_cast<ast::type::Primitive*>(&node)) return EPassMode::Copy;
  // pass by ref
  return EPassMode::Ref;
}

ast::ADeclaration::ADeclaration(SYM_REF p_sym, const std::string p_name, bool p_external)
  : declaration_name(p_name)
  , declaration_is_external(p_external)
  , declaration_symbol(p_sym)
{
  visibility = node_module->visibility;
}


void ast::Expr_ID::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::Expr_ID_Qualified::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::Expr_ID_Type::accept(Visitor_Base& v)
{
  v.visit(*this);
}

llvm::Value* ast::Expr_ID::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::Expr_ID_Qualified::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::Expr_ID_Type::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::Expr_ID_Type::codegen_ty(Visitor_Codegen& v)
{
  return v.visit_ty(*this);
}

void ast::Root::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::Root::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
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

std::string ast::Node::mangle_scope() const
{
  return node_module->get_mangling_name();
}

std::string ast::Expr_ID_Type::mangle_type() const
{
  std::string out = name->mangle_id_node();

  size_t count = 0;
  for (auto& ty : gen_args) {
    out += "." + ty->mangle_type();
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


bool ast::Expr_ID_Type::compare_with(const AType& other) const
{
  if (auto ptr = dynamic_cast<const Expr_ID_Type*>(&other)) {
    return name == ptr->name && gen_args == ptr->gen_args;
  }
  return false;
}