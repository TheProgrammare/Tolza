
#pragma once

#include <string>


enum class ETokenType;
using TokTy = ETokenType;


enum class EUnaryOpType {
	NONE,
	// arithmetic
	Incr,				// increment ++
	Decr,				// decrement --
	// logical
	_not,				// not !
};


[[nodiscard]] EUnaryOpType TokTy_to_EUnaryOpType(TokTy tok);

[[nodiscard]] std::string EUnaryOpType_to_str(EUnaryOpType opTy);

enum class EBinOpType {
	NONE,
	// arithmetic
	Add,				// add +
	Sub,				// substract -
	Mul,				// multiply *
	Div,				// divide /
	Mod,				// modulo %mod% not signed if divided > 0
	Quo,				// quotien %quo%
	Rem,				// remain %rem% signed with dividend
	Pow,				// power **
	Sign,				// singator +- to set sign
	Index,				// index [i]
	Slice,				// slicing [start..end]
	bSlice,				// slicing bits ~[start..end]
	// comparator
	Gre,				// greater >
	Low,				// lower <
	Gre_eq,				// greater equal >=
	Low_eq,				// lower equal <=
	_eq,				// equal ==
	_in,				// inside in
	_nin,				// not inside !in or nin
	_is,				// is
	_nis,				// not is !is or nis
	_neq,				// not equal !=
	_eqs,				// equal strictly ===
	_neqs,				// not equal strictly !==
	// logical
	_and,				// and logical and
	_nand,				// not and logical !and or nand
	_or,				// or logical or
	_xor,				// xor logical xor
	_nor,				// not or logical !or or nor
	_xnor,				// not xor logical !xor or xnor
	_b_and,				// and bitwise b_and
	_b_nand,			// not and bitwise !b_and or b_nand
	_b_or,				// or bitwise b_or
	_b_xor,				// xor bitwise b_xor
	_b_nor,				// not or bitwise !b_or or b_nor
	_b_xnor,			// not xor bitwise !b_xor or b_xnor

	ls0,				// left shift fill 0 <<[0]
	ls1,				// left shift fill 1 <<[1]
	lsa,				// left shift arithmetic (fill with MSB) <<[a]
	rs0,				// right shift fill 0 [0]>>
	rs1,				// right shift fill 1 [1]>>
	rsa,				// right shift arithmetic (fill with MSB) [a]>>
	lr,					// left rotate <<[r]
	rr,					// right rotate [r]>>
};


[[nodiscard]] EBinOpType TokTy_to_EBinOpType(TokTy tok);

[[nodiscard]] std::string EBinOpType_to_str(EBinOpType opTy);

enum class ECapability { NONE, Ref, Mut, Copy, Clone, Move };
[[nodiscard]] ECapability TokTy_to_ECapability(TokTy tok);

[[nodiscard]] ECapability deduce_type_ECapability(bool is_complex, bool is_mut);

enum class EPassMode { NONE, Mut, Ref, Copy, Clone, Move, Addr };
[[nodiscard]] EPassMode TokTy_to_EPassMode(TokTy tok);
[[nodiscard]] EPassMode str_to_EPassMode(std::string val);
[[nodiscard]] std::string EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool EPassMode_Can_Default(EPassMode passMode);


enum class EExprPassMode { NONE, Mut, Ref, Copy, Move };
[[nodiscard]] EExprPassMode TokTy_to_EExprPassMode(TokTy tok);


enum class EPrimType {
	NONE,
	boolean,
	ASCII,
	UTF32,
	str,
	text,
	iSize, i8, i16, i32, i64, i128,
	uSize, u8, u16, u32, u64, u128,
	bSize, b8, b16, b32, b64, b128,
	fSize, f32, f64, f128,
	Void,
	deci,
	udeci,
	Enum,
	Flag,
	Component,
	Role,
	Entity,
	Generic,
	Function,
	Fn_Proto,
	tuple,
	Array,
	map,
	Range,
	Iterator,
	Slice,
};

[[nodiscard]] std::string EPrimTy_to_str(EPrimType ty);

[[nodiscard]] std::string EPrimTy_to_mangle(EPrimType ty);

[[nodiscard]] EPrimType TokTy_to_EPrimType(TokTy tok);

enum class EPtrType {
	NONE,
	raw_ptr,
	unique_ptr,
	shared_ptr,
};

[[nodiscard]] EPtrType TokTy_to_EPtrType(TokTy tok);

[[nodiscard]] std::string EPtrType_to_str(EPtrType ty);

[[nodiscard]] std::string EPtrType_to_mangle(EPtrType ty);


enum class EVariableKind { NONE, Const, Let, Var };

[[nodiscard]] EVariableKind TokTy_to_EVariableKind(TokTy tok);


enum class EAssignmentType { NONE, Copy, Clone, MoveSemantic };

[[nodiscard]] EAssignmentType TokTy_to_EAssignmentType(TokTy tok);


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
	Goto_Label,
	Entity,
	Entity_Op,
	Entity_OpIndex,
	Entity_Cast,
	Entity_New,
	Entity_Del,
	Typealias,
	Generic,
};

enum class EScopeType {
	NONE,
	Export,
	Mod,
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
	Entity, Entity_Op, Entity_Cast, Entity_New,
	If,	Else, Elif,
	For,
	While, Do_While,
	Loop,
	Match, Match_Case,
};

[[nodiscard]] size_t EPrimType_to_bits(EPrimType ty);
[[nodiscard]] size_t EPrimType_to_bytes(EPrimType ty);

// [isUnsigned][num of digit]
constexpr size_t size_byte_udeci[2][30] = {
	{ 1, 1,	2, 2, 3, 3,	4, 4, 5, 5,	6, 6, 7, 7,	8, 8, 8, 9,	9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 14, 14 },
	{ 1, 1,	2, 2, 3, 3,	3, 4, 4, 5,	5, 6, 6, 7,	7, 7, 8, 8,	9,	9, 10, 10, 10, 11, 11, 12, 12, 12, 13, 13 },
};

constexpr size_t size_bit_udeci[2][30] = {
	{ 4, 7, 10, 14, 17, 20, 23, 27, 30, 34, 37, 40, 43, 47, 50, 53, 56, 60, 63, 66, 69, 73, 76, 79, 82, 86, 89, 92, 95, 98 },
	{ 5, 8, 11, 15, 18, 21, 24, 28, 31, 35, 38, 41, 44, 48, 51, 54, 57, 61, 64, 67, 70, 74, 77, 80, 83, 87, 90, 93, 96, 99 }
};


[[nodiscard]] bool is_op_handled(EPrimType src, EBinOpType op);
[[nodiscard]] bool is_cast_explicit(EPrimType src, EPrimType target);
[[nodiscard]] bool is_cast_implicit(EPrimType src, EPrimType target);

