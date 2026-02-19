#include "AST_Declaration_Local.hpp"

std::string AST::Declaration::Local::Variable_Unpack::debug_str() const
{
  std::string out;
  out += "<def> ";
  switch (kind) {
  case EVariableKind::Const: out += "const"; break;
  case EVariableKind::Let:   out += "let"; break;
  case EVariableKind::Var:   out += "var"; break;
  case EVariableKind::NONE:  return "";
  }
  out += "[";
  for (auto& elem : elements) {
    out += elem->name + ", ";
  }
  return out + "]";
}