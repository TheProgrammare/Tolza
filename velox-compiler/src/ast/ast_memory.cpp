#include "ast_memory.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"


void ast::memory::Del::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::memory::Del::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::memory::Align::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::memory::Align::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::memory::Drop::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::memory::Drop::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}