#pragma once


#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

#include "nexus/ids.hpp"
#include "resolver_base.hpp"
#include <bits/types/clockid_t.h>
#include <cstddef>

namespace utils
{


struct Dump : public resolver::Base {
  // keep parent constructor
  using Base::Base;

  static std::string dump_node(ast::ID nodeid) noexcept;

  static std::string dump_Symbol_Id(const ast::Symbol_Id& n) noexcept;
  static std::string dump_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept;
  static std::string dump_Symbol_Type(const ast::Symbol_Type& n) noexcept;
  static std::string dump_Path_Regex(const ast::Path_Regex& n) noexcept;
  static std::string dump_Root(const ast::Root& n) noexcept;
  static std::string dump_Import(const ast::Import& n) noexcept;
  static std::string dump_Global_Variable(const ast::Global_Variable& n) noexcept;
  static std::string dump_Global_Function(const ast::Global_Function& n) noexcept;
  static std::string dump_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept;
  static std::string dump_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept;
  static std::string dump_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept;
  static std::string dump_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept;
  static std::string dump_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept;
  static std::string dump_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept;
  static std::string dump_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept;
  static std::string dump_Global_Module(const ast::Global_Module& n) noexcept;
  static std::string dump_Global_Extern(const ast::Global_Extern& n) noexcept;
  static std::string dump_Global_Export(const ast::Global_Export& n) noexcept;
  static std::string dump_Global_Reexport(const ast::Global_Reexport& n) noexcept;
  static std::string dump_Global_Enum(const ast::Global_Enum& n) noexcept;
  static std::string dump_Global_Flag(const ast::Global_Flag& n) noexcept;
  static std::string dump_Global_Union(const ast::Global_Union& n) noexcept;
  static std::string dump_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept;
  static std::string dump_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept;
  static std::string dump_Global_Generic(const ast::Global_Generic& n) noexcept;
  static std::string dump_Enum_Field(const ast::Enum_Field& n) noexcept;
  static std::string dump_Flag_Field(const ast::Flag_Field& n) noexcept;
  static std::string dump_Union_Field(const ast::Union_Field& n) noexcept;
  static std::string dump_CodeBlock(const ast::CodeBlock& n) noexcept;
  static std::string dump_Local_Lambda(const ast::Local_Lambda& n) noexcept;
  static std::string dump_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n) noexcept;
  static std::string dump_Local_Parameter(const ast::Local_Parameter& n) noexcept;
  static std::string dump_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept;
  static std::string dump_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept;
  static std::string dump_Local_Pattern_Element(const ast::Local_Pattern_Element& n) noexcept;
  static std::string dump_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n) noexcept;
  static std::string dump_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n) noexcept;
  static std::string dump_Local_Pattern_Form(const ast::Local_Pattern_Form& n) noexcept;
  static std::string dump_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept;
  static std::string dump_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n) noexcept;
  static std::string dump_Local_Binding(const ast::Local_Binding& n) noexcept;
  static std::string dump_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept;
  static std::string dump_Local_Variable(const ast::Local_Variable& n) noexcept;
  static std::string dump_Local_Capability(const ast::Local_Capability& n) noexcept;
  static std::string dump_SFM_Facet(const ast::SFM_Facet& n) noexcept;
  static std::string dump_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept;
  static std::string dump_SFM_View(const ast::SFM_View& n) noexcept;
  static std::string dump_SFM_Form(const ast::SFM_Form& n) noexcept;
  static std::string dump_SFM_Rule(const ast::SFM_Rule& n) noexcept;
  static std::string dump_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept;
  static std::string dump_Generic_Type(const ast::Generic_Type& n) noexcept;
  static std::string dump_Generic_Cast(const ast::Generic_Cast& n) noexcept;
  static std::string dump_Generic_Op(const ast::Generic_Op& n) noexcept;
  static std::string dump_Generic_View(const ast::Generic_View& n) noexcept;
  static std::string dump_Generic_Facet(const ast::Generic_Facet& n) noexcept;
  static std::string dump_Generic_Extension(const ast::Generic_Extension& n) noexcept;
  static std::string dump_Generic_Rule(const ast::Generic_Rule& n) noexcept;
  static std::string dump_Literal_Boolean(const ast::Literal_Boolean& n) noexcept;
  static std::string dump_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept;
  static std::string dump_Literal_Integral(const ast::Literal_Integral& n) noexcept;
  static std::string dump_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept;
  static std::string dump_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept;
  static std::string dump_Literal_Cune(const ast::Literal_Cune& n) noexcept;
  static std::string dump_Literal_Rune(const ast::Literal_Rune& n) noexcept;
  static std::string dump_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept;
  static std::string dump_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept;
  static std::string dump_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept;
  static std::string dump_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept;
  static std::string dump_Literal_Table(const ast::Literal_Table& n) noexcept;
  static std::string dump_Literal_Table_Population(const ast::Literal_Table_Population& n) noexcept;
  static std::string dump_Literal_Map(const ast::Literal_Map& n) noexcept;
  static std::string dump_Literal_Tuple(const ast::Literal_Tuple& n) noexcept;
  static std::string dump_Literal_Range(const ast::Literal_Range& n) noexcept;
  static std::string dump_Literal_Record(const ast::Literal_Record& n) noexcept;
  static std::string dump_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept;
  static std::string dump_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept;
  static std::string dump_Expression_Self(const ast::Expression_Self& n) noexcept;
  static std::string dump_Expression_Other(const ast::Expression_Other& n) noexcept;
  static std::string dump_Expression_Invocation(const ast::Expression_Invocation& n) noexcept;
  static std::string dump_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept;
  static std::string dump_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept;
  static std::string dump_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept;
  static std::string dump_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept;
  static std::string dump_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n) noexcept;
  static std::string dump_Expression_Mut_Of(const ast::Expression_Mut_Of& n) noexcept;
  static std::string dump_Expression_Ref_Of(const ast::Expression_Ref_Of& n) noexcept;
  static std::string dump_Expression_Move_Of(const ast::Expression_Move_Of& n) noexcept;
  static std::string dump_Expression_Copy_Of(const ast::Expression_Copy_Of& n) noexcept;
  static std::string dump_Expression_Addr_Of(const ast::Expression_Addr_Of& n) noexcept;
  static std::string dump_Expression_Size_Of(const ast::Expression_Size_Of& n) noexcept;
  static std::string dump_Expression_GetBits(const ast::Expression_GetBits& n) noexcept;
  static std::string dump_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept;
  static std::string dump_Expression_Get_Type(const ast::Expression_Get_Type& n) noexcept;
  static std::string dump_Statement_If(const ast::Statement_If& n) noexcept;
  static std::string dump_Statement_For(const ast::Statement_For& n) noexcept;
  static std::string dump_Statement_Loop(const ast::Statement_Loop& n) noexcept;
  static std::string dump_Statement_While(const ast::Statement_While& n) noexcept;
  static std::string dump_Statement_GoTo(const ast::Statement_GoTo& n) noexcept;
  static std::string dump_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept;
  static std::string dump_Statement_Return(const ast::Statement_Return& n) noexcept;
  static std::string dump_Statement_Break(const ast::Statement_Break& n) noexcept;
  static std::string dump_Statement_Continue(const ast::Statement_Continue& n) noexcept;
  static std::string dump_Statement_Match(const ast::Statement_Match& n) noexcept;
  static std::string dump_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept;
  static std::string dump_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept;
  static std::string dump_Operation_Is(const ast::Operation_Is& n) noexcept;
  static std::string dump_Operation_In(const ast::Operation_In& n) noexcept;
  static std::string dump_Operation_Transfert(const ast::Operation_Transfert& n) noexcept;
  static std::string dump_Operation_Binary(const ast::Operation_Binary& n) noexcept;
  static std::string dump_Operation_Unary(const ast::Operation_Unary& n) noexcept;
  static std::string dump_Operation_Interval(const ast::Operation_Interval& n) noexcept;
  static std::string dump_Memory_Del(const ast::Memory_Del& n) noexcept;
  static std::string dump_Memory_Align(const ast::Memory_Align& n) noexcept;
  static std::string dump_Memory_Drop(const ast::Memory_Drop& n) noexcept;
};

} // namespace utils