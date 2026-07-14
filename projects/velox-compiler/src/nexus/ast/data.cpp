#include "data.hpp"
#include "Neargye/magic_enum.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include <string_view>


std::string_view ast::EOp_Subscript_to_str(EOp_Subscript opTy)
{
  switch (opTy) {
  case EOp_Subscript::_index:       return "[i]";
  case EOp_Subscript::_index_bound: return "?[i]";
  case EOp_Subscript::_slice:       return "[a..b]";
  case EOp_Subscript::_slice_bound: return "?[a..b]";
  case EOp_Subscript::_b_slice:     return "~[a..b]";
  };
}

ast::EOp_Bin ast::ETokenKind_to_EOp_Bin(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::ASSIGN_PLUS:
  case token::ETokenKind::OP_PLUS:              return EOp_Bin::_add;
  case token::ETokenKind::ASSIGN_MINUS:
  case token::ETokenKind::OP_MINUS:             return EOp_Bin::_sub;
  case token::ETokenKind::ASSIGN_MULTIPLY:
  case token::ETokenKind::OP_MULTIPLY:          return EOp_Bin::_mul;
  case token::ETokenKind::ASSIGN_DIVIDE:
  case token::ETokenKind::OP_DIVIDE:            return EOp_Bin::_div;
  case token::ETokenKind::ASSIGN_MODULO:
  case token::ETokenKind::OP_MODULO:            return EOp_Bin::_mod;
  case token::ETokenKind::ASSIGN_QUOTIEN:
  case token::ETokenKind::OP_QUOTIEN:           return EOp_Bin::_quo;
  case token::ETokenKind::ASSIGN_REMAIN:
  case token::ETokenKind::OP_REMAIN:            return EOp_Bin::_rem;
  case token::ETokenKind::ASSIGN_POWER:
  case token::ETokenKind::OP_POWER:             return EOp_Bin::_pow;
  case token::ETokenKind::IN:                   return EOp_Bin::_in;
  case token::ETokenKind::NIN:                  return EOp_Bin::_nin;
  case token::ETokenKind::IS:                   return EOp_Bin::_is;
  case token::ETokenKind::NIS:                  return EOp_Bin::_nis;
  case token::ETokenKind::R_ANGLE:              return EOp_Bin::_gre;
  case token::ETokenKind::L_ANGLE:              return EOp_Bin::_low;
  case token::ETokenKind::OP_GEQ:               return EOp_Bin::_gre_eq;
  case token::ETokenKind::OP_LEQ:               return EOp_Bin::_low_eq;
  case token::ETokenKind::OP_EQ:                return EOp_Bin::_eq;
  case token::ETokenKind::OP_EQS:               return EOp_Bin::_eqs;
  case token::ETokenKind::OP_NEQ:               return EOp_Bin::_neq;
  case token::ETokenKind::OP_NEQS:              return EOp_Bin::_neqs;
  case token::ETokenKind::ASSIGN_AND:
  case token::ETokenKind::OP_AND:               return EOp_Bin::_and;
  case token::ETokenKind::ASSIGN_B_AND:
  case token::ETokenKind::OP_B_AND:             return EOp_Bin::_b_and;
  case token::ETokenKind::ASSIGN_NAND:
  case token::ETokenKind::OP_NAND:              return EOp_Bin::_nand;
  case token::ETokenKind::ASSIGN_B_NAND:
  case token::ETokenKind::OP_B_NAND:            return EOp_Bin::_b_nand;
  case token::ETokenKind::ASSIGN_OR:
  case token::ETokenKind::OP_OR:                return EOp_Bin::_or;
  case token::ETokenKind::ASSIGN_B_OR:
  case token::ETokenKind::OP_B_OR:              return EOp_Bin::_b_or;
  case token::ETokenKind::ASSIGN_XOR:
  case token::ETokenKind::OP_XOR:               return EOp_Bin::_xor;
  case token::ETokenKind::ASSIGN_B_XOR:
  case token::ETokenKind::OP_B_XOR:             return EOp_Bin::_b_xor;
  case token::ETokenKind::ASSIGN_NOR:
  case token::ETokenKind::OP_NOR:               return EOp_Bin::_nor;
  case token::ETokenKind::ASSIGN_B_NOR:
  case token::ETokenKind::OP_B_NOR:             return EOp_Bin::_b_nor;
  case token::ETokenKind::ASSIGN_XNOR:
  case token::ETokenKind::OP_XNOR:              return EOp_Bin::_xnor;
  case token::ETokenKind::ASSIGN_B_XNOR:
  case token::ETokenKind::OP_B_XNOR:            return EOp_Bin::_b_xnor;
  case token::ETokenKind::ASSIGN_SHIFT_LEFT_0:
  case token::ETokenKind::OP_SHIFT_LEFT_0:      return EOp_Bin::_b_shl_0;
  case token::ETokenKind::ASSIGN_SHIFT_LEFT_1:
  case token::ETokenKind::OP_SHIFT_LEFT_1:      return EOp_Bin::_b_shl_1;
  case token::ETokenKind::ASSIGN_SHIFT_LEFT_A:
  case token::ETokenKind::OP_SHIFT_LEFT_A:      return EOp_Bin::_b_shl_a;
  case token::ETokenKind::ASSIGN_SHIFT_RIGHT_0:
  case token::ETokenKind::OP_SHIFT_RIGHT_0:     return EOp_Bin::_b_shr_0;
  case token::ETokenKind::ASSIGN_SHIFT_RIGHT_1:
  case token::ETokenKind::OP_SHIFT_RIGHT_1:     return EOp_Bin::_b_shr_1;
  case token::ETokenKind::ASSIGN_SHIFT_RIGHT_A:
  case token::ETokenKind::OP_SHIFT_RIGHT_A:     return EOp_Bin::_b_shr_a;
  case token::ETokenKind::ASSIGN_ROTATE_LEFT:
  case token::ETokenKind::OP_ROTATE_LEFT:       return EOp_Bin::_b_rol;
  case token::ETokenKind::ASSIGN_ROTATE_RIGHT:
  case token::ETokenKind::OP_ROTATE_RIGHT:      return EOp_Bin::_b_ror;
  default:                                      return EOp_Bin::NONE;
  }
}

