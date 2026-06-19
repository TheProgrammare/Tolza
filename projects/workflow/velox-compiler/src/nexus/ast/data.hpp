#pragma once

#include "nexus/forward.hpp"

#include <cstdint>
#include <string_view>

namespace ast
{

enum class ENodeKind : uint8_t {
  Unknown,

  // identifiers
  Identifier,
  ID_Qualified,
  ID_Typed,

  Path_Regex,

  Root,

  Import,

  // declarations
  // globals
  Global_Variable,
  Global_Function,
  Global_Extend_Fn,
  Global_Extend_Cast,
  Global_Extend_Op_Bin,
  Global_Extend_Op_Un,
  Global_Extend_Op_Access,
  Global_Extend_Op_Transfert,
  Global_Extend_Op_Other,
  Global_Module,
  Global_Extern,
  Global_Export,
  Global_Reexport,
  Global_Enum,
  Global_Flag,
  Global_Union,
  Global_Alias_Type,
  Global_Alias_Module,
  Global_Generic,

  Enum_Field,
  Flag_Field,
  Union_Field,

  CodeBlock,

  // declarations
  // locals
  Local_Lambda,
  Local_Lambda_Capture,
  Local_Parameter,
  Local_Gen_Param_Elem,
  Local_Gen_Params,
  Local_Pattern_Element,
  Local_Pattern_Enum,
  Local_Pattern_Tuple,
  Local_Pattern_Form,
  Local_Pattern_Rule_Facet,
  Local_Pattern_Facet,
  Local_Binding,
  Local_Tuple_Destructuring,
  Local_Variable,
  Local_Capability,

  // compositional oriented paradigm
  SFM_Facet,
  SFM_Facet_Field,
  SFM_View,
  SFM_Form,
  SFM_Rule,
  SFM_Rule_Case,

  // generics
  Generic_Type,
  Generic_Cast,
  Generic_Op,
  Generic_View,
  Generic_Facet,
  Generic_Extension,
  Generic_Rule,

  // literals
  Literal_Boolean,
  Literal_Integral,
  Literal_Fixed_Point,
  Literal_Floating_Point,
  Literal_Cune,
  Literal_Rune,
  Literal_Text_Pure,
  Literal_Text_Interpolation,
  Literal_Textual_Format,
  Literal_Format_Specifier,
  Literal_Table,
  Literal_Table_Population,
  Literal_Map,
  Literal_Tuple,
  Literal_Range,
  Literal_Iterator,
  Literal_Enum,
  Literal_Structured_Data,
  Literal_Form,

  // expressions
  Expression_If_Ternary,
  Expression_Member_Access,
  Expression_Self,
  Expression_Other,
  Expression_Call,
  Expression_Call_Argument,
  Expression_Call_Rule,
  Expression_Call_Pipe,
  Expression_Table_Access,
  Expression_Ptr_Val,
  Expression_Mut_Of,
  Expression_Ref_Of,
  Expression_Move_Of,
  Expression_Copy_Of,
  Expression_Addr_Of,
  Expression_Size_Of,
  Expression_GetBits,
  Expression_New_Ptr,
  Expression_Get_Type,

  // statements
  Statement_If,
  Statement_For,
  Statement_Loop,
  Statement_While,
  Statement_GoTo,
  Statement_GoTo_Label,
  Statement_Return,
  Statement_Break,
  Statement_Continue,
  Statement_Match,
  Statement_Match_Case,

  // operations
  Operation_Cast_As,
  Operation_Is,
  Operation_In,
  Operation_Assignment,
  Operation_Binary,
  Operation_Unary,
  Operation_Interval,

  // memory
  Memory_Del,
  Memory_Align,
  Memory_Drop,
};

[[nodiscard]] inline bool ENodeKind_is_ID(ENodeKind kind)
{
  return kind >= ENodeKind::Identifier && kind <= ENodeKind::ID_Typed;
}

[[nodiscard]] inline bool ENodeKind_is_literal(ENodeKind kind)
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Literal_Form;
}

