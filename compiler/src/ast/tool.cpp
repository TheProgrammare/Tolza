#include "ast/tool.hpp"

#include "ast/data.hpp"
#include "ast/forward.hpp"
#include "ast/node/declaration_extension.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/literal.hpp"
#include "id/base.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "lexer/data.hpp"
#include "nexus/forward.hpp"
#include "type/forward.hpp"
#include "type/type.hpp"

#include <cassert>
#include <string_view>
#include <vector>


std::string_view ast::EOp_Subscript_to_sym(EOp_Subscript opTy) noexcept
{
  switch (opTy) {
  case EOp_Subscript::_index:       return "[i]";
  case EOp_Subscript::_index_bound: return "?[i]";
  case EOp_Subscript::_slice:       return "[start..end]";
  case EOp_Subscript::_slice_bound: return "?[start..end]";
  case EOp_Subscript::_b_slice:     return "~[start..end]";
  case EOp_Subscript::_b_index:     return "~[i]";
  case EOp_Subscript::NONE:         return {};
  };
}

ast::EOp_Bin ast::ETokenKind_to_EOp_Bin(token::ETokenKind tok) noexcept
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

std::string_view ast::EOp_Bin_to_sym(EOp_Bin opTy) noexcept
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

ast::ECapability ast::ETokenKind_to_ECapability(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return ECapability::mut;
  case token::ETokenKind::CAPA_REF:  return ECapability::ref;
  case token::ETokenKind::CAPA_COPY: return ECapability::copy;
  case token::ETokenKind::CAPA_MOVE: return ECapability::move;
  default:                           return ECapability::NONE;
  }
}

ast::ECapability ast::deduce_type_ECapability(bool is_complex, bool is_mut) noexcept
{
  if (is_mut) return ECapability::mut;
  if (is_complex) return ECapability::move;
  return ECapability::copy;
}


ast::EOp_Unary ast::ETokenKind_to_EOp_Unary(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::EXCLAMATION:
  case token::ETokenKind::OP_NOT:      return EOp_Unary::_not;
  case token::ETokenKind::OP_PLUS:     return EOp_Unary::_plus;
  case token::ETokenKind::OP_MINUS:    return EOp_Unary::_minus;
  default:                             return EOp_Unary::NONE;
  }
}

std::string_view ast::EOp_Unary_to_sym(EOp_Unary opTy) noexcept
{
  switch (opTy) {
  case EOp_Unary::_not:   return "!";
  case EOp_Unary::_plus:  return "+";
  case EOp_Unary::_minus: return "-";
  default:                return {};
  }
}

ast::EPassMode ast::ETokenKind_to_EPassMode(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EPassMode::mut;
  case token::ETokenKind::CAPA_REF:  return EPassMode::ref;
  case token::ETokenKind::CAPA_COPY: return EPassMode::copy;
  case token::ETokenKind::CAPA_MOVE: return EPassMode::move;
  case token::ETokenKind::ADDR:      return EPassMode::addr;
  case token::ETokenKind::CONST:     return EPassMode::_const;
  default:                           return EPassMode::NONE;
  }
}

bool ast::EPassMode_Can_Default(EPassMode passMode) noexcept
{
  switch (passMode) {
  case EPassMode::mut:
  case EPassMode::addr:
  case EPassMode::move:   return false;
  case EPassMode::ref:
  case EPassMode::copy:
  case EPassMode::_const:
  case EPassMode::NONE:   return true;
  }
}


ast::EExprPassMode ast::ETokenKind_to_EExprPassMode(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::CAPA_MUT:  return EExprPassMode::mut;
  case token::ETokenKind::CAPA_REF:  return EExprPassMode::ref;
  case token::ETokenKind::CAPA_COPY: return EExprPassMode::copy;
  case token::ETokenKind::CAPA_MOVE: return EExprPassMode::move;
  default:                           return EExprPassMode::NONE;
  }
}


ast::EVariableKind ast::ETokenKind_to_EVariableKind(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::LET:   return EVariableKind::_let;
  case token::ETokenKind::VAR:   return EVariableKind::_var;
  case token::ETokenKind::CONST: return EVariableKind::_const;
  default:                       return EVariableKind::NONE;
  }
}

ast::ETransfertType ast::ETokenKind_to_ETransfertType(token::ETokenKind tok) noexcept
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

