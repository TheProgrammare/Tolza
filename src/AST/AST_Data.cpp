#include "AST_Data.hpp"

#include <system_error>

#include "Globals.hpp"
#include "Lexer/Token.hpp"

EBinOpType TokTy_to_EBinOpType(TokTy tok)
{
	switch (tok)
	{
		case TokTy::OP_PLUS:				return EBinOpType::Add;
		case TokTy::OP_MINUS:				return EBinOpType::Sub;
		case TokTy::OP_ASTERISK:			return EBinOpType::Mul;
		case TokTy::OP_DIVIDE:				return EBinOpType::Div;
		case TokTy::OP_MODULO:				return EBinOpType::Mod;
		case TokTy::OP_QUOTIEN:				return EBinOpType::Quo;
		case TokTy::OP_REMAIN:				return EBinOpType::Rem;
		case TokTy::OP_POWER:				return EBinOpType::Pow;
		case TokTy::SIGNATOR:				return EBinOpType::Sign;
		case TokTy::IN:						return EBinOpType::_in;
		case TokTy::NIN:					return EBinOpType::_nin;
		case TokTy::IS:						return EBinOpType::_is;
		case TokTy::NIS:					return EBinOpType::_nis;
		case TokTy::CLOSE_BRACKETS:			return EBinOpType::Gre;
		case TokTy::OPEN_BRACKETS:			return EBinOpType::Low;
		case TokTy::OP_GREATER_EQUAL:		return EBinOpType::Gre_eq;
		case TokTy::OP_LOWER_EQUAL:			return EBinOpType::Low_eq;
		case TokTy::OP_EQUAL:				return EBinOpType::_eq;
		case TokTy::OP_EQUAL_STRICTLY:		return EBinOpType::_eqs;
		case TokTy::OP_NOT_EQUAL:			return EBinOpType::_neq;
		case TokTy::OP_NOT_EQUAL_STRICTLY:	return EBinOpType::_neqs;
		case TokTy::AND:					return EBinOpType::_and;
		case TokTy::B_AND:					return EBinOpType::_b_and;
		case TokTy::NAND:					return EBinOpType::_nand;
		case TokTy::B_NAND:					return EBinOpType::_b_nand;
		case TokTy::OR:						return EBinOpType::_or;
		case TokTy::B_OR:					return EBinOpType::_b_or;
		case TokTy::XOR:					return EBinOpType::_xor;
		case TokTy::B_XOR:					return EBinOpType::_b_xor;
		case TokTy::NOR:					return EBinOpType::_nor;
		case TokTy::B_NOR:					return EBinOpType::_b_nor;
		case TokTy::XNOR:					return EBinOpType::_xnor;
		case TokTy::B_XNOR:					return EBinOpType::_b_xnor;
		case TokTy::SHIFT_LEFT_0:			return EBinOpType::sl0;
		case TokTy::SHIFT_LEFT_1:			return EBinOpType::sl1;
		case TokTy::SHIFT_LEFT_A:			return EBinOpType::sla;
		case TokTy::SHIFT_RIGHT_0:			return EBinOpType::sr0;
		case TokTy::SHIFT_RIGHT_1:			return EBinOpType::sr1;
		case TokTy::SHIFT_RIGHT_A:			return EBinOpType::sra;
		case TokTy::ROTATE_LEFT:			return EBinOpType::rl;
		case TokTy::ROTATE_RIGHT:			return EBinOpType::rr;
		case TokTy::ROTATE_LEFT_CARRY:		return EBinOpType::rlc;
		case TokTy::ROTATE_RIGHT_CARRY:		return EBinOpType::rrc;
		default:
			throw std::runtime_error("Token '" + std::to_string(int(tok)) + "' is not an operator type.");
	}
}