[[nodiscard]] inline bool ENodeKind_is_callable(ENodeKind kind)
{
  return kind == ENodeKind::Expression_Call || kind == ENodeKind::Global_Function || kind == ENodeKind::Global_Extend_Fn
         || kind == ENodeKind::Local_Lambda || kind == ENodeKind::SFM_Rule || kind == ENodeKind::Expression_Call_Rule;
}

[[nodiscard]] inline bool ENodeKind_is_local(ENodeKind kind)
{
  return kind >= ENodeKind::CodeBlock && kind <= ENodeKind::Local_Capability;
}

[[nodiscard]] inline bool ENodeKind_is_global(ENodeKind kind)
{
  return (kind >= ENodeKind::Global_Variable && kind <= ENodeKind::Global_Generic)
         || (kind >= ENodeKind::SFM_Facet && kind <= ENodeKind::SFM_Rule) || kind == ENodeKind::Import;
}

[[nodiscard]] inline bool ENodeKind_is_declaration(ENodeKind kind)
{
  return ENodeKind_is_local(kind) || ENodeKind_is_global(kind);
}

[[nodiscard]] inline bool ENodeKind_is_statement(ENodeKind kind)
{
  return kind >= ENodeKind::Statement_If && kind <= ENodeKind::Statement_Match_Case;
}

[[nodiscard]] inline bool ENodeKind_is_operation(ENodeKind kind)
{
  return kind >= ENodeKind::Operation_Cast_As && kind <= ENodeKind::Operation_Interval;
}

[[nodiscard]] inline bool ENodeKind_is_expression(ENodeKind kind)
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Expression_New_Ptr;
}


enum class EUnaryOpType : uint8_t {
  NONE,
  // arithmetic
  _not,         // not !
  _plus,        // as positive +
  _minus,       // as negative -
  _invert_sign, // invert sign
};

[[nodiscard]] EUnaryOpType ETokenKind_to_EUnaryOpType(token::ETokenKind tok);

[[nodiscard]] std::string_view EUnaryOpType_to_str(EUnaryOpType opTy);

enum class EAccessOpType : uint8_t {
  NONE,
  _index,       // NO BIN OP AST USED, index[i] -> T
  _index_bound, // NO BIN OP AST USED, index?[i] -> T?
  _slice,       // NO BIN OP AST USED, slicing[start..end] -> Slice<T>
  _slice_bound, // NO BIN OP AST USED, slicing?[start..end] -> Slice<T>?
  _b_slice,     // NO BIN OP AST USED, slicing bits ~[start..end]
};

[[nodiscard]] std::string_view EAccessOpType_to_str(EAccessOpType opTy);


enum class EBinOpType : uint8_t {
  NONE,
  // arithmetic
  _add,    // add +
  _sub,    // substract -
  _mul,    // multiply *
  _div,    // divide /
  _mod,    // modulo %mod% not signed if divided > 0
  _quo,    // quotien %quo%
  _rem,    // remain %rem% signed with dividend
  _divrem, // quotien + remainder in one operation %divrem%
  _pow,    // power **
  // comparator
  _ordering, // ordering <=>
  _gre,      // greater >
  _low,      // lower <
  _gre_eq,   // greater equal >=
  _low_eq,   // lower equal <=
  _eq,       // equal ==
  _in,       // inside in
  _nin,      // not inside !in or nin
  _is,       // is
  _nis,      // not is !is or nis
  _neq,      // not equal !=
  _eqs,      // equal strictly ===
  _neqs,     // not equal strictly !==
  // logical
  _and,    // and logical and
  _nand,   // not and logical
  _or,     // or logical
  _xor,    // xor logical
  _nor,    // not or logical
  _xnor,   // not xor logical
  _b_and,  // b.and bitwise
  _b_nand, // b.nand not and bitwise
  _b_or,   // b.or bitwise
  _b_xor,  // b.xor bitwise
  _b_nor,  // b.nor not or bitwise
  _b_xnor, // b.xnor not xor bitwise

