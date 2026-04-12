#pragma once

#include "ast/ast_declaration_cop.hpp"
#include "visitor_base.hpp"

struct Visitor_Print : public Visitor_Base {
  using Visitor_Base::Visitor_Base;

  std::string out_print;

  // ============ AST ============
  void visit(ast::Node& n) override;

  void visit(ast::AType& n) override;
  void visit(ast::ALiteral& n) override;
  void visit(ast::ADeclaration& n) override;
  void visit(ast::ALocal& n) override;
  void visit(ast::AExpression& n) override;
  void visit(ast::AIdentifier& n) override;
  void visit(ast::Expr_ID& n) override;
  void visit(ast::Expr_ID_Qualified& n) override;
  void visit(ast::Expr_ID_Type& n) override;

  void visit(ast::Root& n) override;

  // ============ DECLARATION ============
  void visit(ast::declaration::Global& n) override;
  void visit(ast::declaration::Function& n) override;

  void visit(ast::declaration::Mod& n) override;
  void visit(ast::declaration::Export& n) override;
  void visit(ast::declaration::Extern& n) override;

  void visit(ast::declaration::Enum& n) override;
  void visit(ast::declaration::Enum_Element& n) override;

  void visit(ast::declaration::Flag& n) override;
  void visit(ast::declaration::Union& n) override;

  void visit(ast::declaration::Mod_Alias& n) override;
  void visit(ast::declaration::Type_Alias& n) override;

  void visit(ast::declaration::Generic& n) override;

  // ============ LOCAL ============
  void visit(ast::declaration::local::CodeBlock& n) override;

  void visit(ast::declaration::local::Lambda& n) override;
  void visit(ast::declaration::local::Lambda_Capture& n) override;
  void visit(ast::declaration::local::Capture_Member& n) override;

  void visit(ast::declaration::local::Parameter& n) override;
  void visit(ast::declaration::local::Generic_Parameter_Element& n) override;
  void visit(ast::declaration::local::Generic_Parameters& n) override;

  void visit(ast::declaration::local::Pattern& n) override;
  void visit(ast::declaration::local::Pattern_Enum& n) override;
  void visit(ast::declaration::local::Pattern_Tuple& n) override;
  void visit(ast::declaration::local::Pattern_Entity& n) override;
  void visit(ast::declaration::local::Pattern_System_Component& n) override;
  void visit(ast::declaration::local::Pattern_Component& n) override;

  void visit(ast::declaration::local::Variable_Binding& n) override;
  void visit(ast::declaration::local::Tuple_Destructuring& n) override;
  void visit(ast::declaration::local::Variable& n) override;

  void visit(ast::declaration::local::Capability& n) override;

  // ============ COP ============
  void visit(ast::declaration::cop::Component& n) override;
  void visit(ast::declaration::cop::Component_Field& n) override;

  void visit(ast::declaration::cop::Role& n) override;

  void visit(ast::declaration::cop::Entity& n) override;
  void visit(ast::declaration::cop::Entity_New& n) override;
  void visit(ast::declaration::cop::Entity_Del& n) override;
  void visit(ast::declaration::cop::Entity_Cast& n) override;
  void visit(ast::declaration::cop::Entity_Op& n) override;
  void visit(ast::declaration::cop::Entity_Access_Op& n) override;
  void visit(ast::declaration::cop::Entity_Transfert& n) override;

  void visit(ast::declaration::cop::System& n) override;
  void visit(ast::declaration::cop::System_Case& n) override;

  // ============ GENERIC ============
  void visit(ast::generic::Is_Type& n) override;
  void visit(ast::generic::Can_Cast& n) override;
  void visit(ast::generic::Have_Op& n) override;
  void visit(ast::generic::Have_Role& n) override;
  void visit(ast::generic::Use_Component& n) override;
  void visit(ast::generic::Compatible_System& n) override;

  // ============ TYPE ============
  void visit(ast::type::Ptr& n) override;
  void visit(ast::type::Table& n) override;
  void visit(ast::type::Primitive& n) override;
  void visit(ast::type::Tuple& n) override;
  void visit(ast::type::Function_Proto& n) override;

  void visit(ast::type::Get_Expr_Type& n) override;

  // ============ LITERAL ============
  void visit(ast::literal::Boolean& n) override;
  void visit(ast::literal::Integral& n) override;
  void visit(ast::literal::Fixed_Point& n) override;
  void visit(ast::literal::Floating_Point& n) override;

  void visit(ast::literal::CUNE& n) override;
  void visit(ast::literal::RUNE& n) override;

  void visit(ast::literal::Text_Pure& n) override;
  void visit(ast::literal::Text_Interpolation& n) override;
  void visit(ast::literal::Textual_Format& n) override;
  void visit(ast::literal::Format_Specifier& n) override;

  void visit(ast::literal::Table& n) override;
  void visit(ast::literal::Table_Population& n) override;

  void visit(ast::literal::Map& n) override;

  void visit(ast::literal::Tuple& n) override;

  void visit(ast::literal::Range& n) override;
  void visit(ast::literal::Iterator& n) override;

  void visit(ast::literal::Enum& n) override;

  void visit(ast::literal::Structured_Data& n) override;
  void visit(ast::literal::Entity& n) override;

  // ============ Expression ============
  void visit(ast::expression::If_Ternary& n) override;

  void visit(ast::expression::Member_Access& n) override;

  void visit(ast::expression::Self& n) override;
  void visit(ast::expression::Other& n) override;

  void visit(ast::expression::Call& n) override;
  void visit(ast::expression::Call_Argument& n) override;
  void visit(ast::expression::Call_System& n) override;
  void visit(ast::expression::Call_Pipe& n) override;

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

  // ============ STATEMENT ============
  void visit(ast::statement::If& n) override;

  void visit(ast::statement::For& n) override;
  void visit(ast::statement::Loop& n) override;
  void visit(ast::statement::While& n) override;
  void visit(ast::statement::GoTo& n) override;
  void visit(ast::statement::GoTo_Label& n) override;

  void visit(ast::statement::Return& n) override;
  void visit(ast::statement::Break& n) override;
  void visit(ast::statement::Continue& n) override;

  void visit(ast::statement::Match& n) override;
  void visit(ast::statement::Match_Case& n) override;

  // ============ OPERATION ============
  void visit(ast::operation::Cast_As& n) override;
  void visit(ast::operation::Is& n) override;
  void visit(ast::operation::In& n) override;
  void visit(ast::operation::Assignment& n) override;
  void visit(ast::operation::Binary& n) override;
  void visit(ast::operation::Unary& n) override;
  void visit(ast::operation::Interval& n) override;
  void visit(ast::operation::Ptr_Dist& n) override;

  // ============ MEMORY ============
  void visit(ast::memory::Del& n) override;
  void visit(ast::memory::Align& n) override;
  void visit(ast::memory::Drop& n) override;

  std::string get_file_path() const;
};

// %0 velox-compiler version
// %1 file path
// %2 ast view
extern const char* PRINT_FILE_HTLM_TEMPLATE;