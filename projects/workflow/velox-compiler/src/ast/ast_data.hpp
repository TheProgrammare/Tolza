
#pragma once

#include <string>

enum class ETokenType;
using TokTy = ETokenType;

enum class EUnaryOpType {
  NONE,
  // arithmetic
  _not,         // not !
  _plus,        // as positive +
  _minus,       // as negative -
  _invert_sign, // invert sign
};

[[nodiscard]] EUnaryOpType TokTy_to_EUnaryOpType(TokTy tok);

[[nodiscard]] std::string EUnaryOpType_to_str(EUnaryOpType opTy);

enum class EAccessOpType {
  Index,      // NO BIN OP AST USED, index[i] -> T
  IndexBound, // NO BIN OP AST USED, index?[i] -> T?
  Slice,      // NO BIN OP AST USED, slicing[start..end] -> Slice<T>
  SliceBound, // NO BIN OP AST USED, slicing?[start..end] -> Slice<T>?
  bSlice,     // NO BIN OP AST USED, slicing bits ~[start..end]
};

[[nodiscard]] std::string EAccessOpType_to_str(EAccessOpType opTy);


enum class EBinOpType {
  NONE,
  // arithmetic
  Add,    // add +
  Sub,    // substract -
  Mul,    // multiply *
  Div,    // divide /
  Mod,    // modulo %mod% not signed if divided > 0
  Quo,    // quotien %quo%
  Rem,    // remain %rem% signed with dividend
  Divrem, // quotien + remainder in one operation %divrem%
  Pow,    // power **
  // comparator
  Gre,    // greater >
  Low,    // lower <
  Gre_eq, // greater equal >=
  Low_eq, // lower equal <=
  _eq,    // equal ==
  _in,    // inside in
  _nin,   // not inside !in or nin
  _is,    // is
  _nis,   // not is !is or nis
  _neq,   // not equal !=
  _eqs,   // equal strictly ===
  _neqs,  // not equal strictly !==
  // logical
  _and,    // and logical and
  _nand,   // not and logical !and or nand
  _or,     // or logical or
  _xor,    // xor logical xor
  _nor,    // not or logical !or or nor
  _xnor,   // not xor logical !xor or xnor
  _b_and,  // and bitwise b_and
  _b_nand, // not and bitwise !b_and or b_nand
  _b_or,   // or bitwise b_or
  _b_xor,  // xor bitwise b_xor
  _b_nor,  // not or bitwise !b_or or b_nor
  _b_xnor, // not xor bitwise !b_xor or b_xnor

  ls0, // left shift fill 0 <<[0]
  ls1, // left shift fill 1 <<[1]
  lsa, // left shift arithmetic (fill with MSB) <<[a]
  rs0, // right shift fill 0 [0]>>
  rs1, // right shift fill 1 [1]>>
  rsa, // right shift arithmetic (fill with MSB) [a]>>
  lr,  // left rotate <<[r]
  rr,  // right rotate [r]>>
};

[[nodiscard]] EBinOpType TokTy_to_EBinOpType(TokTy tok);

[[nodiscard]] std::string EBinOpType_to_str(EBinOpType op);

[[nodiscard]] inline bool EBinOpType_is_logical(EBinOpType op)
{
  switch (op) {
  case EBinOpType::_and:
  case EBinOpType::_nand:
  case EBinOpType::_or:
  case EBinOpType::_xor:
  case EBinOpType::_nor:
  case EBinOpType::_xnor: return true;
  default:                return false;
  }
}


[[nodiscard]] inline bool EBinOpType_is_comparison(EBinOpType op)
{
  switch (op) {

  case EBinOpType::Gre:
  case EBinOpType::Low:
  case EBinOpType::Gre_eq:
  case EBinOpType::Low_eq:
  case EBinOpType::_eq:
  case EBinOpType::_in:
  case EBinOpType::_nin:
  case EBinOpType::_is:
  case EBinOpType::_nis:
  case EBinOpType::_neq:
  case EBinOpType::_eqs:
  case EBinOpType::_neqs:
  case EBinOpType::_and:
  case EBinOpType::_nand:
  case EBinOpType::_or:
  case EBinOpType::_xor:
  case EBinOpType::_nor:
  case EBinOpType::_xnor:  return true;
  default:                 return false;
  }
}

