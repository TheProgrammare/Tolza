#pragma once

#include <memory>
#include <vector>

#include "ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Type {
  Parser_Type(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::unique_ptr<ast::AType> parse_type();

private:
  [[nodiscard]] std::unique_ptr<ast::type::Table>          table(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<ast::type::Ptr>            pointer(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<ast::type::Primitive>      primitive(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<ast::Expr_ID_Generic>      id_type(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<ast::type::Tuple>          tuple(bool isConst, bool isOptional, bool isVolatile);
  [[nodiscard]] std::unique_ptr<ast::type::Function_Proto> function_proto(bool isConst, bool isOptional,
                                                                          bool isVolatile);
  // isRef, isConst, isOptional
  [[nodiscard]] std::tuple<bool, bool, bool>               get_type_annotation();

public:
  [[nodiscard]] std::unique_ptr<ast::type::Tuple>          explicit_tuple();
  [[nodiscard]] std::unique_ptr<ast::type::Get_Expr_Type>  expr_get_expr_type();
  [[nodiscard]] std::shared_ptr<ast::type::Function_Proto> explicit_function_proto(bool isLam = false);
  [[nodiscard]] std::vector<std::shared_ptr<ast::declaration::local::Parameter>> parameters();

  Parser_Context& ctx;
};
} // namespace parser