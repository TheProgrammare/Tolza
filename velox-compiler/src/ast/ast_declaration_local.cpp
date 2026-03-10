#include "ast_declaration_local.hpp"

#include "ast/ast_type.hpp"

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