[[nodiscard]] inline bool EBinOpType_is_bitwise(EBinOpType op)
{
  switch (op) {
  case EBinOpType::_b_and:
  case EBinOpType::_b_nand:
  case EBinOpType::_b_or:
  case EBinOpType::_b_xor:
  case EBinOpType::_b_nor:
  case EBinOpType::_b_xnor:
  case EBinOpType::ls0:
  case EBinOpType::ls1:
  case EBinOpType::lsa:
  case EBinOpType::rs0:
  case EBinOpType::rs1:
  case EBinOpType::rsa:
  case EBinOpType::lr:
  case EBinOpType::rr:      return true;
  default:                  return false;
  }
}

constexpr auto k_boolean_op = {
    EBinOpType::_eq, EBinOpType::_in, EBinOpType::_nin, EBinOpType::_is, EBinOpType::_nis, EBinOpType::_neq,
};

constexpr auto k_textual_op = {
    EBinOpType::_eq,  EBinOpType::_in,   EBinOpType::_nin, EBinOpType::_is, EBinOpType::_nis, EBinOpType::_neq,

    EBinOpType::_eqs, EBinOpType::_neqs,

    EBinOpType::Add,
};

constexpr auto k_bitwise_op = {
    EBinOpType::_eq,    EBinOpType::_in,     EBinOpType::_nin,  EBinOpType::_is,
    EBinOpType::_nis,   EBinOpType::_neq,

    EBinOpType::_b_and, EBinOpType::_b_nand, EBinOpType::_b_or, EBinOpType::_b_xor,
    EBinOpType::_b_nor, EBinOpType::_b_xnor,

    EBinOpType::ls0,    EBinOpType::ls1,     EBinOpType::lsa,   EBinOpType::rs0,
    EBinOpType::rs1,    EBinOpType::rsa,     EBinOpType::lr,    EBinOpType::rr,
};

constexpr auto k_decimal_op = {
    EBinOpType::Add,    EBinOpType::Sub,   EBinOpType::Mul,    EBinOpType::Div,    EBinOpType::Mod,  EBinOpType::Rem,
    EBinOpType::Divrem, EBinOpType::Pow,

    EBinOpType::Gre,    EBinOpType::Low,   EBinOpType::Gre_eq, EBinOpType::Low_eq,

    EBinOpType::_eq,    EBinOpType::_in,   EBinOpType::_nin,   EBinOpType::_is,    EBinOpType::_nis, EBinOpType::_neq,

    EBinOpType::_eqs,   EBinOpType::_neqs,
};

constexpr auto k_integral_op = {
    EBinOpType::Add, EBinOpType::Sub,    EBinOpType::Mul,    EBinOpType::Div,    EBinOpType::Mod,  EBinOpType::Quo,
    EBinOpType::Rem, EBinOpType::Divrem, EBinOpType::Pow,

    EBinOpType::Gre, EBinOpType::Low,    EBinOpType::Gre_eq, EBinOpType::Low_eq,

    EBinOpType::_eq, EBinOpType::_in,    EBinOpType::_nin,   EBinOpType::_is,    EBinOpType::_nis, EBinOpType::_neq,
};


enum class ECapability { NONE, Ref, Mut, Copy, Clone, Move };
[[nodiscard]] ECapability TokTy_to_ECapability(TokTy tok);
[[nodiscard]] std::string ECapability_to_str(ECapability capa);

[[nodiscard]] ECapability deduce_type_ECapability(bool is_complex, bool is_mut);

enum class EPassMode { NONE, Mut, Ref, Copy, Clone, Move, Addr };
[[nodiscard]] EPassMode   TokTy_to_EPassMode(TokTy tok);
[[nodiscard]] EPassMode   str_to_EPassMode(std::string val);
[[nodiscard]] std::string EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool        EPassMode_Can_Default(EPassMode passMode);

