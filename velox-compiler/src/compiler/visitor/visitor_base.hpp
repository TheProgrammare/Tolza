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

#include "compiler/ast/ast_forward.hpp"

struct ScriptInfo;

struct Visitor_Base {
  virtual ~Visitor_Base() = default;
  Visitor_Base()          = delete;
  Visitor_Base(ScriptInfo& _scr_info)
    : scr_info(_scr_info)
  {
  }

  ScriptInfo& scr_info;

  std::vector<std::string> errors;

  // ============ AST ============
  virtual void visit(ast::Node& n) = 0;

  virtual void visit(ast::AType& n)             = 0;
  virtual void visit(ast::ALiteral& n)          = 0;
  virtual void visit(ast::ADeclaration& n)      = 0;
  virtual void visit(ast::ALocal& n)            = 0;
  virtual void visit(ast::AExpression& n)       = 0;
  virtual void visit(ast::Expr_ID& n)           = 0;
  virtual void visit(ast::Expr_ID_Qualified& n) = 0;
  virtual void visit(ast::Expr_ID_Generic& n)   = 0;

  virtual void visit(ast::Root& n) = 0;

  // ============ DECLARATION ============
  virtual void visit(ast::declaration::Global& n)   = 0;
  virtual void visit(ast::declaration::Function& n) = 0;

  virtual void visit(ast::declaration::Mod& n)    = 0;
  virtual void visit(ast::declaration::Export& n) = 0;

  virtual void visit(ast::declaration::Enum& n)         = 0;
  virtual void visit(ast::declaration::Enum_Element& n) = 0;

  virtual void visit(ast::declaration::Flag& n) = 0;

  virtual void visit(ast::declaration::Mod_Alias& n)  = 0;
  virtual void visit(ast::declaration::Type_Alias& n) = 0;

  virtual void visit(ast::declaration::Generic& n) = 0;

  // ============ LOCAL ============
  virtual void visit(ast::declaration::local::CodeBlock& n) = 0;

  virtual void visit(ast::declaration::local::Lambda& n)         = 0;
  virtual void visit(ast::declaration::local::Lambda_Capture& n) = 0;
  virtual void visit(ast::declaration::local::Capture_Member& n) = 0;

  virtual void visit(ast::declaration::local::Parameter& n)         = 0;
  virtual void visit(ast::declaration::local::Generic_Parameter& n) = 0;

  virtual void visit(ast::declaration::local::Pattern& n)           = 0;
  virtual void visit(ast::declaration::local::Pattern_Enum& n)      = 0;
  virtual void visit(ast::declaration::local::Pattern_Tuple& n)     = 0;
  virtual void visit(ast::declaration::local::Pattern_Entity& n)    = 0;
  virtual void visit(ast::declaration::local::Pattern_Component& n) = 0;

  virtual void visit(ast::declaration::local::Variable_Binding& n) = 0;
  virtual void visit(ast::declaration::local::Variable_Unpack& n)  = 0;
  virtual void visit(ast::declaration::local::Variable& n)         = 0;

  virtual void visit(ast::declaration::local::Capability& n) = 0;

  // ============ COP ============
  virtual void visit(ast::declaration::cop::Component& n)       = 0;
  virtual void visit(ast::declaration::cop::Component_Field& n) = 0;

  virtual void visit(ast::declaration::cop::Role& n) = 0;

  virtual void visit(ast::declaration::cop::Entity& n)         = 0;
  virtual void visit(ast::declaration::cop::Entity_Cast& n)    = 0;
  virtual void visit(ast::declaration::cop::Entity_Op& n)      = 0;
  virtual void visit(ast::declaration::cop::Entity_OpIndex& n) = 0;

  virtual void visit(ast::declaration::cop::System& n)      = 0;
  virtual void visit(ast::declaration::cop::System_Case& n) = 0;

  // ============ GENERIC ============
  virtual void visit(ast::generic::Is_Type& n)           = 0;
  virtual void visit(ast::generic::Can_Cast& n)          = 0;
  virtual void visit(ast::generic::Have_Op& n)           = 0;
  virtual void visit(ast::generic::Have_Role& n)         = 0;
  virtual void visit(ast::generic::Use_Component& n)     = 0;
  virtual void visit(ast::generic::Compatible_System& n) = 0;

