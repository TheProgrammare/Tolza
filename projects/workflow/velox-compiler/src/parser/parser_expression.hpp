#pragma once

#include <memory>
#include <vector>

#include "ast/ast_forward.hpp"

enum class EExtern_Kind;

namespace parser
{
struct Parser_Context;
struct Parser_Expression {
  Parser_Expression(Parser_Context& p_ctx)
    : ctx(p_ctx)
  {
  }

  [[nodiscard]] std::unique_ptr<ast::AExpression> parse_expression();
  [[nodiscard]] std::unique_ptr<ast::AExpression> parse_expression_term();
  [[nodiscard]] std::unique_ptr<ast::AExpression> base_expression();
  [[nodiscard]] std::unique_ptr<ast::AExpression> suffix_expression(std::unique_ptr<ast::AExpression> p_base_expr);
  [[nodiscard]] std::unique_ptr<ast::operation::Cast_As> cast_as(std::unique_ptr<ast::AExpression> p_expr);

  [[nodiscard]] std::unique_ptr<ast::AIdentifier>               identifier(bool p_no_qualified_id = false,
                                                                           bool p_keyword_allowed = false);
  [[nodiscard]] std::unique_ptr<ast::Expr_ID_Type>              identifier_typed();
  [[nodiscard]] std::unique_ptr<ast::expression::Member_Access> member_access(std::unique_ptr<ast::AExpression> p_left);
  [[nodiscard]] std::unique_ptr<ast::expression::Table_Access> table_access(std::unique_ptr<ast::AExpression> p_target);
  [[nodiscard]] std::unique_ptr<ast::expression::Call> function_call(std::unique_ptr<ast::AExpression> p_callee);
  [[nodiscard]] std::unique_ptr<ast::expression::Call_System>
  system_call(std::unique_ptr<ast::AExpression> p_target_entity);
  [[nodiscard]] std::vector<std::unique_ptr<ast::expression::Call_Argument>> call_arguments();
  [[nodiscard]] std::unique_ptr<ast::expression::If_Ternary>                 if_ternary();

  // special memory expression
  [[nodiscard]] std::unique_ptr<ast::expression::New_Ptr>    new_ptr();
  [[nodiscard]] std::unique_ptr<ast::expression::Ptr_Val>    ptr_val();
  [[nodiscard]] std::unique_ptr<ast::expression::Ref_Of>     ref_of();
  [[nodiscard]] std::unique_ptr<ast::expression::Mut_Of>     mut_of();
  [[nodiscard]] std::unique_ptr<ast::expression::Addr_Of>    addr_of();
  [[nodiscard]] std::unique_ptr<ast::expression::Size_Of>    size_of();
  [[nodiscard]] std::unique_ptr<ast::expression::Move>       move();
  [[nodiscard]] std::unique_ptr<ast::expression::GetBits>    getbits(std::unique_ptr<ast::AExpression> p_expr);
  [[nodiscard]] std::unique_ptr<ast::expression::Ptr_At>     ptr_at(std::unique_ptr<ast::AExpression> p_ref);
  [[nodiscard]] std::unique_ptr<ast::expression::Ptr_Offset> ptr_offset(std::unique_ptr<ast::AExpression> p_ref);

  Parser_Context& ctx;
};
} // namespace parser