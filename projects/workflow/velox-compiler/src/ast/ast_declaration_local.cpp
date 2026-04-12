#include "ast_declaration_local.hpp"

#include "ast/ast_type.hpp"
#include "visitor/visitor_base.hpp"

#include "codegen/visitor_codegen.hpp"
#include <llvm/IR/Value.h>

llvm::Value* ast::declaration::local::CodeBlock::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}

void ast::declaration::local::CodeBlock::accept(Visitor_Base& v)
{
  v.visit(*this);
}

void ast::declaration::local::Variable_Binding::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable_Binding::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}

llvm::Value* ast::declaration::local::Variable_Binding::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


void ast::declaration::local::Pattern_Enum::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Pattern_Tuple::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Pattern_Entity::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Pattern_System_Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Pattern_Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Tuple_Destructuring::accept(Visitor_Base& v)
{
  v.visit(*this);
}

llvm::Value* ast::declaration::local::Pattern_Enum::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Tuple::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Entity::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_System_Component::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Component::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Tuple_Destructuring::codegen(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}


void ast::declaration::local::Lambda::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Lambda::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Function* ast::declaration::local::Lambda::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Variable::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Capability::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Capability::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
llvm::Value* ast::declaration::local::Capability::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Capture_Member::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::declaration::local::Lambda_Capture::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Parameter::codegen_pass(Visitor_Codegen& v)
{
  return v.visit(*this);
}
void ast::declaration::local::Parameter::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Generic_Parameter_Element::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
void ast::declaration::local::Generic_Parameter_Element::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Generic_Parameters::codegen_pass(Visitor_Codegen& v)
{
  v.visit(*this);
  return nullptr;
}
void ast::declaration::local::Generic_Parameters::accept(Visitor_Base& v)
{
  v.visit(*this);
}


std::string ast::declaration::local::Tuple_Destructuring::debug_str() const
{
  std::string out;
  out += "unpack variable ";
  switch (kind) {
  case EVariableKind::Const: out += "const"; break;
  case EVariableKind::Let:   out += "let"; break;
  case EVariableKind::Var:   out += "var"; break;
  case EVariableKind::NONE:  return "";
  }
  out += " [";
  for (auto& elem : elements) {
    out += elem->declaration_name + ", ";
  }
  return out + "]";
}

std::string ast::declaration::local::Lambda::debug_str() const
{
  return "lam " + prototype->debug_str();
}

std::string ast::declaration::local::Parameter::debug_str() const
{
  std::string out = EPassMode_to_str(passmode) + " " + declaration_name + ": " + type->debug_str();
  if (is_variadic) out += "...";
  if (defaultValue) out += " = " + defaultValue->debug_str();
  return out;
}

std::string ast::declaration::local::Generic_Parameter_Element::debug_str() const
{
  std::string out = generic_references.empty() ? name : name + ": ";
  for (size_t i = 0; i < generic_references.size(); i++) {
    out += generic_references[i]->debug_str();
    if (i != generic_references.size() - 1) out += " + ";
  }
  return out;
}

std::string ast::declaration::local::Generic_Parameters::debug_str() const
{
  std::string out = "&lt;";
  for (size_t i = 0; i < parameters.size(); i++) {
    out += parameters[i]->debug_str();
    if (i != parameters.size() - 1) out += ", ";
  }
  return out + "&gt;";
}


std::string ast::declaration::local::Variable::debug_str() const
{
  std::string out;
  out += "local ";
  switch (kind) {
  case EVariableKind::Const: out += "const "; break;
  case EVariableKind::Let:   out += "let "; break;
  case EVariableKind::Var:   out += "var "; break;
  case EVariableKind::NONE:  return "NO VAR KIND";
  }
  out += declaration_name;

  if (type) out += ": " + type->debug_str();
  return out;
}