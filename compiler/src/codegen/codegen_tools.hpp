/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <cstdint>
#include <expected>
#include <llvm/IR/IRBuilder.h>

#include <string>
#include <string_view>
#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ast/forward.hpp"
#include "codegen/llvm_forward.hpp"


namespace codegen
{

struct Codegen_AST;

struct Tools {
  Tools(codegen::Codegen_AST& p_res);
  codegen::Codegen_AST& res;

  llvm::LLVMContext& ctx;
  llvm::IRBuilder<>& builder;
  llvm::Module*      mod;


  [[nodiscard]] llvm::Value* engage_move_semantic(ast::ID p_target) noexcept;
  [[nodiscard]] llvm::Value* engage_copy_semantic(ast::ID p_target) noexcept;

  [[nodiscard]] std::expected<definition::ID, std::string>  find_definition(ast::ID expr_id) noexcept;
  [[nodiscard]] std::expected<ast::ID, std::string>         get_definition_expression(definition::ID defid) noexcept;
  [[nodiscard]] std::expected<llvm::Constant*, std::string> create_constant(ast::ID lit_id) noexcept;


  [[nodiscard]] llvm::Constant* get_cstr_constant(std::string_view val) noexcept;
  [[nodiscard]] llvm::Constant* get_str_constant(std::string_view val) noexcept;
  [[nodiscard]] llvm::Constant* get_text_constant(const std::u32string& val) noexcept;
  [[nodiscard]] llvm::Constant* get_int_constant(size_t bits_size, int64_t int_val, std::string_view str_val = "",
                                                 bool is_signed = true, size_t radix = 10) noexcept;
  [[nodiscard]] llvm::Constant* get_float_constant(size_t bits_size, double double_val,
                                                   std::string_view str_val = "") noexcept;

  [[nodiscard]] llvm::Constant* get_text_zeroinit(type::ETextType ty) noexcept;
  [[nodiscard]] llvm::Constant* get_primtive_zeroinit(type::EPrimitiveTypeKind ty) noexcept;
  [[nodiscard]] llvm::Constant* get_zeroinitializer(type::ID ty) noexcept;


  [[nodiscard]] std::u32string utf8_to_utf32(std::string_view s) noexcept;
  [[nodiscard]] llvm::Value*   primitive_coerce(llvm::Value* p_val, llvm::Type* p_src, llvm::Type* p_dst) noexcept;

  [[nodiscard]] llvm::Value* codegen_explicit_cast(ast::ID from, type::ID to) const noexcept;

  [[nodiscard]] llvm::Value* make_uninit(llvm::Type* ty, llvm::Value* val = nullptr) const noexcept;
  [[nodiscard]] llvm::Value* make_zeroinit(llvm::Type* ty) const noexcept;
  [[nodiscard]] llvm::Value* make_struct(llvm::Type* ty, std::vector<llvm::Value*> fields) const noexcept;
  [[nodiscard]] llvm::Value* builtin_c_strlen(llvm::Value* in) const noexcept;

  // returns start, length, end included
  [[nodiscard]] std::tuple<llvm::Value*, llvm::Value*, bool> get_span(ast::ID nodeid) const noexcept;
  [[nodiscard]] llvm::Value*    codegen_unary_op(ast::ID term, ast::EOp_Unary unary_op) const noexcept;
  [[nodiscard]] llvm::Value*    codegen_binary_op(ast::ID left, ast::ID right, ast::EOp_Bin op_ty) const noexcept;
  [[nodiscard]] llvm::Constant* init_global_array(const type::Array& ty, llvm::ConstantArray* default_val,
                                                  bool is_uninit, bool is_const) const noexcept;
  [[nodiscard]] llvm::Value*    init_array(const type::Array& ty, llvm::ConstantArray* default_val,
                                           bool is_uninit) const noexcept;
};

} // namespace codegen
