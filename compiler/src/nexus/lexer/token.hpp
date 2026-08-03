#pragma once

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "nexus/forward.hpp"


namespace token
{

constexpr uint32_t INVALID_POS = -1;
constexpr uint16_t INVALID_LEN = -1;

// T_ = Type L_ = Literal S_ = Special (no text key representation)
enum class ETokenKind : uint8_t {
  UNKNOWN,
  S_END_OF_FILE,
  // end of metacode (end of line)
  S_METACODE_END,
  S_METACODE_EXPAND_IF,
  S_METACODE_PLACEHOLDER,
  MOD,
  SUPER_MOD,
  SELF,
  IDENTIFIER,
  OPERATOR,
  DELIMITER,
  // types
  TYPE,
  DOLLAR,
  INTERROGATIVE,
  EXCLAMATION,
  T_U0,
  T_BOOL,
  // 8 bits
  T_CUNE,
  // 32 bits
  T_RUNE,
  T_S8,
  T_S16,
  T_S32,
  T_S64,
  T_S128,
  T_SSIZE,
  T_U8,
  T_U16,
  T_U32,
  T_U64,
  T_U128,
  T_USIZE,
  T_B8,
  T_B16,
  T_B32,
  T_B64,
  T_B128,
  T_BSIZE,
  T_PTRDIFF,
  T_F16,
  T_F32,
  T_F64,
  T_F80,
  T_F128,
  T_FSIZE,
  T_D32,
  T_D64,
  T_D128,
  T_DSIZE,
  T_UD32,
  T_UD64,
  T_UD128,
  T_UDSIZE,
  T_OPAQUE,
  // C string convention: i8* null terminated
  T_CSTR,
  // fat pointer of cune null terminated
  T_STR,
  // fat pointer of rune null terminated
  T_TEXT,
  T_ARRAY,
  T_TUPLE,
  HASHTAG,
  AT,
  // literal values
  // Literal binary
  L_BIN,
  // Literal octal
  L_OCT,
  // Literal hexadecimal
  L_HEX,
  // Literal integral
  L_I,
  // Literal decimal
  L_D,
  // Literal no zeroinit
  L_UNINIT,
  L_NULLPTR,
  // format
  S_INTERPOLATION_START,
  S_INTERPOLATION_END,
  // text (default) : {i32*, i32} utf32
  // or str ""str : {i8*, i32} encoding agnostic
  L_TEXTUAL,
  TRUE,
  FALSE,
  L_CUNE,
  L_ARRAY,
  // variable kind
  LET,
  VAR,
  CONST,
  // casting
  AS,
  // flag
  CAPA_REF,
  ADDR,
  AMPERSAND,
  CAPA_MOVE,
  VARIADIC,
  CAPA_MUT,
  CAPA_COPY,
  MEM_DIST,
  RULE,
  WITH,
  ON,
  COMPILETIME,
  // fn lam keyword
  FUNCTION,
  EXTENSION,
  LAMBDA,
  FORM,
  METACODE,
  VIEW,
  FACET,
  USE,
  ENUM,
  FLAG,
  UNION,
  GENERIC,
  IMPORT,
  EXPORT,
  REEXPORT,
  EXTERN,
  PIPE,
  PIPE_MUT,
  TILDE,
  // memory keyword
  PTR,
  TICK,
  NEW,
  DEL,
  DROP,
  // structure
  L_PAREN,
  R_PAREN,
  L_ANGLE,
  R_ANGLE,
  L_CURLY,
  R_CURLY,
  L_SQUARE,
  R_SQUARE,
  COLON,
  SEMICOLON,
  COMMA,
  INJECT,
  // data operation
  ASSIGN,
  COPY_ASSIGN,
  MOVE_ASSIGN,
  DOT,
  ARROW,
  STATIC_ACCESS,
  UNDERSCORE,
  SPACE,
  TURBO_FISH,
  RUN_RULE,
  OP,
  // logical gates
  OP_AND,
  OP_NAND,
  OP_OR,
  OP_XOR,
  OP_NOR,
  OP_XNOR,
  OP_NOT,
  // binary gates
  OP_B_AND,
  OP_B_NAND,
  OP_B_OR,
  OP_B_XOR,
  OP_B_NOR,
  OP_B_XNOR,
  OP_B_NOT,
  // comparators
  OP_EQ,
  OP_NEQ,
  OP_GEQ,
  OP_LEQ,
  OP_EQS,
  OP_NEQS,
  // math operation
  OP_PLUS,
  OP_MINUS,
  OP_MULTIPLY,
  OP_POWER,
  OP_CIRCUMFLEX,
  OP_DIVIDE,
  OP_MODULO,
  OP_QUOTIEN,
  OP_REMAIN,
  OP_DIVREM,
  OP_MEM_ADD,
  OP_MEM_SUB,
  OP_MEM_DIST,
  // bit operation
  OP_SHIFT_LEFT_0,
  OP_SHIFT_RIGHT_0,
  OP_SHIFT_LEFT_1,
  OP_SHIFT_RIGHT_1,
  OP_SHIFT_LEFT_A,
  OP_SHIFT_RIGHT_A,
  OP_ROTATE_LEFT,
  OP_ROTATE_RIGHT,
  // math affectation
  ASSIGN_PLUS,
  ASSIGN_MINUS,
  ASSIGN_MULTIPLY,
  ASSIGN_POWER,
  ASSIGN_DIVIDE,
  ASSIGN_MODULO,
  ASSIGN_QUOTIEN,
  ASSIGN_REMAIN,
  ASSIGN_DIVREM,
  ASSIGN_MEM_ADD,
  ASSIGN_MEM_SUB,
  ASSIGN_MEM_DIST,
  // logical gates affectation
  ASSIGN_AND,
  ASSIGN_NAND,
  ASSIGN_OR,
  ASSIGN_XOR,
  ASSIGN_NOR,
  ASSIGN_XNOR,
  ASSIGN_NOT,
  // binary gates affectation
  ASSIGN_B_AND,
  ASSIGN_B_NAND,
  ASSIGN_B_OR,
  ASSIGN_B_XOR,
  ASSIGN_B_NOR,
  ASSIGN_B_XNOR,
  ASSIGN_B_NOT,
  // bit affectation
  ASSIGN_SHIFT_LEFT_0,
  ASSIGN_SHIFT_RIGHT_0,
  ASSIGN_SHIFT_LEFT_1,
  ASSIGN_SHIFT_RIGHT_1,
  ASSIGN_SHIFT_LEFT_A,
  ASSIGN_SHIFT_RIGHT_A,
  ASSIGN_ROTATE_LEFT,
  ASSIGN_ROTATE_RIGHT,

