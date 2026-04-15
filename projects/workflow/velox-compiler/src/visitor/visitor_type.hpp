#pragma once

#include <memory>

#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_forward.hpp"
#include "ast/ast_literal.hpp"
#include "visitor_default.hpp"

namespace ast
{
struct ADeclaration;
using SYM_REF = std::shared_ptr<ast::ADeclaration>;
} // namespace ast

struct Visitor_Type : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  bool                        is_same_type(const ast::AType& p_type_1, const ast::AType& p_type_2) const;
  std::shared_ptr<ast::AType> resolve_type(ast::Node& n, ast::AType& input_type, bool p_is_silent_error = false);

  void ensure_expression_resolution(ast::AExpression& p_expr, std::shared_ptr<ast::AType> p_type_inferrance);


  std::shared_ptr<ast::AType> get_inferred_type(ast::Node& n, bool p_is_prototype_expected = false) const;

  std::shared_ptr<ast::AType> get_symbol_type(const ast::Node& n, const ast::SYM_REF sym_data) const;

  // expression inferred type;
  void visit(ast::Expr_ID& n) override;
  void visit(ast::Expr_ID_Qualified& n) override;
  void visit(ast::Expr_ID_Type& n) override;

  void visit(ast::declaration::Global& n) override;
  void visit(ast::declaration::Function& n) override;

  void visit(ast::declaration::local::Pattern_Enum& n) override;
  void visit(ast::declaration::local::Pattern_Tuple& n) override;
  void visit(ast::declaration::local::Pattern_Entity& n) override;
  void visit(ast::declaration::local::Pattern_Component& n) override;


  void visit(ast::declaration::local::Variable& n) override;
  void visit(ast::declaration::local::Variable_Binding& n) override;

  void visit(ast::statement::Return& n) override;

  void visit(ast::expression::If_Ternary& n) override;
  void visit(ast::expression::Member_Access& n) override;
  void visit(ast::expression::Self& n) override;
  void visit(ast::expression::Other& n) override;
  void visit(ast::expression::Call_Argument& n) override;
  void visit(ast::expression::Call& n) override;
  void visit(ast::expression::Call_Pipe& n) override;
  void visit(ast::expression::Call_System& n) override;
  void visit(ast::expression::Table_Access& n) override;
  void visit(ast::expression::Ptr_At& n) override;
  void visit(ast::expression::Ptr_Offset& n) override;
  void visit(ast::expression::Ptr_Val& n) override;
  void visit(ast::expression::Mut_Of& n) override;
  void visit(ast::expression::Ref_Of& n) override;
  void visit(ast::expression::Addr_Of& n) override;
  void visit(ast::expression::Size_Of& n) override;
  void visit(ast::expression::GetBits& n) override;
  void visit(ast::expression::Move& n) override;
  void visit(ast::expression::New_Ptr& n) override;

  void visit(ast::literal::Integral& n) override;
  void visit(ast::literal::Floating_Point& n) override;
  void visit(ast::literal::Table& n) override;
  void visit(ast::literal::Map& n) override;
  void visit(ast::literal::Text_Interpolation& n) override;
  void visit(ast::literal::Text_Pure& n) override;
  void visit(ast::literal::Textual_Format& n) override;
  void visit(ast::literal::Enum& n) override;
  void visit(ast::literal::Tuple& n) override;
  void visit(ast::literal::Range& n) override;
  void visit(ast::literal::Structured_Data& n) override;
  void visit(ast::literal::Entity& n) override;
  void visit(ast::literal::Iterator& n) override;

  void visit(ast::statement::For& n) override;

  void visit(ast::operation::Cast_As& n) override;
  void visit(ast::operation::Is& n) override;
  void visit(ast::operation::In& n) override;
  void visit(ast::operation::Assignment& n) override;
  void visit(ast::operation::Binary& n) override;
  void visit(ast::operation::Unary& n) override;
  void visit(ast::operation::Interval& n) override;
  void visit(ast::operation::Ptr_Dist& n) override;


  // exception: non typed expression
  // void visit(ast::statement::GoTo &n) override;
};
