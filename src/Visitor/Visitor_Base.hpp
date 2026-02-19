/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <string>
#include <vector>

#include "AST/AST_Forward.hpp"

struct ScriptInfo;

struct Visitor_Base {
  virtual ~Visitor_Base() = default;
  Visitor_Base()          = delete;
  Visitor_Base(ScriptInfo& _scr_info) : scr_info(_scr_info) {}

  ScriptInfo& scr_info;

  std::vector<std::string> errors;

  // ============ AST ============
  virtual void visit(AST::Node& n) = 0;

  virtual void visit(AST::AType& n)             = 0;
  virtual void visit(AST::ALiteral& n)          = 0;
  virtual void visit(AST::ADeclaration& n)      = 0;
  virtual void visit(AST::ALocal& n)            = 0;
  virtual void visit(AST::AExpression& n)       = 0;
  virtual void visit(AST::Expr_ID& n)           = 0;
  virtual void visit(AST::Expr_ID_Qualified& n) = 0;
  virtual void visit(AST::Expr_ID_Generic& n)   = 0;

  virtual void visit(AST::Root& n) = 0;

  // ============ DECLARATION ============
  virtual void visit(AST::Declaration::Global& n)   = 0;
  virtual void visit(AST::Declaration::Function& n) = 0;

  virtual void visit(AST::Declaration::Mod& n)    = 0;
  virtual void visit(AST::Declaration::Export& n) = 0;

  virtual void visit(AST::Declaration::Enum& n)         = 0;
  virtual void visit(AST::Declaration::Enum_Element& n) = 0;

  virtual void visit(AST::Declaration::Flag& n) = 0;

  virtual void visit(AST::Declaration::Type_Alias& n) = 0;

  virtual void visit(AST::Declaration::Generic& n) = 0;

  // ============ LOCAL ============
  virtual void visit(AST::Declaration::Local::CodeBlock& n) = 0;

  virtual void visit(AST::Declaration::Local::Lambda& n)         = 0;
  virtual void visit(AST::Declaration::Local::Lambda_Capture& n) = 0;
  virtual void visit(AST::Declaration::Local::Capture_Member& n) = 0;

  virtual void visit(AST::Declaration::Local::Parameter& n)         = 0;
  virtual void visit(AST::Declaration::Local::Generic_Parameter& n) = 0;

  virtual void visit(AST::Declaration::Local::Pattern& n)           = 0;
  virtual void visit(AST::Declaration::Local::Pattern_Enum& n)      = 0;
  virtual void visit(AST::Declaration::Local::Pattern_Tuple& n)     = 0;
  virtual void visit(AST::Declaration::Local::Pattern_Entity& n)    = 0;
  virtual void visit(AST::Declaration::Local::Pattern_Component& n) = 0;

  virtual void visit(AST::Declaration::Local::Variable_Binding& n) = 0;
  virtual void visit(AST::Declaration::Local::Variable_Unpack& n)  = 0;
  virtual void visit(AST::Declaration::Local::Variable& n)         = 0;

  virtual void visit(AST::Declaration::Local::Capability& n) = 0;

  // ============ COP ============
  virtual void visit(AST::Declaration::COP::Component& n)       = 0;
  virtual void visit(AST::Declaration::COP::Component_Field& n) = 0;

  virtual void visit(AST::Declaration::COP::Role& n) = 0;

  virtual void visit(AST::Declaration::COP::Entity& n)         = 0;
  virtual void visit(AST::Declaration::COP::Entity_Cast& n)    = 0;
  virtual void visit(AST::Declaration::COP::Entity_Op& n)      = 0;
  virtual void visit(AST::Declaration::COP::Entity_OpIndex& n) = 0;

  virtual void visit(AST::Declaration::COP::System& n)      = 0;
  virtual void visit(AST::Declaration::COP::System_Case& n) = 0;

  // ============ GENERIC ============
  virtual void visit(AST::Generic::Is_Type& n)           = 0;
  virtual void visit(AST::Generic::Can_Cast& n)          = 0;
  virtual void visit(AST::Generic::Have_Op& n)           = 0;
  virtual void visit(AST::Generic::Have_Role& n)         = 0;
  virtual void visit(AST::Generic::Use_Component& n)     = 0;
  virtual void visit(AST::Generic::Compatible_System& n) = 0;

