#pragma once


#include <string>
#include <string_view>

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

  ast::_gnid current_entity;
  ast::_gnid current_component;
  ast::_gnid current_function;
  ast::_gnid current_system;
  ast::_gnid current_lambda;

  // returns the symbol id definition
  [[nodiscard]] symbol::_id resolve_id_sym(module::_id current_mod, const ast::Node& n, std::string_view id,
                                           bool is_silent_error = false);
  // returns the symbol id definition
  [[nodiscard]] symbol::_id resolve_path_sym(module::_id current_mod, const ast::Node& n, std::string_view id,
                                             const std::vector<std::string_view>& id_quali, ast::EPathSource path_src,
                                             bool is_silent_error = false);

  size_t start_resolver();

  void resolve_ID(ast::ID& n);
  void resolve_ID_Qualified(ast::ID_Qualified& n);
  void resolve_ID_Typed(ast::ID_Typed& n);
  void resolve_Expression_Call(ast::Expression_Call& n);
  void resolve_Statement_GoTo(ast::Statement_GoTo& n);
};

inline const std::string SYM_HINT =
    R"(
  - Did you write correctly the identifier?
  - Did you access correctly to the path?
  - Did you import the concerned module?
  - Did you define correctly the type?)";

} // namespace resolver
