#pragma once

#include <expected>
#include <string>

#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "llvm_forward.hpp"

namespace codegen
{
struct Codegen_AST;
struct Tools;

struct Static_Evaluator {
  Static_Evaluator(codegen::Codegen_AST& p_res);

  codegen::Codegen_AST& res;
  Tools*                tools;

  [[nodiscard]] std::expected<llvm::Constant*, std::string> evaluate_expression(ast::ID n) noexcept;

  [[nodiscard]] bool float_almost_eq_ULP(const llvm::APFloat& L, const llvm::APFloat& R, unsigned maxULP = 4) noexcept;

  [[nodiscard]] std::expected<llvm::Constant*, std::string>
  integral(const ast::Literal_Integral& L, const ast::Literal_Integral& R, ast::EOp_Bin op) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string>
  floating(const ast::Literal_Floating_Point& L, const ast::Literal_Floating_Point& R, ast::EOp_Bin op) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string>
  decimal(const ast::Literal_Fixed_Point& L, const ast::Literal_Fixed_Point& R, ast::EOp_Bin op) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string>
  boolean(const ast::Literal_Boolean& L, const ast::Literal_Boolean& R, ast::EOp_Bin op) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string> boolean_not(ast::ID term) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string> scalar_minus(ast::ID term) noexcept;
  // no effect
  [[nodiscard]] std::expected<llvm::Constant*, std::string> scalar_plus(ast::ID term) noexcept;
};

} // namespace codegen