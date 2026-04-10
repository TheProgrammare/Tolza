#pragma once

#include <memory>

namespace ast
{

struct AType;

namespace type
{

std::shared_ptr<ast::AType> get_bool_type();
std::shared_ptr<ast::AType> get_i8_type();
std::shared_ptr<ast::AType> get_i16_type();
std::shared_ptr<ast::AType> get_i32_type();
std::shared_ptr<ast::AType> get_i64_type();
std::shared_ptr<ast::AType> get_i128_type();
std::shared_ptr<ast::AType> get_isize_type();

// unsigned integers
std::shared_ptr<ast::AType> get_u8_type();
std::shared_ptr<ast::AType> get_u16_type();
std::shared_ptr<ast::AType> get_u32_type();
std::shared_ptr<ast::AType> get_u64_type();
std::shared_ptr<ast::AType> get_u128_type();
std::shared_ptr<ast::AType> get_usize_type();

// booleans
std::shared_ptr<ast::AType> get_b8_type();
std::shared_ptr<ast::AType> get_b16_type();
std::shared_ptr<ast::AType> get_b32_type();
std::shared_ptr<ast::AType> get_b64_type();
std::shared_ptr<ast::AType> get_b128_type();
std::shared_ptr<ast::AType> get_bsize_type();
std::shared_ptr<ast::AType> get_ptrdiff_type();

// floats
std::shared_ptr<ast::AType> get_f16_type();
std::shared_ptr<ast::AType> get_f32_type();
std::shared_ptr<ast::AType> get_f64_type();
std::shared_ptr<ast::AType> get_f80_type();
std::shared_ptr<ast::AType> get_f128_type();
std::shared_ptr<ast::AType> get_fsize_type();

// decimals
std::shared_ptr<ast::AType> get_d32_type();
std::shared_ptr<ast::AType> get_d64_type();
std::shared_ptr<ast::AType> get_d128_type();
std::shared_ptr<ast::AType> get_dsize_type();
std::shared_ptr<ast::AType> get_ud32_type();
std::shared_ptr<ast::AType> get_ud64_type();
std::shared_ptr<ast::AType> get_ud128_type();
std::shared_ptr<ast::AType> get_udsize_type();

// other types
std::shared_ptr<ast::AType> get_void_type();
std::shared_ptr<ast::AType> get_cune_type();
std::shared_ptr<ast::AType> get_rune_type();
std::shared_ptr<ast::AType> get_c_str_type();
std::shared_ptr<ast::AType> get_str_type();
std::shared_ptr<ast::AType> get_text_type();

} // namespace type
  // Type

} // namespace ast
  // AST