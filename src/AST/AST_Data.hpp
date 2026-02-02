
#pragma once

#include <unordered_map>
#include <string>
#include <optional>
#include <vector>


enum class ETokenType;
using TokTy = ETokenType;


enum class EUnaryOpType {
	None,
	// arithmetic
	Incr,				// increment ++
	Decr,				// decrement --
	// logical
	_not,				// not !
};


[[nodiscard]] EUnaryOpType TokTy_to_EUnaryOpType(TokTy tok);

[[nodiscard]] std::string EUnaryOpType_to_str(EUnaryOpType opTy);

enum class EBinOpType {
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
	bSlice,				// slicing bits :[start..end]
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

	sl0,				// shift left fill 0 <<[0]
	sl1,				// shift left fill 1 <<[1]
	sla,				// shift left arithmetic (fill with MSB) <<[a]
	sr0,				// shift right fill 0 [0]>>
	sr1,				// shift right fill 1 [1]>>
	sra,				// shift right arithmetic (fill with MSB) [a]>>
	rl,					// rotate left <<[r]
	rr,					// rotate right [r]>>
	rlc,				// rotate left carry <<[rc]
	rrc,				// rotate right carry [rc]>>

	COUNT,
};



[[nodiscard]] EBinOpType TokTy_to_EBinOpType(TokTy tok);

[[nodiscard]] std::string EBinOpType_to_str(EBinOpType opTy);

enum class ECapability { None, Ref, Mut, Copy, Clone, Move };
[[nodiscard]] ECapability TokTy_to_ECapability(TokTy tok);

[[nodiscard]] ECapability deduce_type_ECapability(bool is_complex, bool is_mut);

enum class EPassMode { None, Mut, Ref, Copy, Clone, Move, Addr };
[[nodiscard]] EPassMode TokTy_to_EPassMode(TokTy tok);
[[nodiscard]] EPassMode str_to_EPassMode(std::string val);
[[nodiscard]] std::string EPassMode_to_str(EPassMode passMode);
[[nodiscard]] bool EPassMode_Can_Default(EPassMode passMode);


enum class EExprPassMode { None, Mut, Ref, Copy, Move };
[[nodiscard]] EExprPassMode TokTy_to_EExprPassMode(TokTy tok);


enum class EPrimType {
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
	Addr,
	deci,
	udeci,
	Enum,
	Flag,
	Component,
	Entity,
	Generic,
	Function,
	tuple,
	Array,
	map,
	Range,
	Iterator,
	Slice,
	COUNT,
};

[[nodiscard]] std::string EPrimTy_to_str(EPrimType ty);

[[nodiscard]] std::string EPrimTy_to_mangle(EPrimType ty);

[[nodiscard]] EPrimType TokTy_to_EPrimType(TokTy tok);

enum class EPtrType {
	raw_ptr,
	unique_ptr,
	shared_ptr,
};

[[nodiscard]] EPtrType TokTy_to_EPtrType(TokTy tok);

[[nodiscard]] std::string EPtrType_to_mangle(EPtrType ty);


enum class EVariableKind { None, Const, Let, Var };

[[nodiscard]] EVariableKind TokTy_to_EVariableKind(TokTy tok);


enum class EAssignmentType { None, Copy, Clone, MoveSemantic };

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
	Role,
	Entity,
	Entity_Op,
	Entity_Cast,
	Entity_falseew,
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

