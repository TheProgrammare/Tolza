#pragma once

#include "ast/ast_data.hpp"
#include "ast_base.hpp"
#include <string>
#include "ast/ast_declaration_local.hpp"

struct Visitor_Base;

namespace ast
{
namespace type
{

struct Ptr final : public AType {
  EPtrType               pointer_type = EPtrType::raw_ptr;
  std::unique_ptr<AType> inner;

  std::string mangle_type() const override
  {
    return EPtrType_to_mangle(pointer_type) + inner->mangle_type();
  }
  std::string debug_str() const override
  {
    return EPtrType_to_str(pointer_type);
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Table final : public AType {
  std::optional<size_t> table_size; // nullopt = dynamic
  std::unique_ptr<Node> sizeSymbol;

  std::unique_ptr<AType> inner;

  std::string mangle_type() const override
  {
    if (table_size) return "list_" + inner->mangle_type();
    return "arr" + std::to_string(table_size.value()) + "_" + inner->mangle_type();
  }
  std::string debug_str() const override
  {
    return "<type> table";
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Primitive final : public AType {
  EPrimType type = EPrimType::Void;

  Primitive() = default;
  Primitive(EPrimType p_type)
    : type(p_type)
  {
  }

  std::string mangle_type() const override
  {
    return EPrimTy_to_mangle(type);
  }
  std::string debug_str() const override
  {
    return EPrimTy_to_str(type);
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Tuple final : public AType {
  std::vector<std::unique_ptr<AType>> types;
  std::vector<std::string>            name_fields;

  std::string mangle_type() const override
  {
    std::string out = "tuple" + std::to_string(types.size());
    for (auto& ty : types) out += ty->mangle_type();
    return out;
  }
  std::string debug_str() const override
  {
    if (name_fields.empty()) return "<type> tuple(" + std::to_string(types.size()) + ")";
    return "<type> named tuple(" + std::to_string(types.size()) + ")";
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Function_Proto final : public AType {
  std::vector<std::shared_ptr<declaration::local::Parameter>>         parameters;
  std::vector<std::unique_ptr<declaration::local::Generic_Parameter>> gen_parameters;
  std::unique_ptr<Tuple>                                              returnType;

  bool isVariadic = false;

  std::string mangle_type() const override
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
  std::string debug_str() const override
  {
    return "<type> fn";
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Get_Expr_Type final : public AType {
  std::unique_ptr<AExpression> target;

  std::string mangle_type() const override
  {
    return target->inferred_type->mangle_type();
  }
  std::string debug_str() const override
  {
    return "<type> comptime";
  };
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

} // namespace type
  // Type
} // namespace ast
  // AST