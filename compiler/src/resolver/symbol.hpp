#pragma once


#include "ast/forward.hpp"
#include "id/nodeid.hpp"
#include "nexus/forward.hpp"
#include "resolver/base.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>


namespace resolver
{


struct Symbol final : Base {
  // keep parent constructor
  using Base::Base;

  size_t sym_resolved_count = 0;

  ast::ID current_form;
  ast::ID current_facet;
  ast::ID current_function;
  ast::ID current_rule;
  ast::ID current_lambda;

  void resolve_node(ast::ID nodeid);

  // returns the symbol id definition
  [[nodiscard]] definition::ID resolve_id_sym(scope::ID ctx, ast::ID nodeid, const std::string& id,
                                              bool is_silent_error = false);
  // returns the symbol id definition
  [[nodiscard]] definition::ID resolve_path_sym(module::ID ctx, ast::ID nodeid, const std::string& id,
                                                const std::vector<std::string>& path, ast::EPathAnchor anchor,
                                                bool is_silent_error = false);

  void add_resolution(ast::ID nodeid, definition::ID defid);

  [[nodiscard]] size_t start_resolver();
  void                 ensure_types_symbols();


  void resolve_Root(ast::Root& n);
  void resolve_CodeBlock(ast::CodeBlock& n);
  void resolve_Global_Export(ast::Global_Export& n);
  void resolve_Global_Extern(ast::Global_Extern& n);

  void resolve_Symbol_Id(ast::Symbol_Id& n);
  void resolve_Symbol_Qualified(ast::Symbol_Qualified& n);
  void resolve_Symbol_Type(ast::Symbol_Type& n);
  void resolve_Global_Function(ast::Global_Function& n);
  void resolve_Global_Variable(ast::Global_Variable& n);
  void resolve_Call_Contract(ast::Call_Contract& n);
  void resolve_Global_Extend_Fn(ast::Global_Extend_Fn& n);
  void resolve_SFM_Rule(ast::SFM_Rule& n);
  void resolve_Local_Variable(ast::Local_Variable& n);
  void resolve_Local_Lambda(ast::Local_Lambda& n);
  void resolve_Local_Parameter(ast::Local_Parameter& n);
  void resolve_Expression_Invocation(ast::Expression_Invocation& n);
  void resolve_Expression_Invocation_Arg(ast::Expression_Invocation_Arg& n);
  void resolve_Expression_Member_Access(ast::Expression_Member_Access& n);
  void resolve_Literal_Record(ast::Literal_Record& n);
  void resolve_Literal_Range(ast::Literal_Range& n);
  void resolve_Literal_Table(ast::Literal_Table& n);
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
  void resolve_Operation_Cast_As(ast::Operation_Cast_As& n);
  void resolve_Operation_Mem(ast::Operation_Mem& n);
};

constexpr std::string_view SYM_HINT =
    R"(
  - Did you write correctly the identifier?
  - Did you access correctly to the path?
  - Did you import the concerned module?
  - Did you define correctly the type?)";

} // namespace resolver
