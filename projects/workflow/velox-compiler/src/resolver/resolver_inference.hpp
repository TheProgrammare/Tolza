#pragma once


#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

#include "resolver_base.hpp"

namespace resolver
{


struct Inference : public Base {
  // keep parent constructor
  using Base::Base;

  ast::Node& get_node(ast::_id n) const;
  ast::Node& get_node(ast::_gnid n) const;

  ast::_gnid        make_gnid(ast::Node& n) const;
  bool              is_inferred(ast::Node& n) const;
  bool              is_inferred(ast::_gnid n) const;
  type::_id         get_type_id(ast::Node& n) const;
  type::_id         get_type_id(ast::_gnid n) const;
  const type::Type& get_type(ast::Node& n) const;
  const type::Type& get_type(ast::_gnid n) const;
  void              add_inference(ast::Node& n, type::_id type) const;

  void resolve_Node(ast::_gnid& n, bool mandatory = true);
  void resolve_Node(ast::Node& n, bool mandatory = true);

  bool start_resolver();

  void resolve_ID(ast::ID& n);
  void resolve_ID_Qualified(ast::ID_Qualified& n);
  void resolve_ID_Typed(ast::ID_Typed& n);
  void resolve_Global_Variable(ast::Global_Variable& n);
  void resolve_Global_Function(ast::Global_Function& n);
  void resolve_Local_Pattern_Enum(ast::Local_Pattern_Enum& n);
  void resolve_Local_Pattern_Tuple(ast::Local_Pattern_Tuple& n);
  void resolve_Local_Pattern_Entity(ast::Local_Pattern_Entity& n);
  void resolve_Local_Pattern_Sys_Comp(ast::Local_Pattern_Sys_Comp& n);
  void resolve_Local_Pattern_Comp(ast::Local_Pattern_Comp& n);
  void resolve_Local_Variable(ast::Local_Variable& n);
  void resolve_Local_Binding(ast::Local_Binding& n);
  void resolve_Statement_Return(ast::Statement_Return& n);
  void resolve_Statement_For(ast::Statement_For& n);
  void resolve_Expression_If_Ternary(ast::Expression_If_Ternary& n);
  void resolve_Expression_Member_Access(ast::Expression_Member_Access& n);
  void resolve_Expression_Self(ast::Expression_Self& n);
  void resolve_Expression_Other(ast::Expression_Other& n);
  void resolve_Expression_Call(ast::Expression_Call& n);
  void resolve_Expression_Call_Argument(ast::Expression_Call_Argument& n);
  void resolve_Expression_Call_System(ast::Expression_Call_System& n);
  void resolve_Expression_Call_Pipe(ast::Expression_Call_Pipe& n);
  void resolve_Expression_Table_Access(ast::Expression_Table_Access& n);
  void resolve_Expression_Ptr_Val(ast::Expression_Ptr_Val& n);
  void resolve_Expression_Mut_Of(ast::Expression_Mut_Of& n);
  void resolve_Expression_Ref_Of(ast::Expression_Ref_Of& n);
  void resolve_Expression_Move_Of(ast::Expression_Move_Of& n);
  void resolve_Expression_Copy_Of(ast::Expression_Copy_Of& n);
  void resolve_Expression_Addr_Of(ast::Expression_Addr_Of& n);
  void resolve_Expression_Size_Of(ast::Expression_Size_Of& n);
  void resolve_Expression_GetBits(ast::Expression_GetBits& n);
  void resolve_Expression_New_Ptr(ast::Expression_New_Ptr& n);
  void resolve_Expression_Get_Type(ast::Expression_Get_Type& n);
  void resolve_Literal_Boolean(ast::Literal_Boolean& n);
  void resolve_Literal_Integral(ast::Literal_Integral& n);
  void resolve_Literal_Fixed_Point(ast::Literal_Fixed_Point& n);
  void resolve_Literal_Floating_Point(ast::Literal_Floating_Point& n);
  void resolve_Literal_Cune(ast::Literal_Cune& n);
  void resolve_Literal_Rune(ast::Literal_Rune& n);
  void resolve_Literal_Text_Pure(ast::Literal_Text_Pure& n);
  void resolve_Literal_Text_Interpolation(ast::Literal_Text_Interpolation& n);
  void resolve_Literal_Textual_Format(ast::Literal_Textual_Format& n);
  void resolve_Literal_Table(ast::Literal_Table& n);
  void resolve_Literal_Map(ast::Literal_Map& n);
  void resolve_Literal_Tuple(ast::Literal_Tuple& n);
  void resolve_Literal_Range(ast::Literal_Range& n);
  void resolve_Literal_Iterator(ast::Literal_Iterator& n);
  void resolve_Literal_Enum(ast::Literal_Enum& n);
  void resolve_Literal_Structured_Data(ast::Literal_Structured_Data& n);
  void resolve_Literal_Entity(ast::Literal_Entity& n);
  void resolve_Operation_Cast_As(ast::Operation_Cast_As& n);
  void resolve_Operation_Is(ast::Operation_Is& n);
  void resolve_Operation_In(ast::Operation_In& n);
  void resolve_Operation_Assignment(ast::Operation_Assignment& n);
  void resolve_Operation_Binary(ast::Operation_Binary& n);
  void resolve_Operation_Unary(ast::Operation_Unary& n);
  void resolve_Operation_Interval(ast::Operation_Interval& n);


  type::_id resolve_type(ast::Node& n, type::Type& input_type, bool p_is_silent_error = false);

  bool is_lazy_literal(ast::Node& n) const;

  void ensure_expression_resolution(ast::Node& p_expr, type::_id p_type_inferrance);
};


} // namespace resolver