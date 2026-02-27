#include "ast_data.hpp"

#include "compiler/lexer/token.hpp"

EBinOpType TokTy_to_EBinOpType(TokTy tok)
{
  switch (tok) {
  case TokTy::OP_PLUS:        return EBinOpType::Add;
  case TokTy::OP_MINUS:       return EBinOpType::Sub;
  case TokTy::OP_ASTERISK:    return EBinOpType::Mul;
  case TokTy::OP_DIVIDE:      return EBinOpType::Div;
  case TokTy::OP_MODULO:      return EBinOpType::Mod;
  case TokTy::OP_QUOTIEN:     return EBinOpType::Quo;
  case TokTy::OP_REMAIN:      return EBinOpType::Rem;
  case TokTy::OP_POWER:       return EBinOpType::Pow;
  case TokTy::IN:             return EBinOpType::_in;
  case TokTy::NIN:            return EBinOpType::_nin;
  case TokTy::IS:             return EBinOpType::_is;
  case TokTy::NIS:            return EBinOpType::_nis;
  case TokTy::CLOSE_BRACKETS: return EBinOpType::Gre;
  case TokTy::OPEN_BRACKETS:  return EBinOpType::Low;
  case TokTy::OP_GEQ:         return EBinOpType::Gre_eq;
  case TokTy::OP_LEQ:         return EBinOpType::Low_eq;
  case TokTy::OP_EQ:          return EBinOpType::_eq;
  case TokTy::OP_EQS:         return EBinOpType::_eqs;
  case TokTy::OP_NEQ:         return EBinOpType::_neq;
  case TokTy::OP_NEQS:        return EBinOpType::_neqs;
  case TokTy::AND:            return EBinOpType::_and;
  case TokTy::B_AND:          return EBinOpType::_b_and;
  case TokTy::NAND:           return EBinOpType::_nand;
  case TokTy::B_NAND:         return EBinOpType::_b_nand;
  case TokTy::OR:             return EBinOpType::_or;
  case TokTy::B_OR:           return EBinOpType::_b_or;
  case TokTy::XOR:            return EBinOpType::_xor;
  case TokTy::B_XOR:          return EBinOpType::_b_xor;
  case TokTy::NOR:            return EBinOpType::_nor;
  case TokTy::B_NOR:          return EBinOpType::_b_nor;
  case TokTy::XNOR:           return EBinOpType::_xnor;
  case TokTy::B_XNOR:         return EBinOpType::_b_xnor;
  case TokTy::SHIFT_LEFT_0:   return EBinOpType::ls0;
  case TokTy::SHIFT_LEFT_1:   return EBinOpType::ls1;
  case TokTy::SHIFT_LEFT_A:   return EBinOpType::lsa;
  case TokTy::SHIFT_RIGHT_0:  return EBinOpType::rs0;
  case TokTy::SHIFT_RIGHT_1:  return EBinOpType::rs1;
  case TokTy::SHIFT_RIGHT_A:  return EBinOpType::rsa;
  case TokTy::ROTATE_LEFT:    return EBinOpType::lr;
  case TokTy::ROTATE_RIGHT:   return EBinOpType::rr;
  default:                    return EBinOpType::NONE;
  }
}

std::string EBinOpType_to_str(EBinOpType opTy)
{
  switch (opTy) {
  case EBinOpType::NONE:    return "NO BIN OP TYPE";

  case EBinOpType::Add:     return "+";
  case EBinOpType::Sub:     return "-";
  case EBinOpType::Mul:     return "*";
  case EBinOpType::Div:     return "/";
  case EBinOpType::Mod:     return "%mod%";
  case EBinOpType::Quo:     return "%quo%";
  case EBinOpType::Rem:     return "%rem%";
  case EBinOpType::Pow:     return "**";
  case EBinOpType::Sign:    return "+-";
  case EBinOpType::Index:   return "[i]";
  case EBinOpType::Slice:   return "[a..b]";
  case EBinOpType::bSlice:  return "~[a..b]";

  case EBinOpType::Gre:     return ">";
  case EBinOpType::Low:     return "<";
  case EBinOpType::Gre_eq:  return ">=";
  case EBinOpType::Low_eq:  return "<=";
  case EBinOpType::_eq:     return "==";
  case EBinOpType::_in:     return "in";
  case EBinOpType::_nin:    return "!in";
  case EBinOpType::_is:     return "is";
  case EBinOpType::_nis:    return "!is";
  case EBinOpType::_neq:    return "!=";
  case EBinOpType::_eqs:    return "===";
  case EBinOpType::_neqs:   return "!==";

  case EBinOpType::_and:    return "and";
  case EBinOpType::_nand:   return "!and";
  case EBinOpType::_or:     return "or";
  case EBinOpType::_xor:    return "xor";
  case EBinOpType::_nor:    return "!or";
  case EBinOpType::_xnor:   return "!xor";
  case EBinOpType::_b_and:  return "and.b";
  case EBinOpType::_b_nand: return "!and.b";
  case EBinOpType::_b_or:   return "or.b";
  case EBinOpType::_b_xor:  return "xor.b";
  case EBinOpType::_b_nor:  return "!or.b";
  case EBinOpType::_b_xnor: return "!xor.b";

  case EBinOpType::ls0:     return "<<[0]";
  case EBinOpType::ls1:     return "<<[1]";
  case EBinOpType::lsa:     return "<<[a]";
  case EBinOpType::rs0:     return "[0]>>";
  case EBinOpType::rs1:     return "[1]>>";
  case EBinOpType::rsa:     return "[a]>>";
  case EBinOpType::lr:      return "<<[r]";
  case EBinOpType::rr:      return "[r]>>";
  }
}