std::string_view ast::EOp_Bin_to_str(EOp_Bin opTy)
{
  switch (opTy) {
  case EOp_Bin::NONE:      return "NO BIN OP TYPE";

  case EOp_Bin::_add:      return "+";
  case EOp_Bin::_sub:      return "-";
  case EOp_Bin::_mul:      return "*";
  case EOp_Bin::_div:      return "/";
  case EOp_Bin::_mod:      return "%mod%";
  case EOp_Bin::_quo:      return "%quo%";
  case EOp_Bin::_rem:      return "%rem%";
  case EOp_Bin::_divrem:   return "%divrem%";
  case EOp_Bin::_pow:      return "**";

  case EOp_Bin::_ordering: return "<=>";
  case EOp_Bin::_gre:      return ">";
  case EOp_Bin::_low:      return "<";
  case EOp_Bin::_gre_eq:   return ">=";
  case EOp_Bin::_low_eq:   return "<=";
  case EOp_Bin::_eq:       return "==";
  case EOp_Bin::_in:       return "in";
  case EOp_Bin::_nin:      return "nin";
  case EOp_Bin::_is:       return "is";
  case EOp_Bin::_nis:      return "nis";
  case EOp_Bin::_neq:      return "!=";
  case EOp_Bin::_eqs:      return "===";
  case EOp_Bin::_neqs:     return "!==";

  case EOp_Bin::_and:      return "and";
  case EOp_Bin::_nand:     return "nand";
  case EOp_Bin::_or:       return "or";
  case EOp_Bin::_xor:      return "xor";
  case EOp_Bin::_nor:      return "nor";
  case EOp_Bin::_xnor:     return "nxor";
  case EOp_Bin::_b_and:    return "b.and";
  case EOp_Bin::_b_nand:   return "b.nand";
  case EOp_Bin::_b_or:     return "b.or";
  case EOp_Bin::_b_xor:    return "b.xor";
  case EOp_Bin::_b_nor:    return "b.nor";
  case EOp_Bin::_b_xnor:   return "b.nxor";

  case EOp_Bin::_b_shl_0:  return "b.shl.0";
  case EOp_Bin::_b_shl_1:  return "b.shl.1";
  case EOp_Bin::_b_shl_a:  return "b.shl.a";
  case EOp_Bin::_b_shr_0:  return "b.shr.0";
  case EOp_Bin::_b_shr_1:  return "b.shr.1";
  case EOp_Bin::_b_shr_a:  return "b.shr.a";
  case EOp_Bin::_b_rol:    return "b.rol";
  case EOp_Bin::_b_ror:    return "b.ror";
  case EOp_Bin::_mem_add:  return "MEM_ADD";
  case EOp_Bin::_mem_sub:  return "MEM_SUB";
  case EOp_Bin::_mem_dist: return "<->";
  }
}

