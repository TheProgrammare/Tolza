#pragma once


#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

#include "nexus/ids.hpp"
#include "resolver_base.hpp"
#include <bits/types/clockid_t.h>
#include <cstddef>
#include <unordered_map>

namespace resolver
{


struct Inference : public Base {
  // keep parent constructor
  using Base::Base;

  size_t inference_count = 0;

  void add_inference(const ast::Node& n, type::ID type);

  void resolve_node(ast::ID n, bool mandatory = true);
  void resolve_node(const ast::Node& n, bool mandatory = true);

  [[nodiscard]] size_t start_resolver();

  void resolve_Root(const ast::Root& n);
  void resolve_CodeBlock(const ast::CodeBlock& n);

  void resolve_Symbol_Id(const ast::Symbol_Id& n);
  void resolve_Symbol_Qualified(const ast::Symbol_Qualified& n);
  void resolve_Symbol_Type(const ast::Symbol_Type& n);
  void resolve_Global_Variable(const ast::Global_Variable& n);
  void resolve_Global_Function(const ast::Global_Function& n);
  void resolve_Global_Alias_Type(const ast::Global_Alias_Type& n);
  void resolve_Global_Export(const ast::Global_Export& n);
  void resolve_Global_Extern(const ast::Global_Extern& n);
  void resolve_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n);
  void resolve_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n);
  void resolve_Local_Pattern_Form(const ast::Local_Pattern_Form& n);
  void resolve_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n);
  void resolve_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n);
  void resolve_Local_Variable(const ast::Local_Variable& n);
  void resolve_Local_Capability(const ast::Local_Capability& n);
  void resolve_Local_Parameter(const ast::Local_Parameter& n);
  void resolve_Local_Binding(const ast::Local_Binding& n);
  void resolve_Expression_If_Ternary(const ast::Expression_If_Ternary& n);
  void resolve_Expression_Member_Access(const ast::Expression_Member_Access& n);
  void resolve_Expression_Self(const ast::Expression_Self& n);
  void resolve_Expression_Other(const ast::Expression_Other& n);
  void resolve_Expression_Invocation(const ast::Expression_Invocation& n);
  void resolve_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n);
  void resolve_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n);
  void resolve_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n);
  void resolve_Expression_Table_Access(const ast::Expression_Table_Access& n);
  void resolve_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n);
  void resolve_Expression_Mut_Of(const ast::Expression_Mut_Of& n);
  void resolve_Expression_Ref_Of(const ast::Expression_Ref_Of& n);
  void resolve_Expression_Move_Of(const ast::Expression_Move_Of& n);
  void resolve_Expression_Copy_Of(const ast::Expression_Copy_Of& n);
  void resolve_Expression_Addr_Of(const ast::Expression_Addr_Of& n);
  void resolve_Expression_Size_Of(const ast::Expression_Size_Of& n);
  void resolve_Expression_GetBits(const ast::Expression_GetBits& n);
  void resolve_Expression_New_Ptr(const ast::Expression_New_Ptr& n);
  void resolve_Expression_Get_Type(const ast::Expression_Get_Type& n);
  void resolve_Statement_If(const ast::Statement_If& n);
  void resolve_Statement_For(const ast::Statement_For& n);
  void resolve_Statement_Loop(const ast::Statement_Loop& n);
  void resolve_Statement_While(const ast::Statement_While& n);
  void resolve_Statement_GoTo(const ast::Statement_GoTo& n);
  void resolve_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n);
  void resolve_Statement_Return(const ast::Statement_Return& n);
  void resolve_Statement_Break(const ast::Statement_Break& n);
  void resolve_Statement_Continue(const ast::Statement_Continue& n);
  void resolve_Statement_Match(const ast::Statement_Match& n);
  void resolve_Statement_Match_Case(const ast::Statement_Match_Case& n);
  void resolve_Literal_Boolean(const ast::Literal_Boolean& n);
  void resolve_Literal_NullPtr(const ast::Literal_NullPtr& n);
  void resolve_Literal_Integral(ast::Literal_Integral& n);
  void resolve_Literal_Fixed_Point(ast::Literal_Fixed_Point& n);
  void resolve_Literal_Floating_Point(ast::Literal_Floating_Point& n);
  void resolve_Literal_Cune(const ast::Literal_Cune& n);
  void resolve_Literal_Rune(const ast::Literal_Rune& n);
  void resolve_Literal_Text_Pure(const ast::Literal_Text_Pure& n);
  void resolve_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n);
  void resolve_Literal_Textual_Format(const ast::Literal_Textual_Format& n);
  void resolve_Literal_Table(const ast::Literal_Table& n);
  void resolve_Literal_Map(const ast::Literal_Map& n);
  void resolve_Literal_Tuple(const ast::Literal_Tuple& n);
  void resolve_Literal_Range(const ast::Literal_Range& n);
  void resolve_Literal_Record(const ast::Literal_Record& n);
  void resolve_Operation_Cast_As(const ast::Operation_Cast_As& n);
  void resolve_Operation_Is(const ast::Operation_Is& n);
  void resolve_Operation_In(const ast::Operation_In& n);
  void resolve_Operation_Transfert(const ast::Operation_Transfert& n);
  void resolve_Operation_Binary(const ast::Operation_Binary& n);
  void resolve_Operation_Unary(const ast::Operation_Unary& n);
  void resolve_Operation_Interval(const ast::Operation_Interval& n);


  [[nodiscard]] bool is_lazy_literal(const ast::Node& n) const;

  void ensure_primitive_literal(ast::ID lit, type::ID ty_inference);

  void ensure_expression_resolution(const ast::Node& expr, type::ID type_inferrance, bool silent_error = false);
};


} // namespace resolver