  // control flow
  IF,
  ELSE,
  ELIF,
  FOR,
  IN,
  NIN,
  IS,
  NIS,
  STEP,
  LOOP,
  WHILE,
  DO_WHILE,
  MATCH,
  BREAK,
  CONTINUE,
  RETURN,
  END,
  GOTO,
  GOTO_LABEL,
  // string
  QUOTE,
  // range
  RANGE,
  RANGE_INCLUSIVE,
  // other
  PERCENTAGE,
};

extern const std::map<std::string_view, ETokenKind> k_keywords;

extern const std::unordered_map<std::string_view, ETokenKind> k_DFA;

const std::initializer_list<ETokenKind> k_variable = {
    ETokenKind::LET,
    ETokenKind::VAR,
    ETokenKind::CONST,
};

const std::initializer_list<ETokenKind> k_start_identifier = {ETokenKind::STATIC_ACCESS, ETokenKind::IDENTIFIER,
                                                              ETokenKind::SUPER_MOD, ETokenKind::SELF};

const std::initializer_list<ETokenKind> k_access = {ETokenKind::STATIC_ACCESS, ETokenKind::DOT};

const std::initializer_list<ETokenKind> k_parameter_passmode = {
    ETokenKind::CAPA_MUT, ETokenKind::CAPA_REF, ETokenKind::CAPA_COPY, ETokenKind::ADDR, ETokenKind::CAPA_MOVE};

const std::initializer_list<ETokenKind> k_capability = {
    ETokenKind::CAPA_MUT,
    ETokenKind::CAPA_REF,
    ETokenKind::CAPA_MOVE,
    ETokenKind::CAPA_COPY,
};


const std::initializer_list<ETokenKind> k_operator = {
    ETokenKind::OP_PLUS,         ETokenKind::OP_MINUS,
    ETokenKind::OP_MULTIPLY,     ETokenKind::OP_POWER,
    ETokenKind::OP_DIVIDE,       ETokenKind::OP_MODULO,
    ETokenKind::OP_QUOTIEN,      ETokenKind::OP_REMAIN,
    ETokenKind::OP_DIVREM,       ETokenKind::OP_EQ,
    ETokenKind::OP_NEQ,          ETokenKind::OP_EQS,
    ETokenKind::OP_NEQS,         ETokenKind::OP_GEQ,
    ETokenKind::OP_LEQ,          ETokenKind::L_ANGLE,
    ETokenKind::R_ANGLE,         ETokenKind::OP_AND,
    ETokenKind::OP_NAND,         ETokenKind::OP_OR,
    ETokenKind::OP_XOR,          ETokenKind::OP_NOR,
    ETokenKind::OP_XNOR,         ETokenKind::OP_NOT,
    ETokenKind::EXCLAMATION,     ETokenKind::OP_B_AND,
    ETokenKind::OP_B_NAND,       ETokenKind::OP_B_OR,
    ETokenKind::OP_B_XOR,        ETokenKind::OP_B_NOR,
    ETokenKind::OP_B_XNOR,       ETokenKind::OP_B_NOT,
    ETokenKind::OP_SHIFT_LEFT_0, ETokenKind::OP_SHIFT_RIGHT_0,
    ETokenKind::OP_SHIFT_LEFT_1, ETokenKind::OP_SHIFT_RIGHT_1,
    ETokenKind::OP_SHIFT_LEFT_A, ETokenKind::OP_SHIFT_RIGHT_A,
    ETokenKind::OP_ROTATE_LEFT,  ETokenKind::OP_ROTATE_RIGHT,
};

const std::initializer_list<ETokenKind> k_op_bitwise_shift = {
    ETokenKind::OP_SHIFT_LEFT_0,  ETokenKind::OP_SHIFT_RIGHT_0, ETokenKind::OP_SHIFT_LEFT_1,
    ETokenKind::OP_SHIFT_RIGHT_1, ETokenKind::OP_SHIFT_LEFT_A,  ETokenKind::OP_SHIFT_RIGHT_A,
    ETokenKind::OP_ROTATE_LEFT,   ETokenKind::OP_ROTATE_RIGHT,
};

const std::initializer_list<ETokenKind> k_op_bitwise = {
    ETokenKind::OP_B_AND,         ETokenKind::OP_B_NAND,        ETokenKind::OP_B_OR,
    ETokenKind::OP_B_XOR,         ETokenKind::OP_B_NOR,         ETokenKind::OP_B_XNOR,
    ETokenKind::OP_B_NOT,         ETokenKind::OP_SHIFT_LEFT_0,  ETokenKind::OP_SHIFT_RIGHT_0,
    ETokenKind::OP_SHIFT_LEFT_1,  ETokenKind::OP_SHIFT_RIGHT_1, ETokenKind::OP_SHIFT_LEFT_A,
    ETokenKind::OP_SHIFT_RIGHT_A, ETokenKind::OP_ROTATE_LEFT,   ETokenKind::OP_ROTATE_RIGHT,
};

const std::initializer_list<ETokenKind> k_op_unary = {ETokenKind::OP_PLUS, ETokenKind::OP_MINUS, ETokenKind::OP_NOT,
                                                      ETokenKind::EXCLAMATION};

const std::initializer_list<ETokenKind> k_op_comparison = {
    ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_EQ,  ETokenKind::OP_NEQ, ETokenKind::OP_EQS,
    ETokenKind::OP_NEQS, ETokenKind::OP_LEQ,  ETokenKind::OP_GEQ, ETokenKind::IN,     ETokenKind::NIN,
};

const std::initializer_list<ETokenKind> k_type_primitive = {
    ETokenKind::T_U0,     ETokenKind::T_BOOL,    ETokenKind::T_CUNE,    ETokenKind::T_CSTR,  ETokenKind::T_STR,
    ETokenKind::T_RUNE,   ETokenKind::T_TEXT,    ETokenKind::T_USIZE,   ETokenKind::L_BIN,   ETokenKind::L_HEX,
    ETokenKind::L_OCT,    ETokenKind::L_I,       ETokenKind::L_D,       ETokenKind::T_S8,    ETokenKind::T_S16,
    ETokenKind::T_S32,    ETokenKind::T_S64,     ETokenKind::T_S128,    ETokenKind::T_SSIZE, ETokenKind::T_U8,
    ETokenKind::T_U16,    ETokenKind::T_U32,     ETokenKind::T_U64,     ETokenKind::T_U128,  ETokenKind::T_USIZE,
    ETokenKind::T_B8,     ETokenKind::T_B16,     ETokenKind::T_B32,     ETokenKind::T_B64,   ETokenKind::T_B128,
    ETokenKind::T_BSIZE,  ETokenKind::T_F16,     ETokenKind::T_F32,     ETokenKind::T_F64,   ETokenKind::T_F80,
    ETokenKind::T_F128,   ETokenKind::T_FSIZE,   ETokenKind::T_PTRDIFF, ETokenKind::T_D32,   ETokenKind::T_D64,
    ETokenKind::T_D128,   ETokenKind::T_DSIZE,   ETokenKind::T_UD32,    ETokenKind::T_UD64,  ETokenKind::T_UD128,
    ETokenKind::T_UDSIZE, ETokenKind::L_NULLPTR, ETokenKind::T_OPAQUE,
};
const std::initializer_list<ETokenKind> k_type_integral = {
    ETokenKind::T_S8, ETokenKind::T_S16, ETokenKind::T_S32, ETokenKind::T_S64, ETokenKind::T_S128, ETokenKind::T_SSIZE,
    ETokenKind::T_U8, ETokenKind::T_U16, ETokenKind::T_U32, ETokenKind::T_U64, ETokenKind::T_U128, ETokenKind::T_USIZE,
    ETokenKind::T_B8, ETokenKind::T_B16, ETokenKind::T_B32, ETokenKind::T_B64, ETokenKind::T_B128, ETokenKind::T_BSIZE,
};
const std::initializer_list<ETokenKind> k_type_integral_signed = {
    ETokenKind::T_S8, ETokenKind::T_S16, ETokenKind::T_S32, ETokenKind::T_S64, ETokenKind::T_S128, ETokenKind::T_SSIZE,
};
const std::initializer_list<ETokenKind> k_type_integral_unsigned = {
    ETokenKind::T_U8, ETokenKind::T_U16, ETokenKind::T_U32, ETokenKind::T_U64, ETokenKind::T_U128, ETokenKind::T_USIZE,
};
const std::initializer_list<ETokenKind> k_type_integral_binary = {
    ETokenKind::T_B8, ETokenKind::T_B16, ETokenKind::T_B32, ETokenKind::T_B64, ETokenKind::T_B128, ETokenKind::T_BSIZE,
};
const std::initializer_list<ETokenKind> k_type_floating_point = {
    ETokenKind::T_F16, ETokenKind::T_F32, ETokenKind::T_F64, ETokenKind::T_F80, ETokenKind::T_F128, ETokenKind::T_FSIZE,
};
const std::initializer_list<ETokenKind> k_type_numeric = {
    ETokenKind::L_BIN,   ETokenKind::L_HEX,   ETokenKind::L_OCT,     ETokenKind::L_I,   ETokenKind::L_D,
    ETokenKind::T_S8,    ETokenKind::T_S16,   ETokenKind::T_S32,     ETokenKind::T_S64, ETokenKind::T_S128,
    ETokenKind::T_SSIZE, ETokenKind::T_U8,    ETokenKind::T_U16,     ETokenKind::T_U32, ETokenKind::T_U64,
    ETokenKind::T_U128,  ETokenKind::T_USIZE, ETokenKind::T_B8,      ETokenKind::T_B16, ETokenKind::T_B32,
    ETokenKind::T_B64,   ETokenKind::T_B128,  ETokenKind::T_BSIZE,   ETokenKind::T_F32, ETokenKind::T_F64,
    ETokenKind::T_F128,  ETokenKind::T_FSIZE, ETokenKind::T_PTRDIFF,
};
const std::initializer_list<ETokenKind> k_type_fixed_point = {
    ETokenKind::T_D32,  ETokenKind::T_D64,  ETokenKind::T_D128,  ETokenKind::T_DSIZE,
    ETokenKind::T_UD32, ETokenKind::T_UD64, ETokenKind::T_UD128, ETokenKind::T_UDSIZE,
};
const std::initializer_list<ETokenKind> k_type_boolean = {ETokenKind::TRUE, ETokenKind::FALSE, ETokenKind::T_BOOL};

const std::initializer_list<ETokenKind> k_op_arithmetic = {
    ETokenKind::OP_PLUS,   ETokenKind::OP_MINUS,   ETokenKind::OP_MULTIPLY, ETokenKind::OP_DIVIDE, ETokenKind::OP_POWER,
    ETokenKind::OP_MODULO, ETokenKind::OP_QUOTIEN, ETokenKind::OP_REMAIN,   ETokenKind::OP_DIVREM};

const std::initializer_list<ETokenKind> k_text_interpolation = {
    ETokenKind::S_INTERPOLATION_START,
    ETokenKind::S_INTERPOLATION_END,
};
const std::initializer_list<ETokenKind> k_type = {
    ETokenKind::T_S8,    ETokenKind::T_S16,     ETokenKind::T_S32,   ETokenKind::T_S64,    ETokenKind::T_S128,
    ETokenKind::T_SSIZE, ETokenKind::T_U8,      ETokenKind::T_U16,   ETokenKind::T_U32,    ETokenKind::T_U64,
    ETokenKind::T_U128,  ETokenKind::T_USIZE,   ETokenKind::T_B8,    ETokenKind::T_B16,    ETokenKind::T_B32,
    ETokenKind::T_B64,   ETokenKind::T_B128,    ETokenKind::T_BSIZE, ETokenKind::T_F16,    ETokenKind::T_F32,
    ETokenKind::T_F64,   ETokenKind::T_F80,     ETokenKind::T_F128,  ETokenKind::T_FSIZE,  ETokenKind::T_BOOL,
    ETokenKind::T_RUNE,  ETokenKind::T_CUNE,    ETokenKind::T_D32,   ETokenKind::T_D64,    ETokenKind::T_D128,
    ETokenKind::T_DSIZE, ETokenKind::T_UD32,    ETokenKind::T_UD64,  ETokenKind::T_UD128,  ETokenKind::T_UDSIZE,
    ETokenKind::T_CSTR,  ETokenKind::T_STR,     ETokenKind::T_TEXT,  ETokenKind::FUNCTION, ETokenKind::IDENTIFIER,
    ETokenKind::TYPE,    ETokenKind::T_PTRDIFF,
};
const std::initializer_list<ETokenKind> k_hybrid_namespace = {
    ETokenKind::T_S8,      ETokenKind::T_S16,    ETokenKind::T_S32,     ETokenKind::T_S64,    ETokenKind::T_S128,
    ETokenKind::T_SSIZE,   ETokenKind::T_U8,     ETokenKind::T_U16,     ETokenKind::T_U32,    ETokenKind::T_U64,
    ETokenKind::T_U128,    ETokenKind::T_USIZE,  ETokenKind::T_B8,      ETokenKind::T_B16,    ETokenKind::T_B32,
    ETokenKind::T_B64,     ETokenKind::T_B128,   ETokenKind::T_BSIZE,   ETokenKind::T_F16,    ETokenKind::T_F32,
    ETokenKind::T_F64,     ETokenKind::T_F80,    ETokenKind::T_F128,    ETokenKind::T_FSIZE,  ETokenKind::T_BOOL,
    ETokenKind::T_RUNE,    ETokenKind::T_CUNE,   ETokenKind::T_D32,     ETokenKind::T_D64,    ETokenKind::T_D128,
    ETokenKind::T_DSIZE,   ETokenKind::T_UD32,   ETokenKind::T_UD64,    ETokenKind::T_UD128,  ETokenKind::T_UDSIZE,
    ETokenKind::T_CSTR,    ETokenKind::T_STR,    ETokenKind::T_TEXT,    ETokenKind::FUNCTION, ETokenKind::IDENTIFIER,
    ETokenKind::TYPE,      ETokenKind::T_U0,     ETokenKind::EXTENSION, ETokenKind::LAMBDA,   ETokenKind::FLAG,
    ETokenKind::PTR,       ETokenKind::FORM,     ETokenKind::FACET,     ETokenKind::USE,      ETokenKind::GENERIC,
    ETokenKind::T_PTRDIFF, ETokenKind::L_UNINIT, ETokenKind::L_NULLPTR, ETokenKind::T_OPAQUE,
};
const std::initializer_list<ETokenKind> k_op_logical = {ETokenKind::OP_AND, ETokenKind::OP_NAND,    ETokenKind::OP_OR,
                                                        ETokenKind::OP_XOR, ETokenKind::OP_NOR,     ETokenKind::OP_XNOR,
                                                        ETokenKind::OP_NOT, ETokenKind::EXCLAMATION};

const std::initializer_list<ETokenKind> k_args_ending = {
    ETokenKind::R_ANGLE, ETokenKind::R_CURLY, ETokenKind::L_CURLY,  ETokenKind::R_PAREN,   ETokenKind::R_SQUARE,
    ETokenKind::ASSIGN,  ETokenKind::PIPE,    ETokenKind::PIPE_MUT, ETokenKind::SEMICOLON, ETokenKind::METACODE,
};
const std::initializer_list<ETokenKind> k_args_delimitation = {
    ETokenKind::R_ANGLE,  ETokenKind::L_ANGLE,  ETokenKind::R_PAREN,   ETokenKind::L_PAREN,
    ETokenKind::R_SQUARE, ETokenKind::L_SQUARE, ETokenKind::SEMICOLON, ETokenKind::PIPE};
const std::initializer_list<ETokenKind> k_args_generic_valid = {
    ETokenKind::T_S8,       ETokenKind::T_S16,         ETokenKind::T_S32,      ETokenKind::T_S64,
    ETokenKind::T_S128,     ETokenKind::T_SSIZE,       ETokenKind::T_U8,       ETokenKind::T_U16,
    ETokenKind::T_U32,      ETokenKind::T_U64,         ETokenKind::T_U128,     ETokenKind::T_USIZE,
    ETokenKind::T_B8,       ETokenKind::T_B16,         ETokenKind::T_B32,      ETokenKind::T_B64,
    ETokenKind::T_B128,     ETokenKind::T_BSIZE,       ETokenKind::T_F32,      ETokenKind::T_F64,
    ETokenKind::T_F128,     ETokenKind::T_FSIZE,       ETokenKind::DOLLAR,     ETokenKind::INTERROGATIVE,
    ETokenKind::IDENTIFIER, ETokenKind::STATIC_ACCESS, ETokenKind::PTR,        ETokenKind::TICK,
    ETokenKind::HASHTAG,    ETokenKind::L_ANGLE,       ETokenKind::R_ANGLE,    ETokenKind::IDENTIFIER,
    ETokenKind::T_BOOL,     ETokenKind::T_RUNE,        ETokenKind::T_CUNE,     ETokenKind::T_D32,
    ETokenKind::T_D64,      ETokenKind::T_D128,        ETokenKind::T_DSIZE,    ETokenKind::T_UD32,
    ETokenKind::T_UD64,     ETokenKind::T_UD128,       ETokenKind::T_UDSIZE,   ETokenKind::T_STR,
    ETokenKind::T_TEXT,     ETokenKind::FUNCTION,      ETokenKind::IDENTIFIER, ETokenKind::TYPE,
    ETokenKind::COMMA,      ETokenKind::LET,           ETokenKind::T_PTRDIFF,
};

const std::initializer_list<ETokenKind> k_lit = {
    ETokenKind::IDENTIFIER, ETokenKind::L_BIN,  ETokenKind::L_OCT,     ETokenKind::L_HEX,
    ETokenKind::L_I,        ETokenKind::L_D,    ETokenKind::L_TEXTUAL, ETokenKind::TRUE,
    ETokenKind::FALSE,      ETokenKind::L_CUNE, ETokenKind::L_ARRAY,
};
const std::initializer_list<ETokenKind> k_invalid_tokens = {
    ETokenKind::METACODE,
    ETokenKind::S_METACODE_END,
    ETokenKind::S_END_OF_FILE,
};

const std::initializer_list<ETokenKind> k_op_assign = {
    ETokenKind::MOVE_ASSIGN,          ETokenKind::COPY_ASSIGN,          ETokenKind::ASSIGN,
    ETokenKind::ASSIGN_PLUS,          ETokenKind::ASSIGN_MINUS,         ETokenKind::ASSIGN_MULTIPLY,
    ETokenKind::ASSIGN_DIVIDE,        ETokenKind::ASSIGN_POWER,         ETokenKind::ASSIGN_MODULO,
    ETokenKind::ASSIGN_QUOTIEN,       ETokenKind::ASSIGN_REMAIN,        ETokenKind::ASSIGN_DIVREM,
    ETokenKind::ASSIGN_MEM_ADD,       ETokenKind::ASSIGN_MEM_SUB,       ETokenKind::ASSIGN_MEM_DIST,
    ETokenKind::ASSIGN_AND,           ETokenKind::ASSIGN_NAND,          ETokenKind::ASSIGN_OR,
    ETokenKind::ASSIGN_XOR,           ETokenKind::ASSIGN_NOR,           ETokenKind::ASSIGN_XNOR,
    ETokenKind::ASSIGN_NOT,           ETokenKind::ASSIGN_B_AND,         ETokenKind::ASSIGN_B_NAND,
    ETokenKind::ASSIGN_B_OR,          ETokenKind::ASSIGN_B_XOR,         ETokenKind::ASSIGN_B_NOR,
    ETokenKind::ASSIGN_B_XNOR,        ETokenKind::ASSIGN_B_NOT,         ETokenKind::ASSIGN_SHIFT_LEFT_0,
    ETokenKind::ASSIGN_SHIFT_RIGHT_0, ETokenKind::ASSIGN_SHIFT_LEFT_1,  ETokenKind::ASSIGN_SHIFT_RIGHT_1,
    ETokenKind::ASSIGN_SHIFT_LEFT_A,  ETokenKind::ASSIGN_SHIFT_RIGHT_A, ETokenKind::ASSIGN_ROTATE_LEFT,
    ETokenKind::ASSIGN_ROTATE_RIGHT,
};
const std::initializer_list<ETokenKind> k_op_boolean = {
    ETokenKind::OP_AND,       ETokenKind::OP_OR,        ETokenKind::OP_NOR,        ETokenKind::OP_XOR,
    ETokenKind::OP_XNOR,      ETokenKind::OP_NAND,      ETokenKind::OP_NEQ,        ETokenKind::OP_B_AND,
    ETokenKind::OP_B_OR,      ETokenKind::OP_B_NOR,     ETokenKind::OP_B_XOR,      ETokenKind::OP_B_XNOR,
    ETokenKind::OP_B_NAND,    ETokenKind::OP_EQ,        ETokenKind::ASSIGN,        ETokenKind::OP_NOT,
    ETokenKind::EXCLAMATION,  ETokenKind::OP_B_NOT,     ETokenKind::ASSIGN_AND,    ETokenKind::ASSIGN_NAND,
    ETokenKind::ASSIGN_OR,    ETokenKind::ASSIGN_XOR,   ETokenKind::ASSIGN_NOR,    ETokenKind::ASSIGN_XNOR,
    ETokenKind::ASSIGN_NOT,   ETokenKind::ASSIGN_B_AND, ETokenKind::ASSIGN_B_NAND, ETokenKind::ASSIGN_B_OR,
    ETokenKind::ASSIGN_B_XOR, ETokenKind::ASSIGN_B_NOR, ETokenKind::ASSIGN_B_XNOR, ETokenKind::ASSIGN_B_NOT,
};
const std::initializer_list<ETokenKind> k_op_integral = {
    ETokenKind::OP_PLUS,         ETokenKind::OP_MINUS,      ETokenKind::OP_MULTIPLY,   ETokenKind::OP_POWER,
    ETokenKind::OP_DIVIDE,       ETokenKind::OP_MODULO,     ETokenKind::OP_QUOTIEN,    ETokenKind::OP_REMAIN,
    ETokenKind::OP_DIVREM,       ETokenKind::ASSIGN,        ETokenKind::ASSIGN_PLUS,   ETokenKind::ASSIGN_MINUS,
    ETokenKind::ASSIGN_MULTIPLY, ETokenKind::ASSIGN_POWER,  ETokenKind::ASSIGN_DIVIDE, ETokenKind::ASSIGN_MODULO,
    ETokenKind::ASSIGN_QUOTIEN,  ETokenKind::ASSIGN_REMAIN, ETokenKind::ASSIGN_DIVREM, ETokenKind::L_ANGLE,
    ETokenKind::R_ANGLE,         ETokenKind::OP_GEQ,        ETokenKind::OP_LEQ,        ETokenKind::OP_EQ,
    ETokenKind::OP_NEQ,
};
const std::initializer_list<ETokenKind> k_op_numeric = {
    ETokenKind::OP_PLUS,         ETokenKind::OP_MINUS,      ETokenKind::OP_MULTIPLY,   ETokenKind::OP_POWER,
    ETokenKind::OP_DIVIDE,       ETokenKind::OP_MODULO,     ETokenKind::OP_QUOTIEN,    ETokenKind::OP_REMAIN,
    ETokenKind::OP_DIVREM,       ETokenKind::ASSIGN,        ETokenKind::ASSIGN_PLUS,   ETokenKind::ASSIGN_MINUS,
    ETokenKind::ASSIGN_MULTIPLY, ETokenKind::ASSIGN_POWER,  ETokenKind::ASSIGN_DIVIDE, ETokenKind::ASSIGN_MODULO,
    ETokenKind::ASSIGN_QUOTIEN,  ETokenKind::ASSIGN_REMAIN, ETokenKind::ASSIGN_DIVREM, ETokenKind::L_ANGLE,
    ETokenKind::R_ANGLE,         ETokenKind::OP_GEQ,        ETokenKind::OP_LEQ,        ETokenKind::OP_EQ,
    ETokenKind::OP_NEQ,          ETokenKind::OP_EQS,        ETokenKind::OP_NEQS,
};
const std::initializer_list<ETokenKind> k_op_char = {
    ETokenKind::ASSIGN, ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_GEQ,  ETokenKind::OP_LEQ,
    ETokenKind::OP_EQ,  ETokenKind::OP_NEQ,  ETokenKind::OP_EQS,  ETokenKind::OP_NEQS,
};
const std::initializer_list<ETokenKind> k_op_text = {
    ETokenKind::OP_PLUS, ETokenKind::ASSIGN, ETokenKind::ASSIGN_PLUS, ETokenKind::L_ANGLE,
    ETokenKind::R_ANGLE, ETokenKind::OP_GEQ, ETokenKind::OP_LEQ,      ETokenKind::OP_EQ,
    ETokenKind::OP_NEQ,  ETokenKind::OP_EQS, ETokenKind::OP_NEQS,
};
const std::initializer_list<ETokenKind> k_op_enum = {
    ETokenKind::ASSIGN, ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_GEQ,
    ETokenKind::OP_LEQ, ETokenKind::OP_EQ,   ETokenKind::OP_NEQ,
};
const std::initializer_list<ETokenKind> k_op_address = {
    ETokenKind::ASSIGN, ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_GEQ,
    ETokenKind::OP_LEQ, ETokenKind::OP_EQ,   ETokenKind::OP_NEQ,
};
const std::initializer_list<ETokenKind> k_op_array = {
    ETokenKind::ASSIGN, ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_GEQ,
    ETokenKind::OP_LEQ, ETokenKind::OP_EQ,   ETokenKind::OP_NEQ,
};
const std::initializer_list<ETokenKind> k_op_format = {
    ETokenKind::L_ANGLE, ETokenKind::R_ANGLE, ETokenKind::OP_CIRCUMFLEX, ETokenKind::TILDE, ETokenKind::ASSIGN,
};

const std::initializer_list<ETokenKind> k_identifier_possible = {
    // mod key
    ETokenKind::MOD,
    // type keys
    ETokenKind::TYPE, ETokenKind::T_U0, ETokenKind::T_BOOL, ETokenKind::ENUM, ETokenKind::T_RUNE, ETokenKind::T_CUNE,
    ETokenKind::T_SSIZE, ETokenKind::T_USIZE, ETokenKind::T_BSIZE, ETokenKind::T_FSIZE, ETokenKind::T_S8,
    ETokenKind::T_U8, ETokenKind::T_B8, ETokenKind::T_S16, ETokenKind::T_U16, ETokenKind::T_B16, ETokenKind::T_S32,
    ETokenKind::T_U32, ETokenKind::T_B32, ETokenKind::T_S64, ETokenKind::T_U64, ETokenKind::T_B64, ETokenKind::T_S128,
    ETokenKind::T_U128, ETokenKind::T_B128, ETokenKind::T_F16, ETokenKind::T_F32, ETokenKind::T_F64, ETokenKind::T_F80,
    ETokenKind::T_F128, ETokenKind::T_CSTR, ETokenKind::T_STR, ETokenKind::T_TEXT, ETokenKind::T_D32, ETokenKind::T_D64,
    ETokenKind::T_D128, ETokenKind::T_DSIZE, ETokenKind::T_UD32, ETokenKind::T_UD64, ETokenKind::T_UD128,
    ETokenKind::T_UDSIZE, ETokenKind::T_TEXT, ETokenKind::T_PTRDIFF, ETokenKind::T_OPAQUE,
    // boolean
    // literals
    ETokenKind::TRUE, ETokenKind::FALSE,
    // variable
    // declaration
    ETokenKind::LET, ETokenKind::VAR, ETokenKind::CONST,
    // typeid
    // checking
    ETokenKind::IS,
    // SFM keys
    ETokenKind::FORM, ETokenKind::USE, ETokenKind::VIEW, ETokenKind::FACET, ETokenKind::RULE, ETokenKind::SELF,
    // generic keys
    ETokenKind::GENERIC,
    // no zeroinit
    ETokenKind::L_UNINIT,
    // nullptr
    ETokenKind::L_NULLPTR,
    // function keys
    ETokenKind::FUNCTION, ETokenKind::EXTENSION, ETokenKind::LAMBDA,
    // pointers
    ETokenKind::PTR, ETokenKind::NEW, ETokenKind::DEL,
    // variable pass mode
    ETokenKind::CAPA_REF, ETokenKind::ADDR, ETokenKind::CAPA_MOVE, ETokenKind::WITH, ETokenKind::ON,
    // operator overloading
    ETokenKind::OP,
    // logical operators
    ETokenKind::OP_AND, ETokenKind::OP_NAND, ETokenKind::OP_OR, ETokenKind::OP_NOR, ETokenKind::OP_XOR,
    ETokenKind::OP_XNOR, ETokenKind::OP_NOT,
    // statement keys
    ETokenKind::IF, ETokenKind::ELSE, ETokenKind::FOR, ETokenKind::IN, ETokenKind::NIN, ETokenKind::STEP,
    // while keys
    ETokenKind::WHILE, ETokenKind::DO_WHILE, ETokenKind::LOOP,
    // match keys
    ETokenKind::MATCH,
    // flow keys
    ETokenKind::BREAK, ETokenKind::CONTINUE, ETokenKind::RETURN,
    // identifier
    ETokenKind::IDENTIFIER};


struct Token final {
  ID         tokid;
  uint32_t   begin  = INVALID_POS;
  uint16_t   length = INVALID_LEN;
  ETokenKind kind   = ETokenKind::UNKNOWN;
#ifdef DEBUG
  std::string debug_val;
#endif
};

struct Arena final {
  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
  }

