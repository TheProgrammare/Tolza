#pragma once

#include <memory>
#include <cstddef>
#include <string>
#include <vector>

#include "ast_base.hpp"


struct Visitor_Base;

namespace ast
{
namespace type
{

struct Primitive;

struct Ptr final : public AType {
  EPtrType               pointer_type = EPtrType::raw_ptr;
  std::shared_ptr<AType> inner;

  CODEGEN_TY

  std::string mangle_type() const override
  {
    return EPtrType_to_mangle(pointer_type) + "." + inner->mangle_type();
  }
  bool        compare_with(const AType& other) const override;
  std::string debug_str() const override
  {
    return "type " + EPtrType_to_str(pointer_type);
  }
  VISTOR_ACCEPT
};

struct Table final : public AType {
  size_t table_size; // 0 = dynamic
  [[maybe_unused]]
  std::unique_ptr<AExpression> size_sym;

  std::shared_ptr<AType> inner;

  CODEGEN_TY

  std::string mangle_type() const override
  {
    if (table_size > 0) return "sTbl" + inner->mangle_type();
    return "dTbl" + std::to_string(table_size) + "_" + inner->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Table*>(&other)) {
      if (table_size != ptr->table_size) return false;

      return inner->is_same(*ptr->inner);
    }
    return false;
  }
  std::string debug_str() const override
  {
    if (table_size) return "type table[" + size_sym->debug_str() + " -&gt; " + std::to_string(table_size) + "]";
    return "type table[" + size_sym->debug_str() + "]";
  }
  VISTOR_ACCEPT
};

struct Matrix final : public AType {
  Table  tbl;
  size_t dimension_size;
  [[maybe_unused]]

  CODEGEN_TY

      std::string mangle_type() const override
  {
    if (tbl.table_size > 0) return "sMtx" + tbl.inner->mangle_type() + "_" + std::to_string(dimension_size);
    return "dMtx" + std::to_string(tbl.table_size) + "_" + tbl.inner->mangle_type() + "_"
           + std::to_string(dimension_size);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Matrix*>(&other)) {
      if (dimension_size != ptr->dimension_size) return false;
      return tbl.is_same(ptr->tbl);
    }
    return false;
  }
  std::string debug_str() const override
  {
    if (tbl.table_size)
      return "type matrix[" + tbl.size_sym->debug_str() + " -&gt; " + std::to_string(tbl.table_size) + "]*"
             + std::to_string(dimension_size);
    return "type matrix[" + tbl.size_sym->debug_str() + "]*" + std::to_string(dimension_size);
  }
  VISTOR_ACCEPT
};

struct Hyper final : public AType {
  Table                        tbl;
  std::unique_ptr<AExpression> dimensionSymbol;

  CODEGEN_TY

  std::string mangle_type() const override
  {
    if (tbl.table_size > 0) return "sHpr" + tbl.inner->mangle_type();
    return "dHpr" + std::to_string(tbl.table_size) + "_" + tbl.inner->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Hyper*>(&other)) {
      return tbl.is_same(ptr->tbl);
    }
    return false;
  }
  std::string debug_str() const override
  {
    if (tbl.table_size)
      return "type hyper[" + tbl.size_sym->debug_str() + " -&gt; " + std::to_string(tbl.table_size) + "]";
    return "type hyper[" + tbl.size_sym->debug_str() + "]";
  }
  VISTOR_ACCEPT
};

struct Primitive final : public AType {
  EPrimType type = EPrimType::u0;

  Primitive() = default;
  Primitive(EPrimType p_type)
    : type(p_type)
  {
  }

  CODEGEN_TY

  std::string mangle_type() const override
  {
    return EPrimType_to_mangle(type);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Primitive*>(&other)) {
      return type == ptr->type;
    }
    return false;
  }
  std::string debug_str() const override
  {
    return "type " + EPrimType_to_str(type);
  }
  VISTOR_ACCEPT
};

struct Tuple final : public AType {
  std::vector<std::shared_ptr<AType>> types;
  std::vector<std::string>            name_fields;

  CODEGEN_TY

  std::string mangle_type() const override
  {
    std::string out = "tu." + std::to_string(types.size());
    for (auto& ty : types) out += "." + ty->mangle_type();
    return out;
  }
  std::string debug_str() const override
  {
    if (name_fields.empty()) return "type tuple(" + std::to_string(types.size()) + ")";
    return "type named tuple(" + std::to_string(types.size()) + ")";
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Tuple*>(&other)) {
      if (types.size() != ptr->types.size()) return false;
      for (size_t i = 0; i < types.size(); i++) {
        if (!types[i]->is_same(*ptr->types[i])) return false;
      }
      return true;
    }
    return false;
  }
  VISTOR_ACCEPT
};

struct Function_Proto final : public AType {
  ~Function_Proto();

  [[maybe_unused]] std::vector<std::shared_ptr<declaration::local::Parameter>>                 parameters;
  [[maybe_unused]] std::vector<std::unique_ptr<declaration::local::Generic_Parameter_Element>> gen_parameters;
  [[maybe_unused]] std::shared_ptr<ast::AType>                                                 return_ty;

  bool is_variadic             = false;
  bool is_explicit_return_type = false;

  CODEGEN_TY

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override;

  std::string debug_str() const override;

  VISTOR_ACCEPT
};

struct Get_Expr_Type final : public AType {
  std::unique_ptr<AExpression> target;

  CODEGEN_TY

  std::string mangle_type() const override
  {
    return target->expression_inferred_type->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    return target->expression_inferred_type->is_same(other);
  }
  std::string debug_str() const override
  {
    return "type get expression type";
  }

  VISTOR_ACCEPT
};

} // namespace type
  // Type
} // namespace ast
  // AST