  _b_shl_0, // b.shl.0 left shift fill 0
  _b_shl_1, // b.shl.1 left shift fill 1
  _b_shl_a, // b.shl.a left shift arithmetic (fill with MSB)
  _b_shr_0, // b.shr.0 right shift fill 0
  _b_shr_1, // b.shr.1 right shift fill 1
  _b_shr_a, // b.shr.a right shift arithmetic (fill with MSB)
  _b_rol,   // b.rol left rotate
  _b_ror,   // b.ror right rotate

  _mem_add,  // mem.add
  _mem_sub,  // mem.sub
  _mem_dist, // mem.dist
};

[[nodiscard]] EBinOpType ETokenKind_to_EBinOpType(token::ETokenKind tok);

[[nodiscard]] std::string_view EBinOpType_to_str(EBinOpType op);

[[nodiscard]] inline bool EBinOpType_is_logical(EBinOpType op)
{
  return op >= EBinOpType::_and && op <= EBinOpType::_xnor;
}

[[nodiscard]] inline bool EBinOpType_is_memory(EBinOpType op)
{
  return op >= EBinOpType::_mem_add && op <= EBinOpType::_mem_dist;
}

[[nodiscard]] inline bool EBinOpType_is_comparison(EBinOpType op)
{
  return op >= EBinOpType::_ordering && op <= EBinOpType::_xnor;
}

[[nodiscard]] inline bool EBinOpType_is_bitwise(EBinOpType op)
{
  return op >= EBinOpType::_b_and && op <= EBinOpType::_b_ror;
}

[[nodiscard]] inline bool EBinOpType_is_boolean(EBinOpType op)
{
  return op >= EBinOpType::_gre && op <= EBinOpType::_neq;
}

[[nodiscard]] inline bool EBinOpType_is_textual(EBinOpType op)
{
  return (op >= EBinOpType::_eq && op <= EBinOpType::_neqs) || op == EBinOpType::_add;
}

[[nodiscard]] inline bool EBinOpType_is_decimal(EBinOpType op)
{
  return op >= EBinOpType::_add && op <= EBinOpType::_neqs;
}

[[nodiscard]] inline bool EBinOpType_is_integral(EBinOpType op)
{
  return op >= EBinOpType::_add && op <= EBinOpType::_neqs;
}


enum class ECapability : uint8_t { NONE, Ref, Mut, Copy, Move };

[[nodiscard]] ECapability      ETokenKind_to_ECapability(token::ETokenKind tok);
[[nodiscard]] std::string_view ECapability_to_str(ECapability capa);
[[nodiscard]] ECapability      deduce_type_ECapability(bool is_complex, bool is_mut);


enum class EPassMode : uint8_t { NONE, Mut, Ref, Copy, Move, Addr };

[[nodiscard]] EPassMode        ETokenKind_to_EPassMode(token::ETokenKind tok);
[[nodiscard]] std::string_view EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool             EPassMode_Can_Default(EPassMode passMode);


enum class EExprPassMode : uint8_t { NONE, Mut, Ref, Copy, Move };

[[nodiscard]] EExprPassMode ETokenKind_to_EExprPassMode(token::ETokenKind tok);


enum class EVariableKind : uint8_t { NONE, Const, Let, Var };

[[nodiscard]] EVariableKind    ETokenKind_to_EVariableKind(token::ETokenKind tok);
[[nodiscard]] std::string_view EVariableKind_to_str(EVariableKind kind);


enum class ETransfertType : uint8_t { NONE, Copy, MoveSemantic };

[[nodiscard]] ETransfertType   ETokenKind_to_ETransfertType(token::ETokenKind tok);
[[nodiscard]] std::string_view ETransfertType_to_str(ETransfertType type);

enum class EOtherOp : uint8_t { NONE, del, predicat };


} // namespace ast