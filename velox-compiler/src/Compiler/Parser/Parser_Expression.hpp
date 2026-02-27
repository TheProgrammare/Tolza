#pragma once

#include <memory>
#include <vector>

#include "Compiler/AST/AST_Forward.hpp"
#include "Compiler/ScriptInfo.hpp"

namespace PAR
{
struct Parser_Context;
struct Parser_Expression {
  Parser_Expression(Parser_Context& ctx)
    : ctx(ctx)
  {
  }

  [[nodiscard]] std::unique_ptr<AST::AExpression>        parse_expression();
  [[nodiscard]] std::unique_ptr<AST::AExpression>        parse_expression_term();
  [[nodiscard]] std::unique_ptr<AST::Operation::Cast_As> cast_as(std::unique_ptr<AST::AExpression> expr);

  void check_reference_external(const std::string& name, const std::vector<std::string>& path, Extern_Item::Kind kind);

  //[[nodiscard]] std::unique_ptr<AST::Expression::Call_Pipe> function_call_pipe();

  [[nodiscard]] std::unique_ptr<AST::AIdentifier>                            identifier(bool no_qualified_id = false,
                                                                                        bool keyword_allowed = false);
  [[nodiscard]] std::unique_ptr<AST::Expr_ID_Generic>                        identifier_typed();
  [[nodiscard]] std::unique_ptr<AST::Expression::Member_Access>              member_access();
  [[nodiscard]] std::unique_ptr<AST::Expression::Table_Access>               table_access();
  [[nodiscard]] std::unique_ptr<AST::Expression::Call>                       function_call();
  [[nodiscard]] std::vector<std::unique_ptr<AST::Expression::Call_Argument>> call_arguments();
  [[nodiscard]] std::unique_ptr<AST::Expression::If_Ternary>                 if_ternary();

  // special memory expression
  [[nodiscard]] std::unique_ptr<AST::Expression::New_Ptr>    new_ptr();
  [[nodiscard]] std::unique_ptr<AST::Expression::Ptr_Val>    ptr_val();
  [[nodiscard]] std::unique_ptr<AST::Expression::Addr_Of>    addr_of();
  [[nodiscard]] std::unique_ptr<AST::Expression::Size_Of>    size_of();
  [[nodiscard]] std::unique_ptr<AST::Expression::Move>       move();
  [[nodiscard]] std::unique_ptr<AST::Expression::GetBits>    getbits(std::unique_ptr<AST::AExpression> expr);
  [[nodiscard]] std::unique_ptr<AST::Expression::Ptr_At>     ptr_at(std::unique_ptr<AST::AExpression> ref);
  [[nodiscard]] std::unique_ptr<AST::Expression::Ptr_Offset> ptr_offset(std::unique_ptr<AST::AExpression> ref);

  [[nodiscard]] ModuleImportation* get_external_source(const std::string& name, const std::vector<std::string>& path);

  Parser_Context& ctx;
};
} // namespace PAR