#include "ast_operation.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"
#include "ast_inferred_type_singleton.hpp"


void ast::operation::Cast_As::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Cast_As::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Is::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Is::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::In::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::In::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Assignment::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Assignment::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Binary::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Binary::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Unary::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Unary::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Interval::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Interval::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::operation::Ptr_Dist::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::operation::Ptr_Dist::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


ast::operation::Is::Is()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::In::In()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::Interval::Interval()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::Ptr_Dist::Ptr_Dist()
{
  inferred_type = ast::type::get_ptrdiff_type();
}