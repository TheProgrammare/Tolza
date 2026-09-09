#pragma once


#include "ast/forward.hpp"
#include "id/nodeid.hpp"

#include <unordered_set>
namespace resolver
{

struct Evaluable final {
  std::unordered_set<ast::ID, ast::ID::Hash> evaluated;

  [[nodiscard]] bool is_evaluable(ast::ID nodeid) noexcept;

  [[nodiscard]] bool add_evaluation(ast::ID nodeid, bool is_evaluable) noexcept;

  [[nodiscard]] bool is_evaluable_Symbol_Id(const ast::Symbol_Id& n) noexcept;
  [[nodiscard]] bool is_evaluable_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept;
  [[nodiscard]] bool is_evaluable_Symbol_Type(const ast::Symbol_Type& n) noexcept;
  [[nodiscard]] bool is_evaluable_Path_Regex(const ast::Path_Regex& n) noexcept;
  [[nodiscard]] bool is_evaluable_Root(const ast::Root& n) noexcept;
  [[nodiscard]] bool is_evaluable_Import(const ast::Import& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Variable(const ast::Global_Variable& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Function(const ast::Global_Function& n) noexcept;
  [[nodiscard]] bool is_evaluable_Call_Contract(const ast::Call_Contract& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Module(const ast::Global_Module& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Extern(const ast::Global_Extern& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Export(const ast::Global_Export& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Reexport(const ast::Global_Reexport& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Enum(const ast::Global_Enum& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Flag(const ast::Global_Flag& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Union(const ast::Global_Union& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept;
  [[nodiscard]] bool is_evaluable_Global_Generic(const ast::Global_Generic& n) noexcept;
  [[nodiscard]] bool is_evaluable_Enum_Field(const ast::Enum_Field& n) noexcept;
  [[nodiscard]] bool is_evaluable_Flag_Field(const ast::Flag_Field& n) noexcept;
  [[nodiscard]] bool is_evaluable_Union_Field(const ast::Union_Field& n) noexcept;
  [[nodiscard]] bool is_evaluable_CodeBlock(const ast::CodeBlock& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Lambda(const ast::Local_Lambda& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Parameter(const ast::Local_Parameter& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Element(const ast::Local_Pattern_Element& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Form(const ast::Local_Pattern_Form& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Binding(const ast::Local_Binding& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Variable(const ast::Local_Variable& n) noexcept;
  [[nodiscard]] bool is_evaluable_Local_Capability(const ast::Local_Capability& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_Facet(const ast::SFM_Facet& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_View(const ast::SFM_View& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_Form(const ast::SFM_Form& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_Rule(const ast::SFM_Rule& n) noexcept;
  [[nodiscard]] bool is_evaluable_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Type(const ast::Generic_Type& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Cast(const ast::Generic_Cast& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Op(const ast::Generic_Op& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_View(const ast::Generic_View& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Facet(const ast::Generic_Facet& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Extension(const ast::Generic_Extension& n) noexcept;
  [[nodiscard]] bool is_evaluable_Generic_Rule(const ast::Generic_Rule& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Boolean(const ast::Literal_Boolean& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Integral(const ast::Literal_Integral& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Cune(const ast::Literal_Cune& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Rune(const ast::Literal_Rune& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Table(const ast::Literal_Table& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Tuple(const ast::Literal_Tuple& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Range(const ast::Literal_Range& n) noexcept;
  [[nodiscard]] bool is_evaluable_Literal_Record(const ast::Literal_Record& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Self(const ast::Expression_Self& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Other(const ast::Expression_Other& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Invocation(const ast::Expression_Invocation& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept;
  [[nodiscard]] bool is_evaluable_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_If(const ast::Statement_If& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_For(const ast::Statement_For& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Loop(const ast::Statement_Loop& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_While(const ast::Statement_While& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_GoTo(const ast::Statement_GoTo& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Return(const ast::Statement_Return& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Break(const ast::Statement_Break& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Continue(const ast::Statement_Continue& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Match(const ast::Statement_Match& n) noexcept;
  [[nodiscard]] bool is_evaluable_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Is(const ast::Operation_Is& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_In(const ast::Operation_In& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Transfert(const ast::Operation_Transfert& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Binary(const ast::Operation_Binary& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Unary(const ast::Operation_Unary& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Interval(const ast::Operation_Interval& n) noexcept;
  [[nodiscard]] bool is_evaluable_Operation_Mem(const ast::Operation_Mem& n) noexcept;
};

} // namespace resolver