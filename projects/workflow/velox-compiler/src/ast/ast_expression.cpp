#include "ast_expression.hpp"

#include "ast_base.hpp"

#include "visitor/visitor_base.hpp"
#include "codegen/visitor_codegen.hpp"

void ast::expression::If_Ternary::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Member_Access::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Self::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Other::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Call_Argument::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Call::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Call_System::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Call_Pipe::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Table_Access::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Ptr_At::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Ptr_Offset::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Ptr_Val::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Addr_Of::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Size_Of::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::GetBits::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::Move::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::expression::New_Ptr::accept(Visitor_Base& v)
{
  v.visit(*this);
}


llvm::Value* ast::expression::If_Ternary::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Member_Access::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Self::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Other::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Call_Argument::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Call::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Call_System::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Call_Pipe::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Table_Access::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Ptr_At::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Ptr_Offset::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Ptr_Val::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Addr_Of::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Size_Of::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::GetBits::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::Move::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::expression::New_Ptr::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


std::string ast::expression::Call::debug_str() const
{
  std::string out = "call[" + callee->debug_str();

  if (!gen_args.empty()) out += "&lt;";
  for (size_t i = 0; i < gen_args.size(); i++) {
    out += gen_args[i]->debug_str();

    if (i == gen_args.size() - 1)
      out += "&gt;";
    else
      out += ", ";
  }

  out += "(";
  for (size_t i = 0; i < param_args.size(); i++) {
    out += param_args[i]->debug_str();

    if (i != param_args.size() - 1) out += ", ";
  }
  out += ")]";

  return out;
}