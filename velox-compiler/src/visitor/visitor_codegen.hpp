
#pragma once

#include <string>
#include <vector>

#include "ast/ast_forward.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

struct ScriptInfo;

using ErrorCode = short;

struct Visitor_Codegen {
  Visitor_Codegen(ScriptInfo& _scr_info);

  llvm::LLVMContext ctx;
  llvm::Module      mod;
  llvm::IRBuilder<> builder;

  ScriptInfo& scr_info;

  mutable std::vector<std::string> errors;


  void error_add(ErrorCode code, const ast::Node& n, const std::string& msg, const std::string& hint) const;

  void error_two_lines(ErrorCode code, const ast::Node& first, const ast::Node& second, const std::string& msg,
                       const std::string& hint) const;


  // ============ AST ============
  llvm::Value* visit(ast::Node& n);

  llvm::Value* visit(ast::AType& n);
  llvm::Value* visit(ast::ALiteral& n);
  llvm::Value* visit(ast::ADeclaration& n);
  llvm::Value* visit(ast::ALocal& n);
  llvm::Value* visit(ast::AExpression& n);
  llvm::Value* visit(ast::AIdentifier& n);
  llvm::Value* visit(ast::Expr_ID& n);
  llvm::Value* visit(ast::Expr_ID_Qualified& n);
  llvm::Value* visit(ast::Expr_ID_Type& n);

  llvm::Value* visit(ast::Root& n);

  // ============ DECLARATION ============
  llvm::Value* visit(ast::declaration::Global& n);
  llvm::Value* visit(ast::declaration::Function& n);

  llvm::Value* visit(ast::declaration::Mod& n);
  llvm::Value* visit(ast::declaration::Export& n);
  llvm::Value* visit(ast::declaration::Extern& n);

  llvm::Value* visit(ast::declaration::Enum& n);
  llvm::Value* visit(ast::declaration::Enum_Element& n);

  llvm::Value* visit(ast::declaration::Flag& n);

  llvm::Value* visit(ast::declaration::Mod_Alias& n);
  llvm::Value* visit(ast::declaration::Type_Alias& n);

  llvm::Value* visit(ast::declaration::Generic& n);

  // ============ LOCAL ============
  llvm::Value* visit(ast::declaration::local::CodeBlock& n);

  llvm::Value* visit(ast::declaration::local::Lambda& n);
  llvm::Value* visit(ast::declaration::local::Lambda_Capture& n);
  llvm::Value* visit(ast::declaration::local::Capture_Member& n);

  llvm::Value* visit(ast::declaration::local::Parameter& n);
  llvm::Value* visit(ast::declaration::local::Generic_Parameter_Element& n);
  llvm::Value* visit(ast::declaration::local::Generic_Parameters& n);

  llvm::Value* visit(ast::declaration::local::Pattern& n);
  llvm::Value* visit(ast::declaration::local::Pattern_Enum& n);
  llvm::Value* visit(ast::declaration::local::Pattern_Tuple& n);
  llvm::Value* visit(ast::declaration::local::Pattern_Entity& n);
  llvm::Value* visit(ast::declaration::local::Pattern_Component& n);

  llvm::Value* visit(ast::declaration::local::Variable_Binding& n);
  llvm::Value* visit(ast::declaration::local::Variable_Unpack& n);
  llvm::Value* visit(ast::declaration::local::Variable& n);

  llvm::Value* visit(ast::declaration::local::Capability& n);

  // ============ COP ============
  llvm::Value* visit(ast::declaration::cop::Component& n);
  llvm::Value* visit(ast::declaration::cop::Component_Field& n);

  llvm::Value* visit(ast::declaration::cop::Role& n);

  llvm::Value* visit(ast::declaration::cop::Entity& n);
  llvm::Value* visit(ast::declaration::cop::Entity_Cast& n);
  llvm::Value* visit(ast::declaration::cop::Entity_Op& n);
  llvm::Value* visit(ast::declaration::cop::Entity_OpIndex& n);

  llvm::Value* visit(ast::declaration::cop::System& n);
  llvm::Value* visit(ast::declaration::cop::System_Case& n);

  // ============ GENERIC ============
  llvm::Value* visit(ast::generic::Is_Type& n);
  llvm::Value* visit(ast::generic::Can_Cast& n);
  llvm::Value* visit(ast::generic::Have_Op& n);
  llvm::Value* visit(ast::generic::Have_Role& n);
  llvm::Value* visit(ast::generic::Use_Component& n);
  llvm::Value* visit(ast::generic::Compatible_System& n);