ECapability TokTy_to_ECapability(TokTy tok)
{
  switch (tok) {
  case TokTy::CAPA_MUT:   return ECapability::Mut;
  case TokTy::CAPA_REF:   return ECapability::Ref;
  case TokTy::CAPA_COPY:  return ECapability::Copy;
  case TokTy::CAPA_MOVE:  return ECapability::Move;
  case TokTy::CAPA_CLONE: return ECapability::Clone;
  default:                return ECapability::NONE;
  }
}

std::string ECapability_to_str(ECapability capa)
{
  switch (capa) {
  case ECapability::Mut:   return "mut";
  case ECapability::Ref:   return "ref";
  case ECapability::Clone: return "clone";
  case ECapability::Copy:  return "copy";
  case ECapability::Move:  return "move";
  case ECapability::NONE:  return "NO CAPABILITY";
  }
}

ECapability deduce_type_ECapability(bool is_complex, bool is_mut)
{
  if (is_mut) return ECapability::Mut;
  if (is_complex) return ECapability::Move;
  return ECapability::Copy;
}

EUnaryOpType TokTy_to_EUnaryOpType(TokTy tok)
{
  switch (tok) {
  case TokTy::NOT:      return EUnaryOpType::_not;
  case TokTy::OP_PLUS:  return EUnaryOpType::_pos;
  case TokTy::OP_MINUS: return EUnaryOpType::_neg;
  default:              return EUnaryOpType::NONE;
  }
}

std::string EUnaryOpType_to_str(EUnaryOpType opTy)
{
  switch (opTy) {
  case EUnaryOpType::_not: return "!";
  case EUnaryOpType::_pos: return "+";
  case EUnaryOpType::_neg: return "-";
  default:                 return "NO UNARY OP TYPE";
  }
}

EPassMode TokTy_to_EPassMode(TokTy tok)
{
  switch (tok) {
  case TokTy::CAPA_MUT:   return EPassMode::Mut;
  case TokTy::CAPA_REF:   return EPassMode::Ref;
  case TokTy::CAPA_COPY:  return EPassMode::Copy;
  case TokTy::CAPA_CLONE: return EPassMode::Clone;
  case TokTy::CAPA_MOVE:  return EPassMode::Move;
  case TokTy::ADDR:       return EPassMode::Addr;
  default:                return EPassMode::NONE;
  }
}

EPassMode str_to_EPassMode(std::string val)
{
  if (val == "move") return EPassMode::Move;
  if (val == "mut") return EPassMode::Mut;
  if (val == "copy") return EPassMode::Copy;
  if (val == "clone") return EPassMode::Clone;
  if (val == "addr") return EPassMode::Addr;
  if (val == "ref") return EPassMode::Ref;
  return EPassMode::NONE;
}

std::string EPassMode_to_str(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::Mut:   return "mut";
  case EPassMode::Ref:   return "ref";
  case EPassMode::Copy:  return "copy";
  case EPassMode::Clone: return "clone";
  case EPassMode::Move:  return "move";
  case EPassMode::Addr:  return "addr";
  case EPassMode::NONE:  return "NO PASS MODE";
  }
}

bool EPassMode_Can_Default(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::Mut:   return false;
  case EPassMode::Ref:   return true;
  case EPassMode::Copy:  return true;
  case EPassMode::Clone: return true;
  case EPassMode::Move:  return false;
  case EPassMode::Addr:  return false;
  case EPassMode::NONE:  return true;
  }
}

EExprPassMode TokTy_to_EExprPassMode(TokTy tok)
{
  switch (tok) {
  case TokTy::CAPA_MUT_OF:  return EExprPassMode::Mut;
  case TokTy::CAPA_REF_OF:  return EExprPassMode::Ref;
  case TokTy::CAPA_COPY_OF: return EExprPassMode::Copy;
  case TokTy::CAPA_MOVE_OF: return EExprPassMode::Move;
  default:                  return EExprPassMode::NONE;
  }
}

