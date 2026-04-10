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
#include <llvm-19/llvm/ADT/APFloat.h>
#include <llvm-19/llvm/ADT/APInt.h>
#include <string>

#include "ast/ast_data.hpp"
#include "ast/ast_forward.hpp"
#include "ast/ast_numeric_128_bits.hpp"

struct Visitor_Codegen;
struct Symbol_Data;

namespace llvm
{
struct Value;
struct Constant;
struct Type;
struct StructType;
} // namespace llvm

class LLVM_Tools
{
public:
  LLVM_Tools(Visitor_Codegen& _v)
    : v(_v)
  {
  }
  Visitor_Codegen& v;

  llvm::Value* engage_move_semantic(ast::AExpression& target);
  llvm::Value* engage_copy_semantic(ast::AExpression& target);
  llvm::Value* engage_clone_semantic(ast::AExpression& target);

  std::expected<Symbol_Data*, std::string>      find_symbol(const ast::AExpression& expr);
  std::expected<ast::AExpression*, std::string> get_symbol_expression(const Symbol_Data& symbol);
  std::expected<llvm::Constant*, std::string>   create_constant(const ast::ALiteral& value);
  llvm::Type*                                   generate_parameter_type(ast::declaration::local::Parameter& param);
  llvm::Type*                                   get_primtive_type(EPrimType ty);


  llvm::Constant* get_cstr_constant(const std::string& val);
  llvm::Constant* get_str_constant(const std::string& val);
  llvm::Constant* get_text_constant(const std::u32string& val);
  llvm::Constant* get_int_constant(size_t bits_size, int64_t int_val, const std::string& str_val = "",
                                   bool is_signed = true, size_t radix = 10);
  llvm::Constant* get_float_constant(size_t bits_size, double double_val, const std::string& str_val = "");

  llvm::Constant* get_primtive_zeroinitializer(EPrimType ty);
  llvm::Constant* get_zeroinitializer(const ast::AType& ty);


  std::u32string utf8_to_utf32(const std::string& s);
  llvm::Value*   primitive_coerce(llvm::Value* val, llvm::Type* src, llvm::Type* dst);
};