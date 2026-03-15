#pragma once

#include <memory>
#include <cstddef>
#include <string>

#include "ast_base.hpp"


struct Visitor_Base;

namespace ast
{
namespace type
{

struct Ptr final : public AType {
  EPtrType               pointer_type = EPtrType::raw_ptr;
  std::unique_ptr<AType> inner;

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override
  {
    return EPtrType_to_mangle(pointer_type) + inner->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Ptr*>(&other)) {
      return inner->is_same(*ptr->inner) && pointer_type == ptr->pointer_type;
    }
    return false;
  }
  std::string debug_str() const override
  {
    return "type " + EPtrType_to_str(pointer_type);
  }
  void accept(Visitor_Base& v) override;
};

struct Table final : public AType {
  std::optional<size_t> table_size; // nullopt = dynamic
  std::unique_ptr<Node> sizeSymbol;

  std::unique_ptr<AType> inner;

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override
  {
    if (table_size) return "list_" + inner->mangle_type();
    return "arr" + std::to_string(table_size.value()) + "_" + inner->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Table*>(&other)) {
      if (table_size.has_value() != table_size.has_value()) return false;

      if (table_size) {
        if (table_size.value() != ptr->table_size.value()) return false;
      }

      return inner->is_same(*ptr->inner);
    }
    return false;
  }
  std::string debug_str() const override
  {
    if (table_size)
      return "type table[" + sizeSymbol->debug_str() + " -&gt; " + std::to_string(table_size.value()) + "]";
    return "type table[" + sizeSymbol->debug_str() + "]";
  }
  void accept(Visitor_Base& v) override;
};

struct Primitive final : public AType {
  EPrimType type = EPrimType::u0;

  Primitive() = default;
  Primitive(EPrimType p_type)
    : type(p_type)
  {
  }

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override
  {
    return EPrimTy_to_mangle(type);
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
    return "type " + EPrimTy_to_str(type);
  }
  void accept(Visitor_Base& v) override;
};

struct Tuple final : public AType {
  std::vector<std::shared_ptr<AType>> types;
  std::vector<std::string>            name_fields;

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override
  {
    std::string out = "tuple" + std::to_string(types.size());
    for (auto& ty : types) out += ty->mangle_type();
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
  void accept(Visitor_Base& v) override;
};

struct Function_Proto final : public AType {
  ~Function_Proto();

  [[maybe_unused]] std::vector<std::shared_ptr<declaration::local::Parameter>>                 parameters;
  [[maybe_unused]] std::vector<std::unique_ptr<declaration::local::Generic_Parameter_Element>> gen_parameters;
  [[maybe_unused]] std::unique_ptr<Tuple>                                                      returnType;

  bool isVariadic = false;

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override;

  std::string debug_str() const override;

  void accept(Visitor_Base& v) override;
};

struct Get_Expr_Type final : public AType {
  std::unique_ptr<AExpression> target;

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string mangle_type() const override
  {
    return target->inferred_type->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    return target->inferred_type->is_same(other);
  }
  std::string debug_str() const override
  {
    return "type get expression type";
  }

  void accept(Visitor_Base& v) override;
};

} // namespace type
  // Type
} // namespace ast
  // AST