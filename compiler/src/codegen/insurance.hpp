#pragma once

#include "codegen/llvm_forward.hpp"
#include "nexus/forward.hpp"

#include <string_view>

namespace codegen
{
class Builder;
struct Codegen_AST;

struct Insurance {
  Insurance(codegen::Codegen_AST& p_res);
  codegen::Codegen_AST& res;

  llvm::LLVMContext& ctx;
  Builder&           builder;
  llvm::Module*      mod;


  [[nodiscard]] llvm::Value*    ensure_rvalue(ast::ID expr, std::string_view name = "") noexcept;
  [[nodiscard]] llvm::Value*    ensure_lvalue(ast::ID expr, bool is_silent_error = false) noexcept;
  [[nodiscard]] llvm::Value*    ensure_variadic_arg(ast::ID expr) noexcept;
  [[nodiscard]] llvm::Constant* ensure_constant(ast::ID expr) noexcept;

  [[nodiscard]] llvm::Value*    ensure_init_global_value(ast::ID expr, bool is_const) noexcept;
  [[nodiscard]] llvm::Value*    ensure_init_value(ast::ID expr) noexcept;
  [[nodiscard]] llvm::Constant* ensure_null_global_value(type::ID tyid) noexcept;
  [[nodiscard]] llvm::Value*    ensure_null_value(type::ID tyid) noexcept;
};

} // namespace codegen