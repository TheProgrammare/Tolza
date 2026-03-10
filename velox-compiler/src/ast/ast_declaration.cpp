#include "ast/ast_declaration.hpp"

#include "ast/ast_type.hpp"

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

std::string ast::declaration::Function::debug_str() const
{
  std::string out = "fn " + name;
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
  if (prototype->returnType) ret = " -&gt; " + prototype->returnType->debug_str();

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
  out += name;

  return out + ": " + type->debug_str();
}