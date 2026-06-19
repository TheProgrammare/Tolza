#include "data.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"


std::string_view ast::EAccessOpType_to_str(EAccessOpType opTy)
{
  switch (opTy) {
  case EAccessOpType::_index:       return "[i]";
  case EAccessOpType::_index_bound: return "?[i]";
  case EAccessOpType::_slice:       return "[a..b]";
  case EAccessOpType::_slice_bound: return "?[a..b]";
  case EAccessOpType::_b_slice:     return "~[a..b]";
  };
}

ast::EBinOpType ast::ETokenKind_to_EBinOpType(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::OP_PLUS:          return EBinOpType::_add;
  case token::ETokenKind::OP_MINUS:         return EBinOpType::_sub;
  case token::ETokenKind::OP_ASTERISK:      return EBinOpType::_mul;
  case token::ETokenKind::OP_DIVIDE:        return EBinOpType::_div;
  case token::ETokenKind::OP_MODULO:        return EBinOpType::_mod;
  case token::ETokenKind::OP_QUOTIEN:       return EBinOpType::_quo;
  case token::ETokenKind::OP_REMAIN:        return EBinOpType::_rem;
  case token::ETokenKind::OP_POWER:         return EBinOpType::_pow;
  case token::ETokenKind::IN:               return EBinOpType::_in;
  case token::ETokenKind::NIN:              return EBinOpType::_nin;
  case token::ETokenKind::IS:               return EBinOpType::_is;
  case token::ETokenKind::NIS:              return EBinOpType::_nis;
  case token::ETokenKind::R_ANGLE:          return EBinOpType::_gre;
  case token::ETokenKind::L_ANGLE:          return EBinOpType::_low;
  case token::ETokenKind::OP_GEQ:           return EBinOpType::_gre_eq;
  case token::ETokenKind::OP_LEQ:           return EBinOpType::_low_eq;
  case token::ETokenKind::OP_EQ:            return EBinOpType::_eq;
  case token::ETokenKind::OP_EQS:           return EBinOpType::_eqs;
  case token::ETokenKind::OP_NEQ:           return EBinOpType::_neq;
  case token::ETokenKind::OP_NEQS:          return EBinOpType::_neqs;
  case token::ETokenKind::OP_AND:           return EBinOpType::_and;
  case token::ETokenKind::OP_B_AND:         return EBinOpType::_b_and;
  case token::ETokenKind::OP_NAND:          return EBinOpType::_nand;
  case token::ETokenKind::OP_B_NAND:        return EBinOpType::_b_nand;
  case token::ETokenKind::OP_OR:            return EBinOpType::_or;
  case token::ETokenKind::OP_B_OR:          return EBinOpType::_b_or;
  case token::ETokenKind::OP_XOR:           return EBinOpType::_xor;
  case token::ETokenKind::OP_B_XOR:         return EBinOpType::_b_xor;
  case token::ETokenKind::OP_NOR:           return EBinOpType::_nor;
  case token::ETokenKind::OP_B_NOR:         return EBinOpType::_b_nor;
  case token::ETokenKind::OP_XNOR:          return EBinOpType::_xnor;
  case token::ETokenKind::OP_B_XNOR:        return EBinOpType::_b_xnor;
  case token::ETokenKind::OP_SHIFT_LEFT_0:  return EBinOpType::_b_shl_0;
  case token::ETokenKind::OP_SHIFT_LEFT_1:  return EBinOpType::_b_shl_1;
  case token::ETokenKind::OP_SHIFT_LEFT_A:  return EBinOpType::_b_shl_a;
  case token::ETokenKind::OP_SHIFT_RIGHT_0: return EBinOpType::_b_shr_0;
  case token::ETokenKind::OP_SHIFT_RIGHT_1: return EBinOpType::_b_shr_1;
  case token::ETokenKind::OP_SHIFT_RIGHT_A: return EBinOpType::_b_shr_a;
  case token::ETokenKind::OP_ROTATE_LEFT:   return EBinOpType::_b_rol;
  case token::ETokenKind::OP_ROTATE_RIGHT:  return EBinOpType::_b_ror;
  default:                                  return EBinOpType::NONE;
  }
}

std::string_view ast::EBinOpType_to_str(EBinOpType opTy)
{
  switch (opTy) {
  case EBinOpType::NONE:     return "NO BIN OP TYPE";

  case EBinOpType::_add:     return "+";
  case EBinOpType::_sub:     return "-";
  case EBinOpType::_mul:     return "*";
  case EBinOpType::_div:     return "/";
  case EBinOpType::_mod:     return "%mod%";
  case EBinOpType::_quo:     return "%quo%";
  case EBinOpType::_rem:     return "%rem%";
  case EBinOpType::_divrem:  return "%divrem%";
  case EBinOpType::_pow:     return "**";

  case EBinOpType::_gre:     return ">";
  case EBinOpType::_low:     return "<";
  case EBinOpType::_gre_eq:  return ">=";
  case EBinOpType::_low_eq:  return "<=";
  case EBinOpType::_eq:      return "==";
  case EBinOpType::_in:      return "in";
  case EBinOpType::_nin:     return "nin";
  case EBinOpType::_is:      return "is";
  case EBinOpType::_nis:     return "nis";
  case EBinOpType::_neq:     return "!=";
  case EBinOpType::_eqs:     return "===";
  case EBinOpType::_neqs:    return "!==";

  case EBinOpType::_and:     return "and";
  case EBinOpType::_nand:    return "nand";
  case EBinOpType::_or:      return "or";
  case EBinOpType::_xor:     return "xor";
  case EBinOpType::_nor:     return "nor";
  case EBinOpType::_xnor:    return "nxor";
  case EBinOpType::_b_and:   return "b.and";
  case EBinOpType::_b_nand:  return "b.nand";
  case EBinOpType::_b_or:    return "b.or";
  case EBinOpType::_b_xor:   return "b.xor";
  case EBinOpType::_b_nor:   return "b.nor";
  case EBinOpType::_b_xnor:  return "b.nxor";

  case EBinOpType::_b_shl_0: return "b.shl.0";
  case EBinOpType::_b_shl_1: return "b.shl.1";
  case EBinOpType::_b_shl_a: return "b.shl.a";
  case EBinOpType::_b_shr_0: return "b.shr.0";
  case EBinOpType::_b_shr_1: return "b.shr.1";
  case EBinOpType::_b_shr_a: return "b.shr.a";
  case EBinOpType::_b_rol:   return "b.rol";
  case EBinOpType::_b_ror:   return "b.ror";
  }
}