  struct Audit final {
    Arena& arena;

    [[nodiscard]] std::string_view Token_to_str(ID tokid) const noexcept;
    [[nodiscard]] size_t           Token_to_line(ID tokid) const noexcept;
    [[nodiscard]] std::string_view Token_to_line_str(ID tokid) const noexcept;
  };

  const Audit audit{*this};

  const cu::ID cuid;

  // index = token id
  std::vector<Token> tokens;

  [[nodiscard]] ID add(Token& tok) noexcept;

  [[nodiscard]] const Token& get(ID id) const noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }

  [[nodiscard]] Token& get(ID id) noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }
};


[[nodiscard]] inline bool str_is_identifier(std::string_view s)
{
  if (s.empty()) return false;
  if (!std::isalpha(s[0]) && s[0] != '_') return false;
  for (size_t i = 1; i < s.size(); ++i)
    if (!std::isalnum(s[i]) && s[i] != '_') return false;
  return true;
}

[[nodiscard]] inline bool isKeywordChar(char ch)
{
  return !std::isspace(ch) && !std::iscntrl(ch);
}

[[nodiscard]] inline ETokenKind str_to_ETokenKind(std::string_view str)
{
  if (auto it = k_keywords.find(str); it != k_keywords.end()) return it->second;
  return ETokenKind::UNKNOWN;
}

} // namespace token
