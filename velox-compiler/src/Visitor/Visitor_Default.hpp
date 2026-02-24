#pragma once

#include <vector>

#include "AST/AST_Base.hpp"
#include "AST/AST_Memory.hpp"
#include "ErrorOutput.hpp"
#include "Globals.hpp"
#include "Visitor_Base.hpp"

struct Visitor_Default : public Visitor_Base {
  using Visitor_Base::Visitor_Base;

  template <size_t Code>
  void error_add(const AST::Node& n, const std::string& msg, const std::string& hint)
  {
    auto error = Error_Diagnostic<Code>(scr_info, n._token, {}, current_EPhase(), EErrorSeverity::error, {}, msg, hint);

    errors.push_back(error.print_error());
  }

  template <size_t Code>
  void error_two_lines(const AST::Node& first, const ScriptInfo& first_scr_info, const AST::Node& second,
                       const ScriptInfo& second_scr_info, const std::string& code, const std::string& msg,
                       const std::string& hint)
  {
    auto first_error = Error_Diagnostic<Code>(first_scr_info, first._token, {}, current_EPhase(), EErrorSeverity::error,
                                              {}, msg, hint);

    auto second_error = Error_Diagnostic<Code>(second_scr_info, second._token, {}, current_EPhase(),
                                               EErrorSeverity::error, {}, msg, hint);

    std::string out = "[from file] " color_MAGENTA + first_error.print_source() + color_RESET "\n";
    out += first_error.print_line() + color_RESET "\n";
    out += "[to file]   " color_MAGENTA + second_error.print_source() + color_RESET "\n";
    out += second_error.print_line() + color_RESET "\n";

    out += first_error.print_messages();
    errors.push_back(out);
  }

  Config::EPhase current_EPhase() { return Config::EPhase::resolver_symbol; }


  // ============ AST ============
  void visit(AST::Node& n) override;

  void visit(AST::AType& n) override;
  void visit(AST::ALiteral& n) override;
  void visit(AST::ADeclaration& n) override;
  void visit(AST::ALocal& n) override;
  void visit(AST::AExpression& n) override;
  void visit(AST::Expr_ID& n) override;
  void visit(AST::Expr_ID_Qualified& n) override;
  void visit(AST::Expr_ID_Generic& n) override;

  void visit(AST::Root& n) override;

  // ============ DECLARATION ============
  void visit(AST::Declaration::Global& n) override;
  void visit(AST::Declaration::Function& n) override;

  void visit(AST::Declaration::Mod& n) override;
  void visit(AST::Declaration::Export& n) override;

  void visit(AST::Declaration::Enum& n) override;
  void visit(AST::Declaration::Enum_Element& n) override;

  void visit(AST::Declaration::Flag& n) override;

  void visit(AST::Declaration::Mod_Alias& n) override;
  void visit(AST::Declaration::Type_Alias& n) override;

  void visit(AST::Declaration::Generic& n) override;

  // ============ LOCAL ============
  void visit(AST::Declaration::Local::CodeBlock& n) override;

  void visit(AST::Declaration::Local::Lambda& n) override;
  void visit(AST::Declaration::Local::Lambda_Capture& n) override;
  void visit(AST::Declaration::Local::Capture_Member& n) override;

  void visit(AST::Declaration::Local::Parameter& n) override;
  void visit(AST::Declaration::Local::Generic_Parameter& n) override;

  void visit(AST::Declaration::Local::Pattern& n) override;
  void visit(AST::Declaration::Local::Pattern_Enum& n) override;
  void visit(AST::Declaration::Local::Pattern_Tuple& n) override;
  void visit(AST::Declaration::Local::Pattern_Entity& n) override;
  void visit(AST::Declaration::Local::Pattern_Component& n) override;

  void visit(AST::Declaration::Local::Variable_Binding& n) override;
  void visit(AST::Declaration::Local::Variable_Unpack& n) override;
  void visit(AST::Declaration::Local::Variable& n) override;

  void visit(AST::Declaration::Local::Capability& n) override;

  // ============ COP ============
  void visit(AST::Declaration::COP::Component& n) override;
  void visit(AST::Declaration::COP::Component_Field& n) override;

  void visit(AST::Declaration::COP::Role& n) override;

