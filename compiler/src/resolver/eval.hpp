#pragma once

#include "ast/forward.hpp"
#include "nexus/forward.hpp"

#include <string_view>


namespace resolver
{

struct Evaluator final {
  Evaluator() = delete;
  explicit Evaluator(cu::CU& p_CU)
    : CU(p_CU)
  {
  }

  cu::CU& CU;

  bool start_resolver();

  void add_error(ErrorCode code, const ast::NodeHeader& n, std::string_view msg, std::string_view hint) const;

  void add_error_two_nodes(ErrorCode, const ast::NodeHeader& first, const ast::NodeHeader& second, std::string_view msg,
                           std::string_view hint) const;

  void add_evaluation(ast::ID nodeid, ast::ID constant) noexcept;

  [[nodiscard]] ast::ID eval_node(ast::ID nodeid) noexcept;

  [[nodiscard]] bool float_almost_eq_ULP(const ast::Literal_Floating_Point& L, const ast::Literal_Floating_Point& R,
                                         unsigned maxULP = 4) noexcept;

  [[nodiscard]] ast::ID integral(const ast::Literal_Integral& L, const ast::Literal_Integral& R,
                                 ast::EOp_Bin op) noexcept;
  [[nodiscard]] ast::ID floating(const ast::Literal_Floating_Point& L, const ast::Literal_Floating_Point& R,
                                 ast::EOp_Bin op) noexcept;
  [[nodiscard]] ast::ID decimal(const ast::Literal_Fixed_Point& L, const ast::Literal_Fixed_Point& R,
                                ast::EOp_Bin op) noexcept;
  [[nodiscard]] ast::ID boolean(const ast::Literal_Boolean& L, const ast::Literal_Boolean& R, ast::EOp_Bin op) noexcept;
  [[nodiscard]] ast::ID boolean_not(ast::ID term) noexcept;
  [[nodiscard]] ast::ID scalar_minus(ast::ID term) noexcept;
  // no effect
  [[nodiscard]] ast::ID scalar_plus(ast::ID term) noexcept;
};

} // namespace resolver