ast::ECapability ast::ETokenKind_to_ECapability(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return ECapability::Mut;
  case token::ETokenKind::CAPA_REF:  return ECapability::Ref;
  case token::ETokenKind::CAPA_COPY: return ECapability::Copy;
  case token::ETokenKind::CAPA_MOVE: return ECapability::Move;
  default:                           return ECapability::NONE;
  }
}

std::string_view ast::ECapability_to_str(ECapability capa)
{
  switch (capa) {
  case ECapability::Mut:  return "mut";
  case ECapability::Ref:  return "ref";
  case ECapability::Copy: return "copy";
  case ECapability::Move: return "move";
  case ECapability::NONE: return "NO CAPABILITY";
  }
}

ast::ECapability ast::deduce_type_ECapability(bool is_complex, bool is_mut)
{
  if (is_mut) return ECapability::Mut;
  if (is_complex) return ECapability::Move;
  return ECapability::Copy;
}

ast::EUnaryOpType ast::ETokenKind_to_EUnaryOpType(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::OP_NOT:   return EUnaryOpType::_not;
  case token::ETokenKind::OP_PLUS:  return EUnaryOpType::_plus;
  case token::ETokenKind::OP_MINUS: return EUnaryOpType::_minus;
  default:                          return EUnaryOpType::NONE;
  }
}

std::string_view ast::EUnaryOpType_to_str(EUnaryOpType opTy)
{
  switch (opTy) {
  case EUnaryOpType::_not:   return "!";
  case EUnaryOpType::_plus:  return "+";
  case EUnaryOpType::_minus: return "-";
  default:                   return "NO UNARY OP TYPE";
  }
}

ast::EPassMode ast::ETokenKind_to_EPassMode(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EPassMode::Mut;
  case token::ETokenKind::CAPA_REF:  return EPassMode::Ref;
  case token::ETokenKind::CAPA_COPY: return EPassMode::Copy;
  case token::ETokenKind::CAPA_MOVE: return EPassMode::Move;
  case token::ETokenKind::ADDR:      return EPassMode::Addr;
  default:                           return EPassMode::NONE;
  }
}

std::string_view ast::EPassMode_to_str(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::Mut:  return "mut";
  case EPassMode::Ref:  return "ref";
  case EPassMode::Copy: return "copy";
  case EPassMode::Move: return "move";
  case EPassMode::Addr: return "addr";
  case EPassMode::NONE: return "NO PASS MODE";
  }
}

bool ast::EPassMode_Can_Default(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::Mut:  return false;
  case EPassMode::Ref:  return true;
  case EPassMode::Copy: return true;
  case EPassMode::Move: return false;
  case EPassMode::Addr: return false;
  case EPassMode::NONE: return true;
  }
}

ast::EExprPassMode ast::ETokenKind_to_EExprPassMode(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EExprPassMode::Mut;
  case token::ETokenKind::CAPA_REF:  return EExprPassMode::Ref;
  case token::ETokenKind::CAPA_COPY: return EExprPassMode::Copy;
  case token::ETokenKind::CAPA_MOVE: return EExprPassMode::Move;
  default:                           return EExprPassMode::NONE;
  }
}


ast::EVariableKind ast::ETokenKind_to_EVariableKind(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::LET:   return EVariableKind::Let;
  case token::ETokenKind::VAR:   return EVariableKind::Var;
  case token::ETokenKind::CONST: return EVariableKind::Const;
  default:                       return EVariableKind::NONE;
  }
}

std::string_view ast::EVariableKind_to_str(EVariableKind kind)
{
  switch (kind) {
  case EVariableKind::Const: return "const";
  case EVariableKind::Let:   return "let";
  case EVariableKind::Var:   return "var";
  case EVariableKind::NONE:  return "NO VAR KIND";
  }
}

ast::ETransfertType ast::ETokenKind_to_ETransfertType(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::ASSIGN:      return ETransfertType::MoveSemantic;
  case token::ETokenKind::COPY_ASSIGN: return ETransfertType::Copy;
  case token::ETokenKind::MOVE_ASSIGN: return ETransfertType::MoveSemantic;
  default:                             return ETransfertType::NONE;
  }
}

std::string_view ast::ETransfertType_to_str(ETransfertType type)
{
  switch (type) {
  case ETransfertType::Copy:         return "cppy";
  case ETransfertType::MoveSemantic: return "move";
  case ETransfertType::NONE:         return "NO TRANSFERT TYPE";
  }
}
