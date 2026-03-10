#include "ast_type.hpp"
#include <string>

std::string ast::type::Function_Proto::mangle_type() const
{
  std::string out   = "fn" + std::to_string(parameters.size());
  size_t      count = 0;
  for (auto& param : parameters) {
    out += param->type->mangle_type();
    if (count++ != parameters.size() - 1) out += "_";
  }

  if (returnType)
    out += "_" + returnType->mangle_type();
  else
    out += "_u0";

  return out;
}

std::string ast::type::Function_Proto::debug_str() const
{
  std::string out = "fn";

  if (!gen_parameters.empty()) out += "&lt;";
  for (size_t i = 0; i < gen_parameters.size(); i++) {
    out += gen_parameters[i]->debug_str();

    if (i == gen_parameters.size() - 1)
      out += "&gt;";
    else
      out += ", ";
  }

  out += "(";
  for (size_t i = 0; i < parameters.size(); i++) {
    out += parameters[i]->debug_str();

    if (i != parameters.size() - 1) out += ", ";
  }
  out += ")";

  std::string ret;
  if (returnType) ret = " -&gt; " + returnType->debug_str();

  return out + ret;
}