std::string EBinOpType_to_str(EBinOpType opTy)
{
	switch (opTy)
	{
		case EBinOpType::Add:				return "+";
		case EBinOpType::Sub:				return "-";
		case EBinOpType::Mul:				return "*";
		case EBinOpType::Div:				return "/";
		case EBinOpType::Mod:				return "%mod%";
		case EBinOpType::Quo:				return "%quo%";
		case EBinOpType::Rem:				return "%rem%";
		case EBinOpType::Pow:				return "**";
		case EBinOpType::Sign:				return "+-";
		case EBinOpType::_in:				return "in";
		case EBinOpType::_nin:				return "!in";
		case EBinOpType::_is:				return "is";
		case EBinOpType::_nis:				return "!is";
		case EBinOpType::Gre:				return ">";
		case EBinOpType::Low:				return "<";
		case EBinOpType::Gre_eq:			return ">=";
		case EBinOpType::Low_eq:			return "<=";
		case EBinOpType::_eq:				return "==";
		case EBinOpType::_eqs:				return "===";
		case EBinOpType::_neqs:				return "!==";
		case EBinOpType::_neq:				return "!=";
		case EBinOpType::_and:				return "and";
		case EBinOpType::_b_and:			return "and.b";
		case EBinOpType::_nand:				return "nand";
		case EBinOpType::_b_nand:			return "nand.b";
		case EBinOpType::_or:				return "or";
		case EBinOpType::_b_or:				return "or.b";
		case EBinOpType::_xor:				return "xor";
		case EBinOpType::_b_xor:			return "xor.b";
		case EBinOpType::_nor:				return "nor";
		case EBinOpType::_b_nor:			return "nor.b";
		case EBinOpType::_xnor:				return "xnor";
		case EBinOpType::_b_xnor:			return "xnor.b";
		default:
			throw std::runtime_error("Expected operator type token");
	}

	return "";
}

ECapability TokTy_to_ECapability(TokTy tok)
{
	switch (tok)
	{
		case TokTy::CAPA_MUT: 			return ECapability::Mut;
		case TokTy::CAPA_REF:			return ECapability::Ref;
		case TokTy::CAPA_COPY:			return ECapability::Copy;
		case TokTy::CAPA_MOVE:			return ECapability::Move;
		case TokTy::CAPA_CLONE:			return ECapability::Clone;
		default:
			return ECapability::None;
	}
}

ECapability deduce_type_ECapability(bool is_complex, bool is_mut)
{
	if (is_mut) return ECapability::Mut;
	if (is_complex) return ECapability::Move;
	if (!is_complex) return ECapability::Copy;
}

EUnaryOpType TokTy_to_EUnaryOpType(TokTy tok)
{
	switch (tok)
	{
		case TokTy::NOT: 					return EUnaryOpType::_not;
		case TokTy::OP_INCREMENT: 			return EUnaryOpType::Incr;
		case TokTy::OP_DECREMENT:			return EUnaryOpType::Decr;
		default:
			return EUnaryOpType::None;
	}
}

std::string EUnaryOpType_to_str(EUnaryOpType opTy)
{
	switch (opTy) {
		case EUnaryOpType::_not:			return "!";
		case EUnaryOpType::Incr:			return "++";
		case EUnaryOpType::Decr:			return "--";		
	}
}

EPassMode TokTy_to_EPassMode(TokTy tok)
{
	switch (tok)
	{
		case TokTy::CAPA_MUT:	return EPassMode::Mut;
		case TokTy::CAPA_REF:	return EPassMode::Ref;
		case TokTy::CAPA_COPY:	return EPassMode::Copy;
		case TokTy::CAPA_CLONE:	return EPassMode::Clone;
		case TokTy::CAPA_MOVE:	return EPassMode::Move;
		case TokTy::ADDR:	return EPassMode::Addr;
		default:
			return EPassMode::None;
	}
}

EPassMode str_to_EPassMode(std::string val) {
	if (val == "move")		return EPassMode::Move;
	if (val == "mut")		return EPassMode::Mut;
	if (val == "copy")		return EPassMode::Copy;
	if (val == "clone")		return EPassMode::Clone;
	if (val == "addr")		return EPassMode::Addr;
	if (val == "ref")		return EPassMode::Ref;
	return EPassMode::None;
}

std::string EPassMode_to_str(EPassMode passMode)
{
	switch (passMode)
	{
	case EPassMode::Mut:			 return "mut";
	case EPassMode::Ref:			 return "ref";
	case EPassMode::Copy:			 return "copy";
	case EPassMode::Clone:			 return "clone";
	case EPassMode::Move:			 return "move";
	case EPassMode::Addr:			 return "addr";
	default:
		throw std::runtime_error("Fatal Error, Unexpected pass mode non existential");
	}
}

