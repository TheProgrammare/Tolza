#include "nexus/forward.hpp"

namespace ast
{

enum class ENodeKind : uint8_t {
  Unknown,

  // identifiers
  ID,
  ID_Qualified,
  ID_Typed,

  Path_Regex,

  Root,

  Import,

  // declarations
  // globals
  Global_Variable,
  Global_Function,
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

  // declarations
  // locals
  Local_CodeBlock,
  Local_Lambda,
  Local_Lambda_Capture,
  Local_Parameter,
  Local_Gen_Param_Elem,
  Local_Gen_Params,
  Local_Pattern_Element,
  Local_Pattern_Enum,
  Local_Pattern_Tuple,
  Local_Pattern_Entity,
  Local_Pattern_Sys_Comp,
  Local_Pattern_Comp,
  Local_Binding,
  Local_Tuple_Destructuring,
  Local_Variable,
  Local_Capability,

  // compositional oriented paradigm
  COP_Component,
  COP_Role,
  COP_Entity,
  COP_Entity_New,
  COP_Entity_Del,
  COP_Entity_Cast,
  COP_Entity_Op,
  COP_Entity_Access_Op,
  COP_Entity_Transfert,
  COP_System,
  COP_Component_Field,
  COP_System_Case,

  // generics
  Generic_Is_Type,
  Generic_Can_Cast,
  Generic_Have_Op,
  Generic_Have_Role,
  Generic_Use_Component,
  Generic_Compatible_System,

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
  Literal_Entity,

  // expressions
  Expression_If_Ternary,
  Expression_Member_Access,
  Expression_Self,
  Expression_Other,
  Expression_Call,
  Expression_Call_Argument,
  Expression_Call_System,
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

inline bool ENodeKind_is_ID(ENodeKind kind)
{
  return kind >= ENodeKind::ID && kind <= ENodeKind::ID_Typed;
}

inline bool ENodeKind_is_literal(ENodeKind kind)
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Literal_Entity;
}

inline bool ENodeKind_is_callable(ENodeKind kind)
{
  return kind == ENodeKind::Expression_Call || kind == ENodeKind::Global_Function || kind == ENodeKind::Local_Lambda
         || kind == ENodeKind::COP_System || kind == ENodeKind::Expression_Call_System;
}

inline bool ENodeKind_is_local(ENodeKind kind)
{
  return kind >= ENodeKind::Local_CodeBlock && kind <= ENodeKind::Local_Capability;
}

inline bool ENodeKind_is_global(ENodeKind kind)
{
  return (kind >= ENodeKind::Global_Variable && kind <= ENodeKind::Global_Generic)
         || (kind >= ENodeKind::COP_Component && kind <= ENodeKind::COP_System) || kind == ENodeKind::Import;
}

inline bool ENodeKind_is_declaration(ENodeKind kind)
{
  return ENodeKind_is_local(kind) || ENodeKind_is_global(kind);
}

inline bool ENodeKind_is_statement(ENodeKind kind)
{
  return kind >= ENodeKind::Statement_If && kind <= ENodeKind::Statement_Match_Case;
}

inline bool ENodeKind_is_operation(ENodeKind kind)
{
  return kind >= ENodeKind::Operation_Cast_As && kind <= ENodeKind::Operation_Interval;
}

inline bool ENodeKind_is_expression(ENodeKind kind)
{
  return kind >= ENodeKind::Literal_Boolean && kind <= ENodeKind::Expression_New_Ptr;
}


enum class EUnaryOpType {
  NONE,
  // arithmetic
  _not,         // not !
  _plus,        // as positive +
  _minus,       // as negative -
  _invert_sign, // invert sign
};

[[nodiscard]] EUnaryOpType ETokenKind_to_EUnaryOpType(token::ETokenKind tok);

[[nodiscard]] std::string_view EUnaryOpType_to_str(EUnaryOpType opTy);

enum class EAccessOpType {
  Index,      // NO BIN OP AST USED, index[i] -> T
  IndexBound, // NO BIN OP AST USED, index?[i] -> T?
  Slice,      // NO BIN OP AST USED, slicing[start..end] -> Slice<T>
  SliceBound, // NO BIN OP AST USED, slicing?[start..end] -> Slice<T>?
  bSlice,     // NO BIN OP AST USED, slicing bits ~[start..end]
};

[[nodiscard]] std::string_view EAccessOpType_to_str(EAccessOpType opTy);


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

inline bool EBinOpType_is_logical(EBinOpType op)
{
  return op >= EBinOpType::_and && op <= EBinOpType::_xnor;
}

inline bool EBinOpType_is_memory(EBinOpType op)
{
  return op >= EBinOpType::_mem_add && op <= EBinOpType::_mem_dist;
}

inline bool EBinOpType_is_comparison(EBinOpType op)
{
  return op >= EBinOpType::Gre && op <= EBinOpType::_xnor;
}

inline bool EBinOpType_is_bitwise(EBinOpType op)
{
  return op >= EBinOpType::_b_and && op <= EBinOpType::_b_ror;
}

inline bool EBinOpType_is_boolean(EBinOpType op)
{
  return op >= EBinOpType::_eq && op <= EBinOpType::_neq;
}

inline bool EBinOpType_is_textual(EBinOpType op)
{
  return (op >= EBinOpType::_eq && op <= EBinOpType::_neqs) || op == EBinOpType::Add;
}

inline bool EBinOpType_is_decimal(EBinOpType op)
{
  return op >= EBinOpType::Add && op <= EBinOpType::_neqs;
}

inline bool EBinOpType_is_integral(EBinOpType op)
{
  return op >= EBinOpType::Add && op <= EBinOpType::_neqs;
}


enum class ECapability { NONE, Ref, Mut, Copy, Move };

[[nodiscard]] ECapability      ETokenKind_to_ECapability(token::ETokenKind tok);
[[nodiscard]] std::string_view ECapability_to_str(ECapability capa);
[[nodiscard]] ECapability      deduce_type_ECapability(bool is_complex, bool is_mut);


enum class EPassMode { NONE, Mut, Ref, Copy, Move, Addr };

[[nodiscard]] EPassMode        ETokenKind_to_EPassMode(token::ETokenKind tok);
[[nodiscard]] std::string_view EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool             EPassMode_Can_Default(EPassMode passMode);


enum class EExprPassMode { NONE, Mut, Ref, Copy, Move };

[[nodiscard]] EExprPassMode ETokenKind_to_EExprPassMode(token::ETokenKind tok);


enum class EVariableKind { NONE, Const, Let, Var };

[[nodiscard]] EVariableKind    ETokenKind_to_EVariableKind(token::ETokenKind tok);
[[nodiscard]] std::string_view EVariableKind_to_str(EVariableKind kind);


enum class ETransfertType { NONE, Copy, MoveSemantic };

[[nodiscard]] ETransfertType   ETokenKind_to_ETransfertType(token::ETokenKind tok);
[[nodiscard]] std::string_view ETransfertType_to_str(ETransfertType type);


enum class EPathSource { NONE, Self, Super, Root };

[[nodiscard]] EPathSource      ETokenKind_to_EPathSource(token::ETokenKind tok);
[[nodiscard]] std::string_view EPathSource_to_str(EPathSource type);

} // namespace ast