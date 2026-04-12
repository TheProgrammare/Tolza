#pragma once

#include <memory>
#include <vector>

#include "ast/ast_forward.hpp"

namespace parser
{
struct Parser_Context;
struct Parser_Type {
  Parser_Type(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  [[nodiscard]] std::shared_ptr<ast::AType> parse_type();

private:
  [[nodiscard]] std::shared_ptr<ast::type::Table>     table(bool p_is_const, bool p_is_optional, bool p_is_volatile);
  [[nodiscard]] std::shared_ptr<ast::type::Ptr>       pointer(bool p_is_const, bool p_is_optional, bool p_is_volatile);
  [[nodiscard]] std::shared_ptr<ast::type::Primitive> primitive(bool p_is_const, bool p_is_optional,
                                                                bool p_is_volatile);
  [[nodiscard]] std::shared_ptr<ast::Expr_ID_Type>    id_type(bool p_is_const, bool p_is_optional, bool p_is_volatile);
  [[nodiscard]] std::shared_ptr<ast::type::Tuple>     tuple(bool p_is_const, bool p_is_optional, bool p_is_volatile);
  [[nodiscard]] std::shared_ptr<ast::type::Function_Proto> function_proto(bool p_is_const, bool p_is_optional,
                                                                          bool p_is_volatile);
  // isRef, isConst, isOptional
  [[nodiscard]] std::tuple<bool, bool, bool>               get_type_annotation();

public:
  [[nodiscard]] std::shared_ptr<ast::type::Tuple>          explicit_tuple();
  [[nodiscard]] std::shared_ptr<ast::type::Get_Expr_Type>  expr_get_expr_type();
  [[nodiscard]] std::shared_ptr<ast::type::Function_Proto> explicit_function_proto(bool p_is_lam = false);
  [[nodiscard]] std::vector<std::shared_ptr<ast::declaration::local::Parameter>> parameters();

  Parser_Context& ctx;
};
} // namespace parser