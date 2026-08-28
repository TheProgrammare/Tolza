#pragma once


#include "ast/ast_base.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "resolver_base.hpp"

#include <string>
#include <string_view>


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

  void resolve_node(ast::ID nodeid) noexcept;

  // returns the symbol id definition
  [[nodiscard]] definition::ID resolve_id_sym(scope::ID ctx, ast::ID nodeid, std::string_view id,
                                              bool is_silent_error = false) noexcept;
  // returns the symbol id definition
  [[nodiscard]] definition::ID resolve_path_sym(module::ID ctx, ast::ID nodeid, std::string_view id,
                                                const std::vector<std::string>& path, EPathAnchor anchor,
                                                bool is_silent_error = false) noexcept;

  void add_resolution(ast::ID nodeid, definition::ID defid) noexcept;

  [[nodiscard]] size_t start_resolver() noexcept;
  void                 ensure_types_symbols() noexcept;

  void resolve_Symbol_Id(ast::Symbol_Id& n) noexcept;
  void resolve_Symbol_Qualified(ast::Symbol_Qualified& n) noexcept;
  void resolve_Symbol_Type(ast::Symbol_Type& n) noexcept;
  void resolve_Global_Function(ast::Global_Function& n) noexcept;
  void resolve_Global_Extend_Fn(ast::Global_Extend_Fn& n) noexcept;
  void resolve_SFM_Rule(ast::SFM_Rule& n) noexcept;
  void resolve_Local_Lambda(ast::Local_Lambda& n) noexcept;
  void resolve_Expression_Invocation(ast::Expression_Invocation& n) noexcept;
  void resolve_Statement_GoTo(ast::Statement_GoTo& n) noexcept;
  void resolve_Literal_Record(ast::Literal_Record& n) noexcept;
};

constexpr std::string_view SYM_HINT =
    R"(
  - Did you write correctly the identifier?
  - Did you access correctly to the path?
  - Did you import the concerned module?
  - Did you define correctly the type?)";

} // namespace resolver
