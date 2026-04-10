#pragma once

#include <memory>
#include <vector>

#include "ast/ast_forward.hpp"

enum class EExtern_Kind;

namespace parser
{
struct Parser_Context;
struct Parser_Expression {
  Parser_Expression(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::unique_ptr<ast::AExpression>        parse_expression();
  [[nodiscard]] std::unique_ptr<ast::AExpression>        parse_expression_term();
  [[nodiscard]] std::unique_ptr<ast::AExpression>        base_expression();
  [[nodiscard]] std::unique_ptr<ast::AExpression>        suffix_expression(std::unique_ptr<ast::AExpression> base_expr);
  [[nodiscard]] std::unique_ptr<ast::operation::Cast_As> cast_as(std::unique_ptr<ast::AExpression> expr);

  void check_reference_external(const std::string& name, const std::vector<std::string>& path, EExtern_Kind kind);

  //[[nodiscard]] std::unique_ptr<ast::expression::Call_Pipe> function_call_pipe();

  [[nodiscard]] std::unique_ptr<ast::AIdentifier>               identifier(bool no_qualified_id = false,
                                                                           bool keyword_allowed = false);
  [[nodiscard]] std::unique_ptr<ast::Expr_ID_Type>              identifier_typed();
  [[nodiscard]] std::unique_ptr<ast::expression::Member_Access> member_access(std::unique_ptr<ast::AExpression> left);
  [[nodiscard]] std::unique_ptr<ast::expression::Table_Access>  table_access(std::unique_ptr<ast::AExpression> target);
  [[nodiscard]] std::unique_ptr<ast::expression::Call>          function_call(std::unique_ptr<ast::AExpression> callee);
  [[nodiscard]] std::unique_ptr<ast::expression::Call_System>
  system_call(std::unique_ptr<ast::AExpression> target_entity);
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
  [[nodiscard]] std::unique_ptr<ast::expression::GetBits>    getbits(std::unique_ptr<ast::AExpression> expr);
  [[nodiscard]] std::unique_ptr<ast::expression::Ptr_At>     ptr_at(std::unique_ptr<ast::AExpression> ref);
  [[nodiscard]] std::unique_ptr<ast::expression::Ptr_Offset> ptr_offset(std::unique_ptr<ast::AExpression> ref);

  Parser_Context& ctx;
};
} // namespace parser