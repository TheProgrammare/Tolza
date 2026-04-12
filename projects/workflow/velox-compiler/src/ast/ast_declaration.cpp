#include "ast_declaration.hpp"

#include "ast_declaration_local.hpp"
#include "ast_type.hpp"

#include "visitor/visitor_base.hpp"
#include "codegen/visitor_codegen.hpp"


void ast::declaration::Enum_Element::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Enum::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Flag::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Union::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Mod::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Export::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Extern::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Function::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Mod_Alias::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Type_Alias::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Generic::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::Global::accept(Visitor_Base& v)
{
  v.visit(*this);
}


llvm::Type* ast::declaration::Enum_Element::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::declaration::Enum::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::declaration::Flag::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::declaration::Union::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Type* ast::declaration::Generic::codegen_ty(Visitor_Codegen& v)
{
  return v.visit(*this);
}


llvm::Value* ast::declaration::Enum::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Flag::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Union::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Mod::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Export::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Extern::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Function::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::Mod_Alias::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Type_Alias::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Generic::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
llvm::Value* ast::declaration::Global::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}


std::string ast::declaration::Enum_Element::debug_str() const
{
  std::string out = "elem ::" + name;
  for (size_t i = 0; i < types.size(); i++) {
    auto& ty = types[i];

    if (i == 0) out += "(";

    out += ty->debug_str();

    if (i != types.size() - 1)
      out += ", ";
    else
      out += ")";
  }
  return out;
}

std::string ast::declaration::Enum_Element::mangle_type() const
{
  return parent_enum->mangle_type() + mangle_id(name);
}

llvm::Function* ast::declaration::Function::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


std::string ast::declaration::Function::debug_str() const
{
  std::string out = "fn " + declaration_name;
  if (!prototype) return out + "()";
  if (!prototype->gen_parameters.empty()) out += "&lt;";
  for (size_t i = 0; i < prototype->gen_parameters.size(); i++) {
    out += prototype->gen_parameters[i]->debug_str();

    if (i == prototype->gen_parameters.size() - 1)
      out += "&gt;";
    else
      out += ", ";
  }

  out += "(";
  for (size_t i = 0; i < prototype->parameters.size(); i++) {
    out += prototype->parameters[i]->debug_str();

    if (i != prototype->parameters.size() - 1) out += ", ";
  }
  out += ")";

  std::string ret;
  if (prototype->return_ty) ret = " -&gt; " + prototype->return_ty->debug_str();

  return out + ret;
}

std::string ast::declaration::Global::debug_str() const
{
  std::string out;
  out += "global ";
  switch (kind) {
  case EVariableKind::Const: out += "const "; break;
  case EVariableKind::Let:   out += "let "; break;
  case EVariableKind::Var:   out += "var "; break;
  case EVariableKind::NONE:  return "NO VAR KIND";
  }
  out += declaration_name;

  return out + ": " + type->debug_str();
}
llvm::Value* ast::declaration::Global::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


ast::declaration::Function::~Function() = default;
