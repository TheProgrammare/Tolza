#include "ast_expression.hpp"

#include "ast_base.hpp"
#include "ast_declaration.hpp"
#include "ast_declaration_cop.hpp"

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