bool EPassMode_Can_Default(EPassMode passMode)
{
	switch (passMode)
	{
	case EPassMode::Mut:			 return false;
	case EPassMode::Ref:			 return true;
	case EPassMode::Copy:			 return true;
	case EPassMode::Clone:			 return true;
	case EPassMode::Move:			 return false;
	case EPassMode::Addr:			 return false;
	default:
		throw std::runtime_error("Fatal Error, Unexpected pass mode non existential");
	}
}

EExprPassMode TokTy_to_EExprPassMode(TokTy tok)
{
	switch (tok)
	{
		case TokTy::CAPA_MUT_OF:		return EExprPassMode::Mut;
		case TokTy::CAPA_REF_OF:		return EExprPassMode::Ref;
		case TokTy::CAPA_COPY_OF:	return EExprPassMode::Copy;
		case TokTy::CAPA_MOVE_OF:	return EExprPassMode::Move;
		default:
			return EExprPassMode::None;
	}
}

std::string EPrimTy_to_str(EPrimType ty)
{
	switch (ty)
	{
		case EPrimType::boolean:	return "bool";
		case EPrimType::UTF32:	return "char";
		case EPrimType::iSize:		return "isize";
		case EPrimType::i8:			return "i8";
		case EPrimType::i16:		return "i16";
		case EPrimType::i32:		return "i32";
		case EPrimType::i64:		return "i64";
		case EPrimType::i128:		return "i128";
		case EPrimType::uSize:		return "usize";
		case EPrimType::u8:			return "u8";
		case EPrimType::u16:		return "u16";
		case EPrimType::u32:		return "u32";
		case EPrimType::u64:		return "u64";
		case EPrimType::u128:		return "u128";
		case EPrimType::bSize:		return "bsize";
		case EPrimType::b8:			return "b8";
		case EPrimType::b16:		return "b16";
		case EPrimType::b32:		return "b32";
		case EPrimType::b64:		return "b64";
		case EPrimType::b128:		return "b128";
		case EPrimType::fSize:		return "fsize";
		case EPrimType::f32:		return "f32";
		case EPrimType::f64:		return "f64";
		case EPrimType::f128:		return "f128";
		case EPrimType::Void:		return "void";
		case EPrimType::Enum:		return "enum";
		case EPrimType::Flag:		return "flag";
		case EPrimType::Entity:		return "entity";
		case EPrimType::Function:	return "function";
		case EPrimType::str:		return "static_string";
		case EPrimType::text:		return "static_text";
		case EPrimType::COUNT:		return "INVALID";
		default:
			throw std::runtime_error("Expected type token");
	}
}

std::string EPrimTy_to_mangle(EPrimType ty)
{
	switch (ty)
	{
		case EPrimType::boolean:	return "b";
		case EPrimType::UTF32:	return "c";
		case EPrimType::iSize:		return "isz";
		case EPrimType::i8:			return "i8";
		case EPrimType::i16:		return "i16";
		case EPrimType::i32:		return "i32";
		case EPrimType::i64:		return "i64";
		case EPrimType::i128:		return "i128";
		case EPrimType::uSize:		return "usz";
		case EPrimType::u8:			return "u8";
		case EPrimType::u16:		return "u16";
		case EPrimType::u32:		return "u32";
		case EPrimType::u64:		return "u64";
		case EPrimType::u128:		return "u128";
		case EPrimType::bSize:		return "bsz";
		case EPrimType::b8:			return "b8";
		case EPrimType::b16:		return "b16";
		case EPrimType::b32:		return "b32";
		case EPrimType::b64:		return "b64";
		case EPrimType::b128:		return "b128";
		case EPrimType::fSize:		return "fsz";
		case EPrimType::f32:		return "f32";
		case EPrimType::f64:		return "f64";
		case EPrimType::f128:		return "f128";
		case EPrimType::Void:		return "void";
		case EPrimType::Enum:		return "en";
		case EPrimType::Flag:		return "fl";
		case EPrimType::Entity:		return "et";
		case EPrimType::Function:	return "fn";
		case EPrimType::COUNT:		return "INVALID";
		default:
			throw std::runtime_error("Expected type token");
	}
}

