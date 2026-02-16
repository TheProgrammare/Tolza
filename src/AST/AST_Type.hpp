#pragma once

#include "AST/AST_Data.hpp"
#include "AST_Base.hpp"

struct Visitor_Base;

namespace AST
{
namespace Type
{

struct Ptr final : public AType {
  EPtrType               pointer_type = EPtrType::raw_ptr;
  std::unique_ptr<AType> inner;

  std::string debug_str() const override { return EPtrType_to_str(pointer_type); }

  void accept(Visitor_Base &v) override { v.visit(*this); }
};

struct Table final : public AType {
  std::optional<size_t> tableSize; // nullopt = dynamic
  std::unique_ptr<Node> sizeSymbol;

  std::unique_ptr<AType> inner;

  std::string debug_str() const override { return "<type> table"; }

  void accept(Visitor_Base &v) override { v.visit(*this); }
};

struct Primitive final : public AType {
  EPrimType _type = EPrimType::u8;

  Primitive(EPrimType p_type) : _type(p_type) {}

  std::string debug_str() const override { return EPrimTy_to_str(_type); }
  void        accept(Visitor_Base &v) override { v.visit(*this); }
};

struct Tuple final : public AType {
  std::vector<std::unique_ptr<AType>> types;
  std::vector<std::string>            name_fields;

  std::string debug_str() const override
  {
    if (name_fields.empty()) return "<type> tuple(" + std::to_string(types.size()) + ")";
    return "<type> named tuple(" + std::to_string(types.size()) + ")";
  }

  void accept(Visitor_Base &v) override { v.visit(*this); }
};

struct Function_Proto final : public AType {
  std::vector<std::shared_ptr<Declaration::Local::Parameter>>         parameters;
  std::vector<std::unique_ptr<Declaration::Local::Generic_Parameter>> gen_parameters;
  std::unique_ptr<Tuple>                                              returnType;

  bool                   isVariadic = false;
  std::shared_ptr<AType> variadic_type;
  size_t                 Variadic_start_pos = 0;

  std::string debug_str() const override { return "<type> fn"; }

  void accept(Visitor_Base &v) override { v.visit(*this); }
};

struct Get_Expr_Type final : public AType {
  std::unique_ptr<AExpression> target;

  std::string debug_str() const override { return "<type> comptime"; };

  void accept(Visitor_Base &v) override { v.visit(*this); }
};

} // namespace Type
} // namespace AST