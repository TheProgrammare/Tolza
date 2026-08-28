#pragma once

#include "nexus/forward.hpp"

#include <cstdint>
#include <string_view>

namespace ast
{

enum class ENodeKind : uint8_t {
  NONE,

  // identifiers

  Symbol_Id,
  Symbol_Qualified,
  Symbol_Type,

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
  Global_Extend_Op_Subscript,
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
  Literal_NullPtr,
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
  Literal_Record,

  // expressions
  Expression_If_Ternary,
  Expression_Member_Access,
  Expression_Self,
  Expression_Other,
  Expression_Invocation,
  Expression_Invocation_Arg,
  Expression_Invocation_Extend,
  Expression_Invocation_Rule,
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
  Operation_Transfert,
  Operation_Binary,
  Operation_Unary,
  Operation_Interval,

  // memory
  Memory_Del,
  Memory_Align,
  Memory_Drop,
};

[[nodiscard]] inline bool ENodeKind_is_symbol(ENodeKind kind)
{
  return kind >= ENodeKind::Symbol_Id && kind <= ENodeKind::Symbol_Type;
}

[[nodiscard]] inline bool ENodeKind_is_literal(ENodeKind kind)
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Literal_Record;
}

[[nodiscard]] inline bool ENodeKind_is_callable(ENodeKind kind)
{
  return kind == ENodeKind::Expression_Invocation || kind == ENodeKind::Global_Function
         || kind == ENodeKind::Global_Extend_Fn || kind == ENodeKind::Local_Lambda || kind == ENodeKind::SFM_Rule
         || kind == ENodeKind::Expression_Invocation_Rule;
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


enum class EOp_Unary : uint8_t {
  NONE,
  // arithmetic
  _not,         // not !
  _plus,        // as positive +
  _minus,       // as negative -
  _invert_sign, // invert sign
};

[[nodiscard]] EOp_Unary ETokenKind_to_EOp_Unary(token::ETokenKind tok);

[[nodiscard]] std::string_view EOp_Unary_to_str(EOp_Unary opTy);

enum class EOp_Subscript : uint8_t {
  NONE,
  _index,       // index[i] -> T
  _index_bound, // index?[i] -> T?
  _slice,       // slicing[start..end] -> Slice<T>
  _slice_bound, // slicing?[start..end] -> Slice<T>?
  _b_index,     // index bit ~[i] -> bool
  _b_slice,     // slicing bits ~[start..end] -> bsize
};

[[nodiscard]] std::string_view EOp_Subscript_to_str(EOp_Subscript opTy);


enum class EOp_Bin : uint8_t {
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

[[nodiscard]] EOp_Bin ETokenKind_to_EOp_Bin(token::ETokenKind tok);

[[nodiscard]] std::string_view EOp_Bin_to_str(EOp_Bin op);

[[nodiscard]] inline bool EOp_Bin_is_logical(EOp_Bin op)
{
  return op >= EOp_Bin::_and && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_memory(EOp_Bin op)
{
  return op >= EOp_Bin::_mem_add && op <= EOp_Bin::_mem_dist;
}

[[nodiscard]] inline bool EOp_Bin_is_comparison(EOp_Bin op)
{
  return op >= EOp_Bin::_ordering && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_bitwise(EOp_Bin op)
{
  return op >= EOp_Bin::_b_and && op <= EOp_Bin::_b_ror;
}

[[nodiscard]] inline bool EOp_Bin_is_boolean(EOp_Bin op)
{
  return op >= EOp_Bin::_ordering && op <= EOp_Bin::_xnor;
}

[[nodiscard]] inline bool EOp_Bin_is_textual(EOp_Bin op)
{
  return (op >= EOp_Bin::_eq && op <= EOp_Bin::_neqs) || op == EOp_Bin::_add;
}

[[nodiscard]] inline bool EOp_Bin_is_decimal(EOp_Bin op)
{
  return op >= EOp_Bin::_add && op <= EOp_Bin::_neqs;
}

[[nodiscard]] inline bool EOp_Bin_is_integral(EOp_Bin op)
{
  return op >= EOp_Bin::_add && op <= EOp_Bin::_neqs;
}

enum class EInvocationKind : uint8_t { NONE, fn_call, enum_bind, pattern };

// name op like : _name
enum class EOp_Other : uint8_t { NONE, _del, _predicat };
[[nodiscard]] EOp_Other ETokenStr_to_EOp_Other(std::string_view tok_str);


enum class ECapability : uint8_t { NONE, ref, mut, copy, move };

[[nodiscard]] ECapability      ETokenKind_to_ECapability(token::ETokenKind tok);
[[nodiscard]] std::string_view ECapability_to_str(ECapability capa);
[[nodiscard]] ECapability      deduce_type_ECapability(bool is_complex, bool is_mut);


enum class EPassMode : uint8_t { NONE, mut, ref, copy, move, addr };

[[nodiscard]] EPassMode        ETokenKind_to_EPassMode(token::ETokenKind tok);
[[nodiscard]] std::string_view EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool             EPassMode_Can_Default(EPassMode passMode);


enum class EExprPassMode : uint8_t { NONE, mut, ref, copy, move };

[[nodiscard]] EExprPassMode ETokenKind_to_EExprPassMode(token::ETokenKind tok);


enum class EVariableKind : uint8_t { NONE, _const, _let, _var };

[[nodiscard]] EVariableKind    ETokenKind_to_EVariableKind(token::ETokenKind tok);
[[nodiscard]] std::string_view EVariableKind_to_str(EVariableKind kind);


enum class ETransfertType : uint8_t { NONE, copy, move };

[[nodiscard]] ETransfertType   ETokenKind_to_ETransfertType(token::ETokenKind tok);
[[nodiscard]] std::string_view ETransfertType_to_str(ETransfertType type);


} // namespace ast