#pragma once

#include <memory>
#include <vector>

#include "AST/AST_Forward.hpp"

namespace PAR
{
struct Parser_Context;
struct Parser_Type {
  Parser_Type(Parser_Context &ctx) : ctx(ctx) {}

  [[nodiscard]] std::unique_ptr<AST::AType> parse_type();

private:
  [[nodiscard]] std::unique_ptr<AST::Type::Table>          table(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<AST::Type::Ptr>            pointer(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<AST::Type::Primitive>      primitive(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<AST::Expr_ID_Generic>      id_type(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<AST::Type::Tuple>          tuple(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<AST::Type::Function_Proto> function_proto(bool isConst, bool isOptional,
                                                                          bool isVolatile);
  // isRef, isConst, isOptional
  [[nodiscard]] std::tuple<bool, bool, bool> get_type_annotation();

public:
  [[nodiscard]] std::unique_ptr<AST::Type::Tuple>          explicit_tuple();
  [[nodiscard]] std::unique_ptr<AST::Type::Get_Expr_Type>  expr_get_expr_type();
  [[nodiscard]] std::shared_ptr<AST::Type::Function_Proto> explicit_function_proto(bool isLam = false);
  [[nodiscard]] std::vector<std::shared_ptr<AST::Declaration::Local::Parameter>> parameters();

  Parser_Context &ctx;
};
} // namespace PAR