  // ============ TYPE ============
  virtual void visit(AST::Type::Ptr& n)            = 0;
  virtual void visit(AST::Type::Table& n)          = 0;
  virtual void visit(AST::Type::Primitive& n)      = 0;
  virtual void visit(AST::Type::Tuple& n)          = 0;
  virtual void visit(AST::Type::Function_Proto& n) = 0;

  virtual void visit(AST::Type::Get_Expr_Type& n) = 0;

  // ============ LITERAL ============
  virtual void visit(AST::Literal::Boolean& n)  = 0;
  virtual void visit(AST::Literal::Integral& n) = 0;
  virtual void visit(AST::Literal::Decimal& n)  = 0;
  virtual void visit(AST::Literal::Floating& n) = 0;

  virtual void visit(AST::Literal::ASCII& n) = 0;
  virtual void visit(AST::Literal::UFT32& n) = 0;

  virtual void visit(AST::Literal::Text& n)             = 0;
  virtual void visit(AST::Literal::Text_Lerp& n)        = 0;
  virtual void visit(AST::Literal::Textual_Element& n)  = 0;
  virtual void visit(AST::Literal::Textual_Format& n)   = 0;
  virtual void visit(AST::Literal::Format_Specifier& n) = 0;

  virtual void visit(AST::Literal::Table& n)            = 0;
  virtual void visit(AST::Literal::Table_Population& n) = 0;

  virtual void visit(AST::Literal::Map& n) = 0;

  virtual void visit(AST::Literal::Tuple& n) = 0;

  virtual void visit(AST::Literal::Range& n) = 0;

  virtual void visit(AST::Literal::Component& n) = 0;
  virtual void visit(AST::Literal::Entity& n)    = 0;

  // ============ Expression ============
  virtual void visit(AST::Expression::If_Ternary& n) = 0;
  virtual void visit(AST::Expression::Enum& n)       = 0;

  virtual void visit(AST::Expression::Member_Access& n) = 0;

  virtual void visit(AST::Expression::Self& n)  = 0;
  virtual void visit(AST::Expression::Other& n) = 0;

  virtual void visit(AST::Expression::Call& n)          = 0;
  virtual void visit(AST::Expression::Call_Argument& n) = 0;
  virtual void visit(AST::Expression::Call_System& n)   = 0;
  virtual void visit(AST::Expression::Call_Pipe& n)     = 0;

  virtual void visit(AST::Expression::Table_Access& n) = 0;

  virtual void visit(AST::Expression::Ptr_At& n)     = 0;
  virtual void visit(AST::Expression::Ptr_Offset& n) = 0;
  virtual void visit(AST::Expression::Ptr_Val& n)    = 0;
  virtual void visit(AST::Expression::Addr_Of& n)    = 0;
  virtual void visit(AST::Expression::Size_Of& n)    = 0;
  virtual void visit(AST::Expression::GetBits& n)    = 0;

  virtual void visit(AST::Expression::Move& n)    = 0;
  virtual void visit(AST::Expression::New_Ptr& n) = 0;

  // ============ STATEMENT ============
  virtual void visit(AST::Statement::If& n) = 0;

  virtual void visit(AST::Statement::For& n)        = 0;
  virtual void visit(AST::Statement::Loop& n)       = 0;
  virtual void visit(AST::Statement::While& n)      = 0;
  virtual void visit(AST::Statement::GoTo& n)       = 0;
  virtual void visit(AST::Statement::GoTo_Label& n) = 0;

  virtual void visit(AST::Statement::Return& n)   = 0;
  virtual void visit(AST::Statement::Break& n)    = 0;
  virtual void visit(AST::Statement::Continue& n) = 0;

  virtual void visit(AST::Statement::Match& n)      = 0;
  virtual void visit(AST::Statement::Match_Case& n) = 0;

  // ============ OPERATION ============
  virtual void visit(AST::Operation::Cast_As& n)    = 0;
  virtual void visit(AST::Operation::Is& n)         = 0;
  virtual void visit(AST::Operation::In& n)         = 0;
  virtual void visit(AST::Operation::Assignment& n) = 0;
  virtual void visit(AST::Operation::Binary& n)     = 0;
  virtual void visit(AST::Operation::Unary& n)      = 0;
  virtual void visit(AST::Operation::Interval& n)   = 0;
  virtual void visit(AST::Operation::Ptr_Dist& n)   = 0;

  // ============ MEMORY ============
  virtual void visit(AST::Memory::Del& n)   = 0;
  virtual void visit(AST::Memory::Align& n) = 0;
  virtual void visit(AST::Memory::Drop& n)  = 0;
};