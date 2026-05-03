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
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <string>

#include "nexus/forward.hpp"
#include "nexus/ast/forward.hpp"

namespace resolver
{
struct Codegen;
}

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
  LLVM_Tools(resolver::Codegen& _v)
    : v(_v)
  {
  }
  resolver::Codegen& v;

  /*
  llvm::Value* engage_move_semantic(ast::Node& p_target);
  llvm::Value* engage_copy_semantic(ast::Node& p_target);

  std::expected<symbol::_id, std::string> find_symbol(ast::Node& p_expr);
  std::expected<ast::_gnid, std::string>      get_symbol_expression(symbol::_id p_symbol);
  std::expected<llvm::Constant*, std::string>    create_constant(ast::Node& p_value);
  llvm::Type*                                    generate_parameter_type(ast::Local_Parameter& p_param);
  llvm::Type*                                    get_primtive_type(type::EPrimitiveTypeKind p_ty);


  llvm::Constant* get_cstr_constant(std::string_view val);
  llvm::Constant* get_str_constant(std::string_view val);
  llvm::Constant* get_text_constant(const std::u32string& val);
  llvm::Constant* get_int_constant(size_t bits_size, int64_t int_val, std::string_view str_val = "",
                                   bool is_signed = true, size_t radix = 10);
  llvm::Constant* get_float_constant(size_t bits_size, double double_val, std::string_view str_val = "");

  llvm::Constant* get_primtive_zeroinitializer(type::EPrimitiveTypeKind ty);
  llvm::Constant* get_zeroinitializer(type::_id ty);


  std::u32string utf8_to_utf32(std::string_view s);
  llvm::Value*   primitive_coerce(llvm::Value* p_val, llvm::Type* p_src, llvm::Type* p_dst);
  */
};