std::string EPrimTy_to_str(EPrimType type)
{
  switch (type) {
  case EPrimType::boolean:   return "boolean";
  case EPrimType::ASCII:     return "ASCII";
  case EPrimType::UTF32:     return "UTF32";
  case EPrimType::str:       return "string";
  case EPrimType::text:      return "text";

  case EPrimType::ptrdiff:   return "ptrdiff";

  case EPrimType::iSize:     return "iSize";
  case EPrimType::i8:        return "i8";
  case EPrimType::i16:       return "i16";
  case EPrimType::i32:       return "i32";
  case EPrimType::i64:       return "i64";
  case EPrimType::i128:      return "i128";

  case EPrimType::uSize:     return "uSize";
  case EPrimType::u8:        return "u8";
  case EPrimType::u16:       return "u16";
  case EPrimType::u32:       return "u32";
  case EPrimType::u64:       return "u64";
  case EPrimType::u128:      return "u128";

  case EPrimType::bSize:     return "bSize";
  case EPrimType::b8:        return "b8";
  case EPrimType::b16:       return "b16";
  case EPrimType::b32:       return "b32";
  case EPrimType::b64:       return "b64";
  case EPrimType::b128:      return "b128";

  case EPrimType::fSize:     return "fSize";
  case EPrimType::f32:       return "f32";
  case EPrimType::f64:       return "f64";
  case EPrimType::f128:      return "f128";

  case EPrimType::Void:      return "Void";
  case EPrimType::deci:      return "decimal";
  case EPrimType::udeci:     return "unsigned decimal";
  case EPrimType::Enum:      return "Enum";
  case EPrimType::Flag:      return "Flag";

  case EPrimType::Component: return "Component";
  case EPrimType::Role:      return "Role";
  case EPrimType::Entity:    return "Entity";
  case EPrimType::Generic:   return "Generic";
  case EPrimType::Function:  return "Function";
  case EPrimType::Fn_Proto:  return "Fnunction Proto";
  case EPrimType::tuple:     return "tuple";
  case EPrimType::Array:     return "Array";
  case EPrimType::map:       return "map";
  case EPrimType::Range:     return "Range";
  case EPrimType::Iterator:  return "Iterator";
  case EPrimType::Slice:     return "Slice";
  case EPrimType::NONE:      return "NO PRIMITIVE TYPE";
  }
}

std::string EPrimTy_to_mangle(EPrimType type)
{
  switch (type) {
  case EPrimType::boolean:   return "b";
  case EPrimType::ASCII:     return "aii";
  case EPrimType::UTF32:     return "utf";
  case EPrimType::str:       return "str";
  case EPrimType::text:      return "txt";

  case EPrimType::Void:      return "u0";
  case EPrimType::ptrdiff:   return "pdif";

  case EPrimType::iSize:     return "isz";
  case EPrimType::i8:        return "i8";
  case EPrimType::i16:       return "i16";
  case EPrimType::i32:       return "i32";
  case EPrimType::i64:       return "i64";
  case EPrimType::i128:      return "i128";

  case EPrimType::uSize:     return "usz";
  case EPrimType::u8:        return "u8";
  case EPrimType::u16:       return "u16";
  case EPrimType::u32:       return "u32";
  case EPrimType::u64:       return "u64";
  case EPrimType::u128:      return "u128";

  case EPrimType::bSize:     return "bsz";
  case EPrimType::b8:        return "b8";
  case EPrimType::b16:       return "b16";
  case EPrimType::b32:       return "b32";
  case EPrimType::b64:       return "b64";
  case EPrimType::b128:      return "b128";

  case EPrimType::fSize:     return "fsz";
  case EPrimType::f32:       return "f32";
  case EPrimType::f64:       return "f64";
  case EPrimType::f128:      return "f128";

  case EPrimType::deci:      return "deci";
  case EPrimType::udeci:     return "udeci";
  case EPrimType::Enum:      return "en";
  case EPrimType::Flag:      return "fg";

  case EPrimType::Component: return "cp";
  case EPrimType::Role:      return "Role";
  case EPrimType::Entity:    return "et";
  case EPrimType::Generic:   return "gn";
  case EPrimType::Function:  return "fn";
  case EPrimType::Fn_Proto:  return "fnp";
  case EPrimType::tuple:     return "tu";
  case EPrimType::Array:     return "arr";
  case EPrimType::map:       return "map";
  case EPrimType::Range:     return "rng";
  case EPrimType::Iterator:  return "iter";
  case EPrimType::Slice:     return "sli";
  case EPrimType::NONE:      return "NO PRIMITIVE TYPE";
  }
}