ast::ECapability ast::ETokenKind_to_ECapability(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return ECapability::mut;
  case token::ETokenKind::CAPA_REF:  return ECapability::ref;
  case token::ETokenKind::CAPA_COPY: return ECapability::copy;
  case token::ETokenKind::CAPA_MOVE: return ECapability::move;
  default:                           return ECapability::NONE;
  }
}

std::string_view ast::ECapability_to_str(ECapability capa)
{
  switch (capa) {
  case ECapability::mut:  return "mut";
  case ECapability::ref:  return "ref";
  case ECapability::copy: return "copy";
  case ECapability::move: return "move";
  case ECapability::NONE: return "NO CAPABILITY";
  }
}

ast::ECapability ast::deduce_type_ECapability(bool is_complex, bool is_mut)
{
  if (is_mut) return ECapability::mut;
  if (is_complex) return ECapability::move;
  return ECapability::copy;
}

ast::EOp_Unary ast::ETokenKind_to_EOp_Unary(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::EXCLAMATION:
  case token::ETokenKind::OP_NOT:      return EOp_Unary::_not;
  case token::ETokenKind::OP_PLUS:     return EOp_Unary::_plus;
  case token::ETokenKind::OP_MINUS:    return EOp_Unary::_minus;
  default:                             return EOp_Unary::NONE;
  }
}

std::string_view ast::EOp_Unary_to_str(EOp_Unary opTy)
{
  switch (opTy) {
  case EOp_Unary::_not:   return "!";
  case EOp_Unary::_plus:  return "+";
  case EOp_Unary::_minus: return "-";
  default:                return "NO UNARY OP TYPE";
  }
}

ast::EPassMode ast::ETokenKind_to_EPassMode(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EPassMode::mut;
  case token::ETokenKind::CAPA_REF:  return EPassMode::ref;
  case token::ETokenKind::CAPA_COPY: return EPassMode::copy;
  case token::ETokenKind::CAPA_MOVE: return EPassMode::move;
  case token::ETokenKind::ADDR:      return EPassMode::addr;
  default:                           return EPassMode::NONE;
  }
}

std::string_view ast::EPassMode_to_str(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::mut:  return "mut";
  case EPassMode::ref:  return "ref";
  case EPassMode::copy: return "copy";
  case EPassMode::move: return "move";
  case EPassMode::addr: return "addr";
  case EPassMode::NONE: return "NO PASS MODE";
  }
}

bool ast::EPassMode_Can_Default(EPassMode passMode)
{
  switch (passMode) {
  case EPassMode::mut:
  case EPassMode::addr:
  case EPassMode::move: return false;
  case EPassMode::ref:
  case EPassMode::copy:
  case EPassMode::NONE: return true;
  }
}

ast::EExprPassMode ast::ETokenKind_to_EExprPassMode(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EExprPassMode::mut;
  case token::ETokenKind::CAPA_REF:  return EExprPassMode::ref;
  case token::ETokenKind::CAPA_COPY: return EExprPassMode::copy;
  case token::ETokenKind::CAPA_MOVE: return EExprPassMode::move;
  default:                           return EExprPassMode::NONE;
  }
}


ast::EVariableKind ast::ETokenKind_to_EVariableKind(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::LET:   return EVariableKind::_let;
  case token::ETokenKind::VAR:   return EVariableKind::_var;
  case token::ETokenKind::CONST: return EVariableKind::_const;
  default:                       return EVariableKind::NONE;
  }
}

std::string_view ast::EVariableKind_to_str(EVariableKind kind)
{
  switch (kind) {
  case EVariableKind::_const: return "const";
  case EVariableKind::_let:   return "let";
  case EVariableKind::_var:   return "var";
  case EVariableKind::NONE:   return "NO VAR KIND";
  }
}

ast::ETransfertType ast::ETokenKind_to_ETransfertType(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::ASSIGN:
  case token::ETokenKind::MOVE_ASSIGN: return ETransfertType::move;
  case token::ETokenKind::COPY_ASSIGN: return ETransfertType::copy;
  default:                             {
    if (tok >= token::ETokenKind::ASSIGN_PLUS && tok <= token::ETokenKind::ASSIGN_ROTATE_RIGHT)
      return ETransfertType::copy;

    return ETransfertType::NONE;
  }
  }
}

std::string_view ast::ETransfertType_to_str(ETransfertType type)
{
  switch (type) {
  case ETransfertType::copy: return "cppy";
  case ETransfertType::move: return "move";
  case ETransfertType::NONE: return "NO TRANSFERT TYPE";
  }
}

ast::EOp_Other ast::ETokenStr_to_EOp_Other(std::string_view tok_str)
{
  return magic_enum::enum_cast<ast::EOp_Other>("_" + std::string(tok_str)).value_or(ast::EOp_Other::NONE);
}