constexpr bool op_primitive[50][43] = {
//			 bool, utf32, ascii, isize,	 i8, i16, i32, i64, i128, usize,	u8, u16, u32, u64, u128, bsize,	 b8, b16, b32, b64, b128, fsize, f32, f64, f128, addr, enum, flag, func,
/*Add*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Sub*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Mul*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Div*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Mod*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Quo*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Pow*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Incr*/	{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Decr*/	{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Sign*/	{ false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Index*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*Slice*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*Iter*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*Gre*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Low*/		{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Gre_eq*/	{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*Low_eq*/	{ false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*_eq*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, },
/*_in*/		{ false,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_nin*/	{ false,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_is*/		{ false,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_nis*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_neq*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, },
/*_eqs*/	{ false,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*_not*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_and*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_nand*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_or*/		{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_xor*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_nor*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_xnor*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*_b_and*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
/*_b_nand*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
/*_b_or*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
/*_b_xor*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
/*_b_nor*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
/*_b_xnor*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true, false, },
};

// [from][to]
constexpr bool cast_implicit[43][43] = {
//			 bool, char,	isize,	 i8, i16, i32, i64, i128, usize,	u8, u16, u32, u64, u128, bsize,	 b8, b16, b32, b64, b128, fsize, f32, f64, f128, addr, enum, flag, func,
/*bool*/	{  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*char*/	{ false,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*iSize*/	{ false, false,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*i8*/		{ false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*i16*/		{ false, false, false, false,  true,  true,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
/*i32*/		{ false, false, false, false, false,  true,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false, },
/*i64*/		{ false, false, false, false, false, false,  true,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, },
/*i128*/	{ false, false, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, },
/*uSize*/	{ false, false,  true, false, false, false,  true,  true,  true, false, false, false, false, false,  true, false, false, false,  true,  true,  true, false, false, false, false, false, false, false, },
/*u8*/		{ false, false,  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, },
/*u16*/		{ false, false, false, false, false,  true,  true,  true, false, false,  true,  true,  true,  true, false, false,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, },
/*u32*/		{ false, false, false, false, false, false,  true,  true, false, false, false,  true,  true,  true, false, false, false,  true,  true,  true, false, false,  true,  true, false, false, false, false, },
/*u64*/		{ false, false, false, false, false, false, false,  true, false, false, false, false,  true,  true, false, false, false, false,  true,  true, false, false, false,  true, false, false, false, false, },
/*u128*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, },
/*bSize*/	{ false, false, false, false, false, false, false, false,  true, false, false, false,  true, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, },
/*b8*/		{ false, false, false, false, false, false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, },
/*b16*/		{ false, false, false, false, false, false, false, false, false, false,  true,  true,  true,  true, false, false,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, },
/*b32*/		{ false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true, false, false, false,  true,  true,  true, false, false,  true,  true, false, false, false, false, },
/*b64*/		{ false, false, false, false, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false,  true,  true, false, false, false,  true, false, false, false, false, },
/*b128*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, },
/*fSize*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false, false, false, },
/*f32*/		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true,  true, false, false, false, false, },
/*f64*/		{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,  true, false, false, false, false, },
/*f128*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, },
/*address*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, },
/*enum*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false, },
/*flag*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, },
/*func*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, }
};

// [from][to]
constexpr bool cast_explicit[43][43] = {
	//			 bool, char,	isize,	 i8, i16, i32, i64, i128, usize,	u8, u16, u32, u64, u128, bsize,	 b8, b16, b32, b64, b128, fsize, f32, f64, f128, addr, enum, flag, func,
	/*bool*/	{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false,  true, false, },
	/*char*/	{ false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false,  true, false, },
	/*iSize*/	{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, },
	/*i8*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*i16*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*i32*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*i64*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*i128*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*uSize*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, },
	/*u8*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*u16*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*u32*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*u64*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*u128*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,  true,  true, false, },
	/*bSize*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false,  true,  true,  true, false, },
	/*b8*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false,  true,  true, false, },
	/*b16*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false,  true,  true, false, },
	/*b32*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false,  true,  true, false, },
	/*b64*/		{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false,  true,  true, false, },
	/*b128*/	{  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false,  true,  true, false, },
	/*fSize*/	{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
	/*f32*/		{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
	/*f64*/		{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
	/*f128*/	{  true, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, },
	/*address*/	{ false, false,  true, false, false, false, false, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, false,  true, false, false, false, },
	/*enum*/	{ false, false,  true, false, false, false, false, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, false,  true,  true,  true, false, },
	/*flag*/	{ false, false,  true, false, false, false, false, false,  true, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, false,  true,  true,  true, false, },
	/*func*/	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true, }
};

[[nodiscard]] bool is_op_handled(EPrimType src, EBinOpType op);
[[nodiscard]] bool is_cast_explicit(EPrimType src, EPrimType target);
[[nodiscard]] bool is_cast_implicit(EPrimType src, EPrimType target);

