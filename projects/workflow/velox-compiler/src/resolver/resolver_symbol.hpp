#pragma once


#include <string>
#include <string_view>

#include "ast/ast_base.hpp"
#include "nexus/ids.hpp"
#include "resolver_base.hpp"

#include "nexus/ast/ast.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"


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

  void resolve_node(ast::Node& node);
  void resolve_node(ast::ID nodeid);

  // returns the symbol id definition
  [[nodiscard]] symbol::ID resolve_id_sym(scope::ID ctx, const ast::Node& n, std::string_view id,
                                          bool is_silent_error = false);
  // returns the symbol id definition
  [[nodiscard]] symbol::ID resolve_path_sym(module::ID ctx, const ast::Node& n, std::string_view id,
                                            const std::vector<std::string>& path, EPathAnchor anchor,
                                            bool is_silent_error = false);

  [[nodiscard]] size_t start_resolver();

  void resolve_Identifier(ast::Identifier& n);
  void resolve_ID_Qualified(ast::ID_Qualified& n);
  void resolve_ID_Typed(ast::ID_Typed& n);
  void resolve_Expression_Call(ast::Expression_Call& n);
  void resolve_Statement_GoTo(ast::Statement_GoTo& n);
};

constexpr std::string_view SYM_HINT =
    R"(
  - Did you write correctly the identifier?
  - Did you access correctly to the path?
  - Did you import the concerned module?
  - Did you define correctly the type?)";

} // namespace resolver