  // ============ TYPE ============
  llvm::Value* visit(ast::type::Ptr& n);
  llvm::Value* visit(ast::type::Table& n);
  llvm::Value* visit(ast::type::Primitive& n);
  llvm::Value* visit(ast::type::Tuple& n);
  llvm::Value* visit(ast::type::Function_Proto& n);

  llvm::Value* visit(ast::type::Get_Expr_Type& n);

  // ============ LITERAL ============
  llvm::Value* visit(ast::literal::Boolean& n);
  llvm::Value* visit(ast::literal::Integral& n);
  llvm::Value* visit(ast::literal::Decimal& n);
  llvm::Value* visit(ast::literal::Floating& n);

  llvm::Value* visit(ast::literal::ASCII& n);
  llvm::Value* visit(ast::literal::UTF32& n);

  llvm::Value* visit(ast::literal::Text& n);
  llvm::Value* visit(ast::literal::Text_Interpolation& n);
  llvm::Value* visit(ast::literal::Textual_Element& n);
  llvm::Value* visit(ast::literal::Textual_Format& n);
  llvm::Value* visit(ast::literal::Format_Specifier& n);

  llvm::Value* visit(ast::literal::Table& n);
  llvm::Value* visit(ast::literal::Table_Population& n);

  llvm::Value* visit(ast::literal::Map& n);

  llvm::Value* visit(ast::literal::Tuple& n);

  llvm::Value* visit(ast::literal::Range& n);
  llvm::Value* visit(ast::literal::Iterator& n);

  llvm::Value* visit(ast::literal::Enum& n);

  llvm::Value* visit(ast::literal::Component& n);
  llvm::Value* visit(ast::literal::Entity& n);

  // ============ Expression ============
  llvm::Value* visit(ast::expression::If_Ternary& n);

  llvm::Value* visit(ast::expression::Member_Access& n);

  llvm::Value* visit(ast::expression::Self& n);
  llvm::Value* visit(ast::expression::Other& n);

  llvm::Value* visit(ast::expression::Call& n);
  llvm::Value* visit(ast::expression::Call_Argument& n);
  llvm::Value* visit(ast::expression::Call_System& n);
  llvm::Value* visit(ast::expression::Call_Pipe& n);

  llvm::Value* visit(ast::expression::Table_Access& n);

  llvm::Value* visit(ast::expression::Ptr_At& n);
  llvm::Value* visit(ast::expression::Ptr_Offset& n);
  llvm::Value* visit(ast::expression::Ptr_Val& n);
  llvm::Value* visit(ast::expression::Addr_Of& n);
  llvm::Value* visit(ast::expression::Size_Of& n);
  llvm::Value* visit(ast::expression::GetBits& n);

  llvm::Value* visit(ast::expression::Move& n);
  llvm::Value* visit(ast::expression::New_Ptr& n);

  // ============ STATEMENT ============
  llvm::Value* visit(ast::statement::If& n);

  llvm::Value* visit(ast::statement::For& n);
  llvm::Value* visit(ast::statement::Loop& n);
  llvm::Value* visit(ast::statement::While& n);
  llvm::Value* visit(ast::statement::GoTo& n);
  llvm::Value* visit(ast::statement::GoTo_Label& n);

  llvm::Value* visit(ast::statement::Return& n);
  llvm::Value* visit(ast::statement::Break& n);
  llvm::Value* visit(ast::statement::Continue& n);

  llvm::Value* visit(ast::statement::Match& n);
  llvm::Value* visit(ast::statement::Match_Case& n);

  // ============ OPERATION ============
  llvm::Value* visit(ast::operation::Cast_As& n);
  llvm::Value* visit(ast::operation::Is& n);
  llvm::Value* visit(ast::operation::In& n);
  llvm::Value* visit(ast::operation::Assignment& n);
  llvm::Value* visit(ast::operation::Binary& n);
  llvm::Value* visit(ast::operation::Unary& n);
  llvm::Value* visit(ast::operation::Interval& n);
  llvm::Value* visit(ast::operation::Ptr_Dist& n);

  // ============ MEMORY ============
  llvm::Value* visit(ast::memory::Del& n);
  llvm::Value* visit(ast::memory::Align& n);
  llvm::Value* visit(ast::memory::Drop& n);
};
