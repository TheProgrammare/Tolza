#pragma once

#include <common/enum_lite.hpp>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string_view>
#include <unordered_map>


namespace token
{

// T_ = Type L_ = Literal S_ = Special (no text key representation)
DEFINE_ENUM(ETokenKind, uint8_t,       //
            S_END_OF_FILE, 1,          // end of metacode (end of line)
            S_METACODE_END, 2,         //
            S_METACODE_EXPAND_IF, 3,   //
            S_METACODE_PLACEHOLDER, 4, //
            MOD, 5,                    //
            SUPER_MOD, 6,              //
            SELF, 7,                   //
            IDENTIFIER, 8,             //
            OPERATOR, 9,               //
            DELIMITER, 10,             //
            // types
            TYPE, 11,          //
            DOLLAR, 12,        //
            INTERROGATIVE, 13, //
            EXCLAMATION, 14,   //
            T_U0, 15,          //
            T_BOOL, 16,        //
            // 8 bits
            T_CUNE, 17, //
            // 32 bits
            T_RUNE, 18,    //
            T_S8, 19,      //
            T_S16, 20,     //
            T_S32, 21,     //
            T_S64, 22,     //
            T_S128, 23,    //
            T_SSIZE, 24,   //
            T_U8, 25,      //
            T_U16, 26,     //
            T_U32, 27,     //
            T_U64, 28,     //
            T_U128, 29,    //
            T_USIZE, 30,   //
            T_B8, 31,      //
            T_B16, 32,     //
            T_B32, 33,     //
            T_B64, 34,     //
            T_B128, 35,    //
            T_BSIZE, 36,   //
            T_PTRDIFF, 37, //
            T_F16, 38,     //
            T_F32, 39,     //
            T_F64, 40,     //
            T_F80, 41,     //
            T_F128, 42,    //
            T_FSIZE, 43,   //
            T_D32, 44,     //
            T_D64, 45,     //
            T_D128, 46,    //
            T_DSIZE, 47,   //
            T_UD32, 48,    //
            T_UD64, 49,    //
            T_UD128, 50,   //
            T_UDSIZE, 51,  //
            T_OPAQUE, 52,  //
            // C string convention: i8* null terminated
            T_CSTR, 53, //
            // fat pointer of cune null terminated
            T_STR, 54, //
            // fat pointer of rune null terminated
            T_TEXT, 55,  //
            T_ARRAY, 56, //
            T_TUPLE, 57, //
            HASHTAG, 58, //
            AT, 59,      //
            // literal values
            // Literal binary
            L_BIN, 60, //
            // Literal octal
            L_OCT, 61, //
            // Literal hexadecimal
            L_HEX, 62, //
            // Literal integral
            L_I, 63, //
            // Literal decimal
            L_D, 64, //
            // Literal no zeroinit
            L_UNINIT, 65,  //
            L_NULLPTR, 66, //
            // format
            S_INTERPOLATION_START, 67, //
            S_INTERPOLATION_END, 68,   //
            // text (default) : {i32*, i32} utf32
            // or str ""str : {i8*, i32} encoding agnostic
            L_TEXTUAL, 69, //
            TRUE, 70,      //
            FALSE, 71,     //
            L_CUNE, 72,    //
            L_ARRAY, 73,   //
            // variable kind
            LET, 74,   //
            VAR, 75,   //
            CONST, 76, //
            // casting
            AS, 77, //
            // flag
            CAPA_REF, 78,    //
            ADDR, 79,        //
            AMPERSAND, 80,   //
            CAPA_MOVE, 81,   //
            VARIADIC, 82,    //
            CAPA_MUT, 83,    //
            CAPA_COPY, 84,   //
            MEM_DIST, 85,    //
            RULE, 86,        //
            WITH, 87,        //
            ON, 88,          //
            COMPILETIME, 89, //
            // fn lam keyword
            FUNCTION, 90,  //
            PRE, 91,       //
            POST, 92,      //
            EXTENSION, 93, //
            LAMBDA, 94,    //
            FORM, 95,      //
            METACODE, 96,  //
            VIEW, 97,      //
            FACET, 98,     //
            USE, 99,       //
            ENUM, 100,     //
            FLAG, 101,     //
            UNION, 102,    //
            GENERIC, 103,  //
            IMPORT, 104,   //
            EXPORT, 105,   //
            REEXPORT, 106, //
            EXTERN, 107,   //
            PIPE, 108,     //
            PIPE_MUT, 109, //
            TILDE, 110,    //
            ASSERT, 111,   //
            // memory keyword
            PTR, 112,  //
            TICK, 113, //
            NEW, 114,  //
            DEL, 115,  //
            DROP, 116, //
            // structure
            L_PAREN, 117,   //
            R_PAREN, 118,   //
            L_ANGLE, 119,   //
            R_ANGLE, 120,   //
            L_CURLY, 121,   //
            R_CURLY, 122,   //
            L_SQUARE, 123,  //
            R_SQUARE, 124,  //
            COLON, 125,     //
            SEMICOLON, 126, //
            COMMA, 127,     //
            INJECT, 128,    //
            // data operation
            ASSIGN, 129,        //
            COPY_ASSIGN, 130,   //
            MOVE_ASSIGN, 131,   //
            DOT, 132,           //
            ARROW, 133,         //
            STATIC_ACCESS, 134, //
            UNDERSCORE, 135,    //
            SPACE, 136,         //
            TURBO_FISH, 137,    //
            RUN_RULE, 138,      //
            OP, 139,            //
            // logical gates
            OP_AND, 140,  //
            OP_NAND, 141, //
            OP_OR, 142,   //
            OP_XOR, 143,  //
            OP_NOR, 144,  //
            OP_XNOR, 145, //
            OP_NOT, 146,  //
            // binary gates
            OP_B_AND, 147,  //
            OP_B_NAND, 148, //
            OP_B_OR, 149,   //
            OP_B_XOR, 150,  //
            OP_B_NOR, 151,  //
            OP_B_XNOR, 152, //
            OP_B_NOT, 153,  //
            // comparators
            OP_EQ, 154,   //
            OP_NEQ, 155,  //
            OP_GEQ, 156,  //
            OP_LEQ, 157,  //
            OP_EQS, 158,  //
            OP_NEQS, 159, //
            // math operation
            OP_PLUS, 160,       //
            OP_MINUS, 161,      //
            OP_MULTIPLY, 162,   //
            OP_POWER, 163,      //
            OP_CIRCUMFLEX, 164, //
            OP_DIVIDE, 165,     //
            OP_MODULO, 166,     //
            OP_QUOTIEN, 167,    //
            OP_REMAIN, 168,     //
            OP_DIVREM, 169,     //
            OP_MEM_ADD, 170,    //
            OP_MEM_SUB, 171,    //
            OP_MEM_DIST, 172,   //
            // bit operation
            OP_SHIFT_LEFT_0, 173,  //
            OP_SHIFT_RIGHT_0, 174, //
            OP_SHIFT_LEFT_1, 175,  //
            OP_SHIFT_RIGHT_1, 176, //
            OP_SHIFT_LEFT_A, 177,  //
            OP_SHIFT_RIGHT_A, 178, //
            OP_ROTATE_LEFT, 179,   //
            OP_ROTATE_RIGHT, 180,  //
            // math affectation
            ASSIGN_PLUS, 181,     //
            ASSIGN_MINUS, 182,    //
            ASSIGN_MULTIPLY, 183, //
            ASSIGN_POWER, 184,    //
            ASSIGN_DIVIDE, 185,   //
            ASSIGN_MODULO, 186,   //
            ASSIGN_QUOTIEN, 187,  //
            ASSIGN_REMAIN, 188,   //
            ASSIGN_DIVREM, 189,   //
            ASSIGN_MEM_ADD, 190,  //
            ASSIGN_MEM_SUB, 191,  //
            ASSIGN_MEM_DIST, 192, //
            // logical gates affectation
            ASSIGN_AND, 193,  //
            ASSIGN_NAND, 194, //
            ASSIGN_OR, 195,   //
            ASSIGN_XOR, 196,  //
            ASSIGN_NOR, 197,  //
            ASSIGN_XNOR, 198, //
            ASSIGN_NOT, 199,  //
            // binary gates affectation
            ASSIGN_B_AND, 200,  //
            ASSIGN_B_NAND, 201, //
            ASSIGN_B_OR, 202,   //
            ASSIGN_B_XOR, 203,  //
            ASSIGN_B_NOR, 204,  //
            ASSIGN_B_XNOR, 205, //
            ASSIGN_B_NOT, 206,  //
            // bit affectation
            ASSIGN_SHIFT_LEFT_0, 207,  //
            ASSIGN_SHIFT_RIGHT_0, 208, //
            ASSIGN_SHIFT_LEFT_1, 209,  //
            ASSIGN_SHIFT_RIGHT_1, 210, //
            ASSIGN_SHIFT_LEFT_A, 211,  //
            ASSIGN_SHIFT_RIGHT_A, 212, //
            ASSIGN_ROTATE_LEFT, 213,   //
            ASSIGN_ROTATE_RIGHT, 214,  //

            // control flow
            IF, 215,         //
            ELSE, 216,       //
            ELIF, 217,       //
            FOR, 218,        //
            IN, 219,         //
            NIN, 220,        //
            IS, 221,         //
            NIS, 222,        //
            STEP, 223,       //
            LOOP, 224,       //
            WHILE, 225,      //
            DO_WHILE, 226,   //
            MATCH, 227,      //
            BREAK, 228,      //
            CONTINUE, 229,   //
            RETURN, 230,     //
            END, 231,        //
            GOTO, 232,       //
            GOTO_LABEL, 233, //
            // string
            QUOTE, 234, //
            // range
            RANGE, 235,           //
            RANGE_INCLUSIVE, 236, //
            // other
            PERCENTAGE, 237, //
)

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

const std::initializer_list<ETokenKind> k_synchronize_point = {
    ETokenKind::FUNCTION, ETokenKind::EXTENSION, ETokenKind::VAR,    ETokenKind::LET,        ETokenKind::CONST,
    ETokenKind::FORM,     ETokenKind::FACET,     ETokenKind::VIEW,   ETokenKind::RULE,       ETokenKind::ENUM,
    ETokenKind::UNION,    ETokenKind::FLAG,      ETokenKind::LAMBDA, ETokenKind::RETURN,     ETokenKind::END,
    ETokenKind::IF,       ETokenKind::ELIF,      ETokenKind::ELSE,   ETokenKind::WHILE,      ETokenKind::DO_WHILE,
    ETokenKind::FOR,      ETokenKind::LOOP,      ETokenKind::GOTO,   ETokenKind::GOTO_LABEL, ETokenKind::MATCH,
};

const std::initializer_list<ETokenKind> k_synchronize_jump = {
    ETokenKind::L_PAREN,
    ETokenKind::L_ANGLE,
    ETokenKind::L_SQUARE,
    ETokenKind::L_CURLY,
    ETokenKind::SEMICOLON,
    ETokenKind::COMMA,
    ETokenKind::COLON,
    ETokenKind::OP_PLUS,
    ETokenKind::OP_MINUS,
    ETokenKind::OP_MULTIPLY,
    ETokenKind::OP_POWER,
    ETokenKind::OP_DIVIDE,
    ETokenKind::OP_MODULO,
    ETokenKind::OP_QUOTIEN,
    ETokenKind::OP_REMAIN,
    ETokenKind::OP_DIVREM,
    ETokenKind::OP_EQ,
    ETokenKind::OP_NEQ,
    ETokenKind::OP_EQS,
    ETokenKind::OP_NEQS,
    ETokenKind::OP_GEQ,
    ETokenKind::OP_LEQ,
    ETokenKind::L_ANGLE,
    ETokenKind::R_ANGLE,
    ETokenKind::OP_AND,
    ETokenKind::OP_NAND,
    ETokenKind::OP_OR,
    ETokenKind::OP_XOR,
    ETokenKind::OP_NOR,
    ETokenKind::OP_XNOR,
    ETokenKind::OP_NOT,
    ETokenKind::EXCLAMATION,
    ETokenKind::OP_B_AND,
    ETokenKind::OP_B_NAND,
    ETokenKind::OP_B_OR,
    ETokenKind::OP_B_XOR,
    ETokenKind::OP_B_NOR,
    ETokenKind::OP_B_XNOR,
    ETokenKind::OP_B_NOT,
    ETokenKind::OP_SHIFT_LEFT_0,
    ETokenKind::OP_SHIFT_RIGHT_0,
    ETokenKind::OP_SHIFT_LEFT_1,
    ETokenKind::OP_SHIFT_RIGHT_1,
    ETokenKind::OP_SHIFT_LEFT_A,
    ETokenKind::OP_SHIFT_RIGHT_A,
    ETokenKind::OP_ROTATE_LEFT,
    ETokenKind::OP_ROTATE_RIGHT,
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

} // namespace token