ast::EOp_Other ast::ETokenStr_to_EOp_Other(std::string_view tok_str) noexcept
{
  if (tok_str == "del") return EOp_Other::_del;
  if (tok_str == "predicat") return EOp_Other::_predicat;
  return EOp_Other::NONE;
}

std::vector<ast::ID> ast::get_parameters(ast::ID nodeid) noexcept
{
#define if_get_params(kind)                                                                                            \
  if (const auto* n = nodeid.as<ast::kind>()) return n->parameters;

  if_get_params(Global_Function);
  if_get_params(Local_Lambda);
  if_get_params(SFM_Rule);

  return {};

#undef if_get_params
}

ast::EVisibility ast::get_decl_visibility(ast::ID nodeid) noexcept
{
#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()->visibility

  switch (nodeid.kind()) {
    case_n(Global_Variable);
    case_n(Global_Function);
    case_n(Global_Extend_Fn);
    case_n(Global_Extend_Cast);
    case_n(Global_Extend_Op_Bin);
    case_n(Global_Extend_Op_Un);
    case_n(Global_Extend_Op_Subscript);
    case_n(Global_Extend_Op_Transfert);
    case_n(Global_Extend_Op_Other);
    case_n(Global_Module);
    case_n(Global_Enum);
    case_n(Global_Flag);
    case_n(Global_Union);
    case_n(Global_Alias_Type);
    case_n(Global_Alias_Module);
    case_n(Global_Generic);
    case_n(SFM_Facet);
    case_n(SFM_View);
    case_n(SFM_Form);
    case_n(SFM_Rule);
  default: assert(false && "Must be used on declaration node with a visibility");
  }

#undef case_n
}

type::ID ast::get_decl_type(ast::ID nodeid) noexcept
{
#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()

  switch (nodeid.kind()) {
    case_n(Global_Variable)->type;
    case_n(Global_Function)->prototype;
    case_n(Global_Alias_Type)->type;
    case_n(Local_Lambda)->prototype;
    case_n(Local_Parameter)->type;
    case_n(Local_Binding)->type;
    case_n(Local_Variable)->type;
    case_n(Local_Capability)->type;
  default: assert(false && "Must be used on named node or with alias");
  }

#undef case_n
}

ast::ID ast::get_value(ID nodeid) noexcept
{
#define case_n(_kind)                                                                                                  \
  case ENodeKind::_kind: return nodeid.as<ast::_kind>()

  const auto kind = nodeid.kind();

  if (ast::ENodeKind_is_literal(kind)) return nodeid;

  switch (kind) {
    case_n(Local_Binding)->expression;
    case_n(Local_Variable)->expression;
    case_n(Local_Capability)->expression;
    case_n(Local_Tuple_Destructuring)->expression;
    case_n(Expression_Invocation_Arg)->expression;
    case_n(Expression_New_Ptr)->expression;
    case_n(Expression_Table_Access)->target;
    case_n(Global_Variable)->expression;
    case_n(Local_Pattern_Element)->literal;
    case_n(Local_Pattern_Enum)->expression;
    case_n(Local_Pattern_Facet)->expression;
    case_n(Local_Pattern_Form)->expression;
    case_n(Local_Pattern_Tuple)->expression;
    case_n(Local_Pattern_Rule_Facet)->expression;
    case_n(Local_Parameter)->default_value;
  default: {
    auto def = nodeid.def();
    if (def) return def.node().value();

    return NO_ID;
  }
  }
}


bool ast::is_table_population(ID nodeid) noexcept
{
  const auto* ptr = nodeid.as<ast::Literal_Table>();
  if (!ptr) return false;

  if (!ptr->ranges.empty()) return true;

  if (!ptr->values.empty()) {
    auto val = ptr->values[0][0];
    return val.is_inferred() && val.type().is<type::Range>();
  }

  return false;
}
bool ast::is_map(ID nodeid) noexcept
{
  const auto* ptr = nodeid.as<ast::Literal_Table>();
  if (!ptr) return false;

  return !ptr->map_value.empty();
}


const type::Prototype* ast::get_prototype(ID id) noexcept
{
#define if_get_proto(kind)                                                                                             \
  if (const auto* ptr = id.as<ast::kind>()) return ptr->prototype.as<type::Prototype>();

  assert(id && "Invalid id");
  if_get_proto(Global_Function);
  if_get_proto(SFM_Rule);
  if_get_proto(Global_Extend_Fn);
  if_get_proto(Local_Lambda);

  assert(false && "No prototyped node");

  return NO_ID;

#undef if_get_proto
}