  // ============ TYPE ============
  virtual void visit(ast::type::Ptr& n)            = 0;
  virtual void visit(ast::type::Table& n)          = 0;
  virtual void visit(ast::type::Primitive& n)      = 0;
  virtual void visit(ast::type::Tuple& n)          = 0;
  virtual void visit(ast::type::Function_Proto& n) = 0;

  virtual void visit(ast::type::Get_Expr_Type& n) = 0;

  // ============ LITERAL ============
  virtual void visit(ast::literal::Boolean& n)  = 0;
  virtual void visit(ast::literal::Integral& n) = 0;
  virtual void visit(ast::literal::Decimal& n)  = 0;
  virtual void visit(ast::literal::Floating& n) = 0;

  virtual void visit(ast::literal::ASCII& n) = 0;
  virtual void visit(ast::literal::UFT32& n) = 0;

  virtual void visit(ast::literal::Text& n)             = 0;
  virtual void visit(ast::literal::Text_Lerp& n)        = 0;
  virtual void visit(ast::literal::Textual_Element& n)  = 0;
  virtual void visit(ast::literal::Textual_Format& n)   = 0;
  virtual void visit(ast::literal::Format_Specifier& n) = 0;

  virtual void visit(ast::literal::Table& n)            = 0;
  virtual void visit(ast::literal::Table_Population& n) = 0;

  virtual void visit(ast::literal::Map& n) = 0;

  virtual void visit(ast::literal::Tuple& n) = 0;

  virtual void visit(ast::literal::Range& n) = 0;

  virtual void visit(ast::literal::Component& n) = 0;
  virtual void visit(ast::literal::Entity& n)    = 0;

  // ============ Expression ============
  virtual void visit(ast::expression::If_Ternary& n) = 0;
  virtual void visit(ast::expression::Enum& n)       = 0;

  virtual void visit(ast::expression::Member_Access& n) = 0;

  virtual void visit(ast::expression::Self& n)  = 0;
  virtual void visit(ast::expression::Other& n) = 0;

  virtual void visit(ast::expression::Call& n)          = 0;
  virtual void visit(ast::expression::Call_Argument& n) = 0;
  virtual void visit(ast::expression::Call_System& n)   = 0;
  virtual void visit(ast::expression::Call_Pipe& n)     = 0;

  virtual void visit(ast::expression::Table_Access& n) = 0;

  virtual void visit(ast::expression::Ptr_At& n)     = 0;
  virtual void visit(ast::expression::Ptr_Offset& n) = 0;
  virtual void visit(ast::expression::Ptr_Val& n)    = 0;
  virtual void visit(ast::expression::Addr_Of& n)    = 0;
  virtual void visit(ast::expression::Size_Of& n)    = 0;
  virtual void visit(ast::expression::GetBits& n)    = 0;

  virtual void visit(ast::expression::Move& n)    = 0;
  virtual void visit(ast::expression::New_Ptr& n) = 0;

  // ============ STATEMENT ============
  virtual void visit(ast::statement::If& n) = 0;

  virtual void visit(ast::statement::For& n)        = 0;
  virtual void visit(ast::statement::Loop& n)       = 0;
  virtual void visit(ast::statement::While& n)      = 0;
  virtual void visit(ast::statement::GoTo& n)       = 0;
  virtual void visit(ast::statement::GoTo_Label& n) = 0;

  virtual void visit(ast::statement::Return& n)   = 0;
  virtual void visit(ast::statement::Break& n)    = 0;
  virtual void visit(ast::statement::Continue& n) = 0;

  virtual void visit(ast::statement::Match& n)      = 0;
  virtual void visit(ast::statement::Match_Case& n) = 0;

  // ============ OPERATION ============
  virtual void visit(ast::operation::Cast_As& n)    = 0;
  virtual void visit(ast::operation::Is& n)         = 0;
  virtual void visit(ast::operation::In& n)         = 0;
  virtual void visit(ast::operation::Assignment& n) = 0;
  virtual void visit(ast::operation::Binary& n)     = 0;
  virtual void visit(ast::operation::Unary& n)      = 0;
  virtual void visit(ast::operation::Interval& n)   = 0;
  virtual void visit(ast::operation::Ptr_Dist& n)   = 0;

  // ============ MEMORY ============
  virtual void visit(ast::memory::Del& n)   = 0;
  virtual void visit(ast::memory::Align& n) = 0;
  virtual void visit(ast::memory::Drop& n)  = 0;
};