enum class EExprPassMode { NONE, Mut, Ref, Copy, Move };
[[nodiscard]] EExprPassMode TokTy_to_EExprPassMode(TokTy tok);

enum class EPrimType {
  NONE,
  boolean,
  cune,
  rune,
  c_str,
  str,
  text,
  iSize,
  i8,
  i16,
  i32,
  i64,
  i128,
  uSize,
  u8,
  u16,
  u32,
  u64,
  u128,
  bSize,
  b8,
  b16,
  b32,
  b64,
  b128,
  ptrdiff,
  fSize,
  f16,
  f32,
  f64,
  f80,
  f128,
  u0,
  d32,
  d64,
  d128,
  dSize,
  ud32,
  ud64,
  ud128,
  udSize,
  Enum,
  Flag,
  Component,
  Role,
  Entity,
  Generic,
  Function,
  Fn_Proto,
  tuple,
  STable,
  DTable,
  SMatrix,
  DMatrix,
  SHyper,
  DHyper,
  map,
  Range,
  Iterator,
  Slice,
  COUNT,
};

[[nodiscard]] bool EPrimType_is_signed(EPrimType type);
[[nodiscard]] bool EPrimType_is_integral(EPrimType type);
[[nodiscard]] bool EPrimType_is_byte(EPrimType type);
[[nodiscard]] bool EPrimType_is_floating(EPrimType type);
[[nodiscard]] bool EPrimType_is_fixed(EPrimType type);
[[nodiscard]] bool EPrimType_is_textual(EPrimType type);


[[nodiscard]] std::string EPrimType_to_str(EPrimType type);

[[nodiscard]] std::string EPrimType_to_mangle(EPrimType type);

[[nodiscard]] EPrimType TokTy_to_EPrimType(TokTy tok);

enum class EPtrType {
  NONE,
  raw_ptr,
  unique_ptr,
  shared_ptr,
};

[[nodiscard]] EPtrType TokTy_to_EPtrType(TokTy tok);

[[nodiscard]] std::string EPtrType_to_str(EPtrType type);

[[nodiscard]] std::string EPtrType_to_mangle(EPtrType type);

enum class EVariableKind { NONE, Const, Let, Var };

[[nodiscard]] EVariableKind TokTy_to_EVariableKind(TokTy tok);
[[nodiscard]] std::string   EVariableKind_to_str(EVariableKind kind);

enum class ETransfertType { NONE, Copy, Clone, MoveSemantic };

[[nodiscard]] ETransfertType TokTy_to_ETransfertType(TokTy tok);
[[nodiscard]] std::string    ETransfertType_to_str(ETransfertType type);

enum class ESymbolType {
  NONE,
  Global,
  Local,
  Parameter,
  Function,
  Lambda,
  Generic_Parameter,
  Enum,
  Union,
  Flag,
  Bind,
  Component,
  System,
  System_Case,
  Role,
  Module,
  Export,
  Extern,
  Goto_Label,
  Entity,
  Entity_Op,
  Entity_OpIndex,
  Entity_Cast,
  Entity_New,
  Entity_Del,
  Entity_Transfert,
  Type_Alias,
  Mod_Alias,
  Generic,
};

enum class EScopeType {
  NONE,
  Mod,
  Export,
  Extern,
  Function,
  Lambda,
  Enum,
  Union,
  Flag,
  Role,
  Generic,
  Component,
  System,
  System_Where,
  Entity,
  Entity_Op,
  Entity_Cast,
  Entity_Transfert,
  Entity_New,
  Entity_Del,
  If,
  Else,
  Elif,
  For,
  While,
  Do_While,
  Loop,
  Match,
  Match_Case,
};

[[nodiscard]] size_t EPrimType_to_bits(EPrimType type);
[[nodiscard]] size_t EPrimType_to_bytes(EPrimType type);

[[nodiscard]] bool is_op_handled(EPrimType src, EBinOpType op);
[[nodiscard]] bool is_cast_explicit(EPrimType src, EPrimType target);
[[nodiscard]] bool is_cast_implicit(EPrimType src, EPrimType target);
