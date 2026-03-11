#include "ast_type.hpp"

#include <string>

#include "ast_declaration_local.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"

void ast::type::Ptr::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::type::Table::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::type::Primitive::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::type::Tuple::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::type::Function_Proto::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::type::Get_Expr_Type::accept(Visitor_Base& v)
{
  v.visit(*this);
}
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

bool ast::type::Function_Proto::compare_with(const AType& other) const
{
  if (auto ptr = dynamic_cast<const Function_Proto*>(&other)) {
    if (isVariadic != ptr->isVariadic) return false;
    if (parameters.size() != ptr->parameters.size()) return false;
    if ((returnType == nullptr) != (ptr->returnType == nullptr)) return false;

    if (returnType) {
      if (!returnType->is_same(*ptr->returnType)) return false;
    }

    for (size_t i = 0; i < parameters.size(); i++) {
      if (!parameters[i]->is_same(*ptr->parameters[i])) return false;
    }
  }
  return false;
}

ast::type::Function_Proto::~Function_Proto() = default;