  void visit(AST::Declaration::COP::Entity& n) override;
  void visit(AST::Declaration::COP::Entity_Cast& n) override;
  void visit(AST::Declaration::COP::Entity_Op& n) override;
  void visit(AST::Declaration::COP::Entity_OpIndex& n) override;

  void visit(AST::Declaration::COP::System& n) override;
  void visit(AST::Declaration::COP::System_Case& n) override;

  // ============ GENERIC ============
  void visit(AST::Generic::Is_Type& n) override;
  void visit(AST::Generic::Can_Cast& n) override;
  void visit(AST::Generic::Have_Op& n) override;
  void visit(AST::Generic::Have_Role& n) override;
  void visit(AST::Generic::Use_Component& n) override;
  void visit(AST::Generic::Compatible_System& n) override;

  // ============ TYPE ============
  void visit(AST::Type::Ptr& n) override;
  void visit(AST::Type::Table& n) override;
  void visit(AST::Type::Primitive& n) override;
  void visit(AST::Type::Tuple& n) override;
  void visit(AST::Type::Function_Proto& n) override;

  void visit(AST::Type::Get_Expr_Type& n) override;

  // ============ LITERAL ============
  void visit(AST::Literal::Boolean& n) override;
  void visit(AST::Literal::Integral& n) override;
  void visit(AST::Literal::Decimal& n) override;
  void visit(AST::Literal::Floating& n) override;

  void visit(AST::Literal::ASCII& n) override;
  void visit(AST::Literal::UFT32& n) override;

  void visit(AST::Literal::Text& n) override;
  void visit(AST::Literal::Text_Lerp& n) override;
  void visit(AST::Literal::Textual_Element& n) override;
  void visit(AST::Literal::Textual_Format& n) override;
  void visit(AST::Literal::Format_Specifier& n) override;

  void visit(AST::Literal::Table& n) override;
  void visit(AST::Literal::Table_Population& n) override;

  void visit(AST::Literal::Map& n) override;

  void visit(AST::Literal::Tuple& n) override;

  void visit(AST::Literal::Range& n) override;

  void visit(AST::Literal::Component& n) override;
  void visit(AST::Literal::Entity& n) override;

  // ============ Expression ============
  void visit(AST::Expression::If_Ternary& n) override;
  void visit(AST::Expression::Enum& n) override;

  void visit(AST::Expression::Member_Access& n) override;

  void visit(AST::Expression::Self& n) override;
  void visit(AST::Expression::Other& n) override;

  void visit(AST::Expression::Call& n) override;
  void visit(AST::Expression::Call_Argument& n) override;
  void visit(AST::Expression::Call_System& n) override;
  void visit(AST::Expression::Call_Pipe& n) override;

  void visit(AST::Expression::Table_Access& n) override;

  void visit(AST::Expression::Ptr_At& n) override;
  void visit(AST::Expression::Ptr_Offset& n) override;
  void visit(AST::Expression::Ptr_Val& n) override;
  void visit(AST::Expression::Addr_Of& n) override;
  void visit(AST::Expression::Size_Of& n) override;
  void visit(AST::Expression::GetBits& n) override;

  void visit(AST::Expression::Move& n) override;
  void visit(AST::Expression::New_Ptr& n) override;

  // ============ STATEMENT ============
  void visit(AST::Statement::If& n) override;

  void visit(AST::Statement::For& n) override;
  void visit(AST::Statement::Loop& n) override;
  void visit(AST::Statement::While& n) override;
  void visit(AST::Statement::GoTo& n) override;
  void visit(AST::Statement::GoTo_Label& n) override;

  void visit(AST::Statement::Return& n) override;
  void visit(AST::Statement::Break& n) override;
  void visit(AST::Statement::Continue& n) override;

  void visit(AST::Statement::Match& n) override;
  void visit(AST::Statement::Match_Case& n) override;

  // ============ OPERATION ============
  void visit(AST::Operation::Cast_As& n) override;
  void visit(AST::Operation::Is& n) override;
  void visit(AST::Operation::In& n) override;
  void visit(AST::Operation::Assignment& n) override;
  void visit(AST::Operation::Binary& n) override;
  void visit(AST::Operation::Unary& n) override;
  void visit(AST::Operation::Interval& n) override;
  void visit(AST::Operation::Ptr_Dist& n) override;

  // ============ MEMORY ============
  void visit(AST::Memory::Del& n) override;
  void visit(AST::Memory::Align& n) override;
  void visit(AST::Memory::Drop& n) override;
};
