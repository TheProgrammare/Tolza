#include "ast_declaration_local.hpp"

#include "ast/ast_type.hpp"
#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"

void ast::declaration::local::CodeBlock::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::CodeBlock::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Variable_Binding::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable_Binding::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Pattern_Enum::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Enum::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Pattern_Tuple::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Tuple::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Pattern_Entity::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Entity::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Pattern_Component::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Pattern_Component::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Variable_Unpack::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable_Unpack::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Lambda::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Lambda::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Variable::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Variable::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Capability::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Capability::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Capture_Member::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Capture_Member::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Lambda_Capture::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Lambda_Capture::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Parameter::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Parameter::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Generic_Parameter_Element::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Generic_Parameter_Element::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}

void ast::declaration::local::Generic_Parameters::accept(Visitor_Base& v)
{
  v.visit(*this);
}
llvm::Value* ast::declaration::local::Generic_Parameters::codegen(Visitor_Codegen& v)
{
  return v.visit(*this);
}


std::string ast::declaration::local::Variable_Unpack::debug_str() const
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
    out += elem->name + ", ";
  }
  return out + "]";
}

std::string ast::declaration::local::Lambda::debug_str() const
{
  return "lam " + prototype->debug_str();
}

std::string ast::declaration::local::Parameter::debug_str() const
{
  std::string out = EPassMode_to_str(passMode) + " " + name + ": " + type->debug_str();
  if (isVariadic) out += "...";
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
  out += name;

  if (type) out += ": " + type->debug_str();
  return out;
}