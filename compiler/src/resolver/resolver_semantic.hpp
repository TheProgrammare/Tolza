
#pragma once

#include "ast/forward.hpp"
#include "resolver_base.hpp"

namespace resolver
{


constexpr std::string_view ERR_TY_NOT_FOUND = "Type not found!";
constexpr std::string_view HINT_NOT_FOUND =
    R"(  - Did you use a correct identifier ?
  - Did you use a correct type ?
  - Did you use a correct path ?
  - Did you are in correct scope ?)";

struct Semantic : Base {
  // keep parent constructor
  using Base::Base;

  [[nodiscard]] bool start_resolver();


  semantic::Metadata& add_metadata(ast::ID nodeid, semantic::Metadata& metadata);
  semantic::Metadata& add_metadata(ast::ID nodeid, semantic::Metadata&& metadata);

  void resolve_node(ast::ID nodeid);

  void resolve_Root(ast::Root& n);
  void resolve_CodeBlock(ast::CodeBlock& n);
  void resolve_Global_Export(ast::Global_Export& n);
  void resolve_Global_Extern(ast::Global_Extern& n);

  void resolve_Statement_If(ast::Statement_If& n);
  void resolve_Statement_For(ast::Statement_For& n);
  void resolve_Statement_Loop(ast::Statement_Loop& n);
  void resolve_Statement_While(ast::Statement_While& n);
  void resolve_Statement_GoTo(ast::Statement_GoTo& n);
  void resolve_Statement_GoTo_Label(ast::Statement_GoTo_Label& n);
  void resolve_Statement_Return(ast::Statement_Return& n);
  void resolve_Statement_Break(ast::Statement_Break& n);
  void resolve_Statement_Continue(ast::Statement_Continue& n);
  void resolve_Statement_Match(ast::Statement_Match& n);
  void resolve_Statement_Match_Case(ast::Statement_Match_Case& n);
  void resolve_Literal_Range(ast::Literal_Range& n);

  void resolve_Global_Function(ast::Global_Function& n);
  void resolve_Call_Contract(ast::Call_Contract& n);
  void resolve_Global_Variable(ast::Global_Variable& n);
  void resolve_Local_Variable(ast::Local_Variable& n);
  void resolve_Operation_Binary(ast::Operation_Binary& n);
  void resolve_Operation_Cast_As(ast::Operation_Cast_As& n);
  void resolve_Expression_Invocation(ast::Expression_Invocation& n);
  void resolve_Expression_Invocation_Arg(ast::Expression_Invocation_Arg& n);
  void resolve_Expression_Member_Access(ast::Expression_Member_Access& n);
  void resolve_Expression_Ptr_Val(ast::Expression_Ptr_Val& n);
};

} // namespace resolver
