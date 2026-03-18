#pragma once


namespace ast
{

struct AType;

namespace type
{

struct Tuple;

ast::AType* get_bool_type();
ast::AType* get_i8_type();
ast::AType* get_i16_type();
ast::AType* get_i32_type();
ast::AType* get_i64_type();
ast::AType* get_i128_type();
ast::AType* get_isize_type();

// unsigned integers
ast::AType* get_u8_type();
ast::AType* get_u16_type();
ast::AType* get_u32_type();
ast::AType* get_u64_type();
ast::AType* get_u128_type();
ast::AType* get_usize_type();

// booleans
ast::AType* get_b8_type();
ast::AType* get_b16_type();
ast::AType* get_b32_type();
ast::AType* get_b64_type();
ast::AType* get_b128_type();
ast::AType* get_bsize_type();
ast::AType* get_ptrdiff_type();

// floats
ast::AType* get_f32_type();
ast::AType* get_f64_type();
ast::AType* get_f128_type();
ast::AType* get_fsize_type();

// decimals
ast::AType* get_deci_type();
ast::AType* get_udeci_type();

// other types
ast::AType*       get_void_type();
ast::type::Tuple* get_void_return_type();
ast::AType*       get_ascii_type();
ast::AType*       get_utf32_type();
ast::AType*       get_str_type();
ast::AType*       get_c_str_type();
ast::AType*       get_text_type();

} // namespace type
  // Type

} // namespace ast
  // AST