EPrimType TokTy_to_EPrimType(TokTy tok)
{
	switch (tok)
	{
		case TokTy::T_BOOL:				return EPrimType::boolean;
		case TokTy::T_UTF32:			return EPrimType::UTF32;
		case TokTy::T_ASCII:			return EPrimType::ASCII;
		case TokTy::T_STRING:			return EPrimType::str;
		case TokTy::T_TEXT:				return EPrimType::text;
		case TokTy::ADDR:				return EPrimType::Addr;

		case TokTy::T_ISIZE:			return EPrimType::iSize;
		case TokTy::T_I8:				return EPrimType::i8;
		case TokTy::T_I16:				return EPrimType::i16;
		case TokTy::T_I32:				return EPrimType::i32;
		case TokTy::T_I64:				return EPrimType::i64;
		case TokTy::T_I128:				return EPrimType::i128;

		case TokTy::T_USIZE:			return EPrimType::uSize;
		case TokTy::T_U8:				return EPrimType::u8;
		case TokTy::T_U16:				return EPrimType::u16;
		case TokTy::T_U32:				return EPrimType::u32;
		case TokTy::T_U64:				return EPrimType::u64;
		case TokTy::T_U128:				return EPrimType::u128;

		case TokTy::T_BSIZE:			return EPrimType::bSize;
		case TokTy::T_B8:				return EPrimType::b8;
		case TokTy::T_B16:				return EPrimType::b16;
		case TokTy::T_B32:				return EPrimType::b32;
		case TokTy::T_B64:				return EPrimType::b64;
		case TokTy::T_B128:				return EPrimType::b128;

		case TokTy::T_FSIZE:			return EPrimType::fSize;
		case TokTy::T_F32:				return EPrimType::f32;
		case TokTy::T_F64:				return EPrimType::f64;
		case TokTy::T_F128:				return EPrimType::f128;
		
		case TokTy::T_VOID:				return EPrimType::Void;

		case TokTy::ENUM:				return EPrimType::Enum;
		case TokTy::FLAG:				return EPrimType::Flag;
		case TokTy::COMPONENT:			return EPrimType::Component;
		case TokTy::ENTITY:				return EPrimType::Entity;
		case TokTy::GENERIC:			return EPrimType::Generic;
		case TokTy::FUNCTION:			return EPrimType::Function;

		default:
			throw std::runtime_error("Expected primitive type token");
	}
}

EPtrType TokTy_to_EPtrType(TokTy tok)
{
	switch (tok)
	{
		case TokTy::PTR:				return EPtrType::raw_ptr;
		case TokTy::UPTR:				return EPtrType::unique_ptr;
		case TokTy::SPTR:				return EPtrType::shared_ptr;
		default:
			throw std::runtime_error("Expected pointer type token");
	}
}

std::string EPtrType_to_mangle(EPtrType ty)
{
	switch (ty)
	{
		case EPtrType::raw_ptr:			 return "p";
		case EPtrType::unique_ptr:		 return "up";
		case EPtrType::shared_ptr:		 return "sp";
		default:
			throw std::runtime_error("Expected ptr token");
	}
}

EVariableKind TokTy_to_EVariableKind(TokTy tok) {
    switch (tok)
	{
	case TokTy::LET:					return EVariableKind::Let;
	case TokTy::VAR:					return EVariableKind::Var;
	case TokTy::CONST:					return EVariableKind::Const;
	default:
		return EVariableKind::None;
	}
}

EAssignmentType TokTy_to_EAssignmentType(TokTy tok)
{
	switch (tok)
	{
		case TokTy::ASSIGN:					return EAssignmentType::MoveSemantic;
		case TokTy::COPY_ASSIGN:			return EAssignmentType::Copy;
		case TokTy::CLONE_ASSIGN:			return EAssignmentType::Clone;
		case TokTy::MOVE_ASSIGN:			return EAssignmentType::MoveSemantic;
		default:
			// for operator assignment
			return EAssignmentType::None;
	}
}



bool is_op_handled(EPrimType src, EBinOpType op) {
	return op_primitive[static_cast<uint8_t>(op)][static_cast<uint8_t>(src)];
}

bool is_cast_explicit(EPrimType src, EPrimType target) {
	return cast_explicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
}

bool is_cast_implicit(EPrimType src, EPrimType target) {
	return cast_implicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
}