EPrimType TokTy_to_EPrimType(TokTy tok)
{
  switch (tok) {
  case TokTy::T_BOOL:    return EPrimType::boolean;
  case TokTy::T_UTF32:   return EPrimType::UTF32;
  case TokTy::T_ASCII:   return EPrimType::ASCII;
  case TokTy::T_STRING:  return EPrimType::str;
  case TokTy::T_TEXT:    return EPrimType::text;

  case TokTy::T_ISIZE:   return EPrimType::iSize;
  case TokTy::T_I8:      return EPrimType::i8;
  case TokTy::T_I16:     return EPrimType::i16;
  case TokTy::T_I32:     return EPrimType::i32;
  case TokTy::T_I64:     return EPrimType::i64;
  case TokTy::T_I128:    return EPrimType::i128;

  case TokTy::T_USIZE:   return EPrimType::uSize;
  case TokTy::T_U8:      return EPrimType::u8;
  case TokTy::T_U16:     return EPrimType::u16;
  case TokTy::T_U32:     return EPrimType::u32;
  case TokTy::T_U64:     return EPrimType::u64;
  case TokTy::T_U128:    return EPrimType::u128;

  case TokTy::T_BSIZE:   return EPrimType::bSize;
  case TokTy::T_B8:      return EPrimType::b8;
  case TokTy::T_B16:     return EPrimType::b16;
  case TokTy::T_B32:     return EPrimType::b32;
  case TokTy::T_B64:     return EPrimType::b64;
  case TokTy::T_B128:    return EPrimType::b128;

  case TokTy::T_FSIZE:   return EPrimType::fSize;
  case TokTy::T_F32:     return EPrimType::f32;
  case TokTy::T_F64:     return EPrimType::f64;
  case TokTy::T_F128:    return EPrimType::f128;

  case TokTy::T_VOID:    return EPrimType::Void;

  case TokTy::ENUM:      return EPrimType::Enum;
  case TokTy::FLAG:      return EPrimType::Flag;
  case TokTy::COMPONENT: return EPrimType::Component;
  case TokTy::ENTITY:    return EPrimType::Entity;
  case TokTy::GENERIC:   return EPrimType::Generic;
  case TokTy::FUNCTION:  return EPrimType::Function;

  default:               return EPrimType::NONE;
  }
}

EPtrType TokTy_to_EPtrType(TokTy tok)
{
  switch (tok) {
  case TokTy::PTR:  return EPtrType::raw_ptr;
  case TokTy::UPTR: return EPtrType::unique_ptr;
  case TokTy::SPTR: return EPtrType::shared_ptr;
  default:          return EPtrType::NONE;
  }
}

std::string EPtrType_to_str(EPtrType type)
{
  switch (type) {
  case EPtrType::raw_ptr:    return "ptr";
  case EPtrType::unique_ptr: return "uptr";
  case EPtrType::shared_ptr: return "sptr";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}

std::string EPtrType_to_mangle(EPtrType type)
{
  switch (type) {
  case EPtrType::raw_ptr:    return "p";
  case EPtrType::unique_ptr: return "up";
  case EPtrType::shared_ptr: return "sp";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}

EVariableKind TokTy_to_EVariableKind(TokTy tok)
{
  switch (tok) {
  case TokTy::LET:   return EVariableKind::Let;
  case TokTy::VAR:   return EVariableKind::Var;
  case TokTy::CONST: return EVariableKind::Const;
  default:           return EVariableKind::NONE;
  }
}

std::string EVariableKind_to_str(EVariableKind kind)
{
  switch (kind) {
  case EVariableKind::Const: return "const";
  case EVariableKind::Let:   return "let";
  case EVariableKind::Var:   return "var";
  case EVariableKind::NONE:  return "NO VAR KIND";
  }
}

EAssignmentType TokTy_to_EAssignmentType(TokTy tok)
{
  switch (tok) {
  case TokTy::ASSIGN:       return EAssignmentType::MoveSemantic;
  case TokTy::COPY_ASSIGN:  return EAssignmentType::Copy;
  case TokTy::CLONE_ASSIGN: return EAssignmentType::Clone;
  case TokTy::MOVE_ASSIGN:  return EAssignmentType::MoveSemantic;
  default:                  return EAssignmentType::NONE;
  }
}

bool is_op_handled(EPrimType src, EBinOpType op)
{
  // return
  // op_primitive[static_cast<uint8_t>(op)][static_cast<uint8_t>(src)];
  return false;
}

bool is_cast_explicit(EPrimType src, EPrimType target)
{
  // return
  // cast_explicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}

bool is_cast_implicit(EPrimType src, EPrimType target)
{
  // return
  // cast_implicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}
