#pragma once

#include <cstdint>
#include <map>
#include <string>

// T_ = Type L_ = Literal S_ = Special (no text key representation)
enum class ETokenType {
  UNKNOWN,
  S_END_OF_FILE,
  // end of metacode (end of line)
  S_METACODE_END,
  S_METACODE_EXPAND_IF,
  S_METACODE_PLACEHOLDER,
  MOD,
  SUPER_MOD,
  SELF_MOD,
  IDENTIFIER,
  OPERATOR,
  DELIMITER,
  // types
  TYPE,
  DOLLAR,
  INTERROGATIVE,
  T_U0,
  T_BOOL,
  // 8 bits
  T_CUNE,
  // 32 bits
  T_RUNE,
  T_I8,
  T_I16,
  T_I32,
  T_I64,
  T_I128,
  T_ISIZE,
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
  // C string convention: i8* null terminated
  T_C_STRING,
  // fat pointer of cune null terminated
  T_STRING,
  // fat pointer of rune null terminated
  T_TEXT,
  T_ARRAY,
  T_TUPLE,
  HASHTAG,
  AT,
  STD_LIB,
  PKG_LIB,
  USR_LIB,
  BIND_LIB,
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
  AS_REINTERPRET,
  AS_SAFE,
  CAST,
  // flag
  CAPA_REF,
  ADDR,
  CAPA_MOVE,
  VARIADIC,
  CAPA_MUT,
  CAPA_COPY,
  CAPA_CLONE,
  CAPA_REF_OF,
  VAL_OF,
  CAPA_MOVE_OF,
  CAPA_MUT_OF,
  CAPA_COPY_OF,
  CAPA_CLONE_OF,
  SIZE_OF,
  ADDR_OF,
  MEM_DIST,
  SYSTEM,
  WITH,
  ON,
  COMPILETIME,
  // fn lam keyword
  FUNCTION,
  LAMBDA,
  ENTITY,
  ENTITY_START_LIT,
  METACODE,
  ROLE,
  COMPONENT,
  ENUM,
  FLAG,
  UNION,
  GENERIC,
  IMPORT,
  EXPORT,
  REEXPORT,
  EXTERN,
  USE,
  PIPE,
  PIPE_MUT,
  TILDE,
  // memory keyword
  PTR,
  SPTR,
  UPTR,
  WPTR,
  PTR_AT,
  PTR_OFFSET,
  TICK,
  SELF,
  NEW,
  DEL,
  DROP,
  // structure
  OPEN_PAREN,
  CLOSE_PAREN,
  OPEN_BRACKETS,
  CLOSE_BRACKETS,
  OPEN_BRACE,
  CLOSE_BRACE,
  OPEN_SQUARE,
  CLOSE_SQUARE,
  COLON,
  SEMICOLON,
  COMMA,
  INJECT,
  // data operation
  ASSIGN,
  CALL_SYSTEM,
  COPY_ASSIGN,
  CLONE_ASSIGN,
  MOVE_ASSIGN,
  DOT,
  ARROW,
  STATIC_ACCESS,
  UNDERSCORE,
  SPACE,
  TURBO_FISH,
  RUN_SYSTEM,
  OP,
  SOME,
  NONE,
  OK,
  ERR,
  // logical gates
  AND,
  NAND,
  OR,
  XOR,
  NOR,
  XNOR,
  NOT,
  // binary gates
  B_AND,
  B_NAND,
  B_OR,
  B_XOR,
  B_NOR,
  B_XNOR,
  B_NOT,
  // comparators

  // ==
  OP_EQ,
  // !=
  OP_NEQ,
  // >=
  OP_GEQ,
  // <=
  OP_LEQ,
  // ===
  OP_EQS,
  // !==
  OP_NEQS,
  // math operation
  OP_PLUS,
  OP_MINUS,
  OP_ASTERISK,
  OP_POWER,
  OP_CIRCUMFLEX,
  OP_DIVIDE,
  OP_MODULO,
  OP_QUOTIEN,
  OP_REMAIN,
  OP_DIVREM,
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
  ASSIGN_FORWARD,
  ASSIGN_BACKWARD,
  // bit operation
  SHIFT_LEFT_0,
  SHIFT_RIGHT_0,
  SHIFT_LEFT_1,
  SHIFT_RIGHT_1,
  SHIFT_LEFT_A,
  SHIFT_RIGHT_A,
  ROTATE_LEFT,
  ROTATE_RIGHT,
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
  THROW,
  TRY,
  CATCH,
  // string
  QUOTE,
  // range
  RANGE,
  RANGE_INCLUSIVE,
  // other
  PERCENTAGE,
};

using TokTy = ETokenType;

const std::map<std::string, ETokenType> k_keywords = {
    // mod key
    {"mod",       ETokenType::MOD             },
    {"super::",   ETokenType::SUPER_MOD       },
    {"self::",    ETokenType::SELF_MOD        },
    // type keys
    {"type",      ETokenType::TYPE            },
    {"$",         ETokenType::DOLLAR          },
    {"?",         ETokenType::INTERROGATIVE   },
    {"u0",        ETokenType::T_U0            },
    {"bool",      ETokenType::T_BOOL          },
    {"cune",      ETokenType::T_CUNE          },
    {"rune",      ETokenType::T_RUNE          },
    {"fsize",     ETokenType::T_FSIZE         },
    {"isize",     ETokenType::T_ISIZE         },
    {"usize",     ETokenType::T_USIZE         },
    {"i8",        ETokenType::T_I8            },
    {"u8",        ETokenType::T_U8            },
    {"b8",        ETokenType::T_B8            },
    {"i16",       ETokenType::T_I16           },
    {"u16",       ETokenType::T_U16           },
    {"b16",       ETokenType::T_B16           },
    {"i32",       ETokenType::T_I32           },
    {"u32",       ETokenType::T_U32           },
    {"b32",       ETokenType::T_B32           },
    {"i64",       ETokenType::T_I64           },
    {"u64",       ETokenType::T_U64           },
    {"b64",       ETokenType::T_B64           },
    {"i128",      ETokenType::T_I64           },
    {"u128",      ETokenType::T_U64           },
    {"b128",      ETokenType::T_B128          },
    {"f16",       ETokenType::T_F16           },
    {"f32",       ETokenType::T_F32           },
    {"f64",       ETokenType::T_F64           },
    {"f80",       ETokenType::T_F80           },
    {"f128",      ETokenType::T_F128          },
    {"c_str",     ETokenType::T_C_STRING      },
    {"str",       ETokenType::T_STRING        },
    {"text",      ETokenType::T_TEXT          },
    {"dsize",     ETokenType::T_DSIZE         },
    {"d32",       ETokenType::T_D32           },
    {"d64",       ETokenType::T_D64           },
    {"d128",      ETokenType::T_D128          },
    {"udsize",    ETokenType::T_UDSIZE        },
    {"ud32",      ETokenType::T_UD32          },
    {"ud64",      ETokenType::T_UD64          },
    {"ud128",     ETokenType::T_UD128         },
    {"ptrdiff",   ETokenType::T_PTRDIFF       },
    // metacode or static table key
    {"#",         ETokenType::HASHTAG         },
    {"@",         ETokenType::AT              },
    {"std:",      ETokenType::STD_LIB         },
    {"pkg:",      ETokenType::PKG_LIB         },
    {"usr:",      ETokenType::USR_LIB         },
    {"bind:",     ETokenType::BIND_LIB        },
    // boolean litteral key
    {"true",      ETokenType::TRUE            },
    {"false",     ETokenType::FALSE           },
    // variable declaration
    {"let",       ETokenType::LET             },
    {"var",       ETokenType::VAR             },
    {"const",     ETokenType::CONST           },
    // cast
    {"as!",       ETokenType::AS_REINTERPRET  },
    {"as?",       ETokenType::AS_SAFE         },
    {"as",        ETokenType::AS              },
    {"cast",      ETokenType::CAST            },
    // typeid checking
    {"is",        ETokenType::IS              },
    {"nis",       ETokenType::NIS             },
    {"!is",       ETokenType::NIS             },
    // class keys
    {"entity",    ETokenType::ENTITY          },
    {"::{",       ETokenType::ENTITY_START_LIT},
    {"role",      ETokenType::ROLE            },
    {"comp",      ETokenType::COMPONENT       },
    {"enum",      ETokenType::ENUM            },
    {"flag",      ETokenType::FLAG            },
    {"union",     ETokenType::UNION           },
    {"sys",       ETokenType::SYSTEM          },
    {"self",      ETokenType::SELF            },
    // execution from trait
    {"~",         ETokenType::TILDE           },
    // generic keys
    {"gen",       ETokenType::GENERIC         },
    {"import",    ETokenType::IMPORT          },
    {"export",    ETokenType::EXPORT          },
    {"reexport",  ETokenType::REEXPORT        },
    {"extern",    ETokenType::EXTERN          },
    {"use",       ETokenType::USE             },
    // function keys
    {"fn",        ETokenType::FUNCTION        },
    {"lam",       ETokenType::LAMBDA          },
    {"|",         ETokenType::PIPE            },
    {"<-|",       ETokenType::PIPE_MUT        },

    // pointers
    {"ptr",       ETokenType::PTR             },
    {"sptr",      ETokenType::SPTR            },
    {"uptr",      ETokenType::UPTR            },
    {"wptr",      ETokenType::WPTR            },
    {"'at(",      ETokenType::PTR_AT          },
    {"'offset(",  ETokenType::PTR_OFFSET      },
    {"new",       ETokenType::NEW             },
    {"del",       ETokenType::DEL             },
    {"drop",      ETokenType::DROP            },
    // exception keys
    {"try",       ETokenType::TRY             },
    {"catch",     ETokenType::CATCH           },
    {"throw",     ETokenType::THROW           },
    // section keys
    {"(",         ETokenType::OPEN_PAREN      },
    {")",         ETokenType::CLOSE_PAREN     },
    {"{",         ETokenType::OPEN_BRACE      },
    {"}",         ETokenType::CLOSE_BRACE     },
    {"<",         ETokenType::OPEN_BRACKETS   },
    {">",         ETokenType::CLOSE_BRACKETS  },
    {"[",         ETokenType::OPEN_SQUARE     },
    {"]",         ETokenType::CLOSE_SQUARE    },
    // separator keys
    {":",         ETokenType::COLON           },
    {";",         ETokenType::SEMICOLON       },
    {",",         ETokenType::COMMA           },
    {"=>",        ETokenType::INJECT          },
    // assingation key
    {"=",         ETokenType::ASSIGN          },
    {"move=",     ETokenType::MOVE_ASSIGN     },
    {"copy=",     ETokenType::COPY_ASSIGN     },
    {"clone=",    ETokenType::CLONE_ASSIGN    },
    // variable pass mode
    {"mut'",      ETokenType::CAPA_MUT_OF     },
    {"ref'",      ETokenType::CAPA_REF_OF     },
    {"size'",     ETokenType::SIZE_OF         },
    {"val'",      ETokenType::VAL_OF          },
    {"addr'",     ETokenType::ADDR_OF         },
    {"move'",     ETokenType::CAPA_MOVE_OF    },
    {"copy'",     ETokenType::CAPA_COPY_OF    },
    {"clone'",    ETokenType::CAPA_CLONE_OF   },
    {"<->",       ETokenType::MEM_DIST        },
    {"ref",       ETokenType::CAPA_REF        },
    {"addr",      ETokenType::ADDR            },
    {"move",      ETokenType::CAPA_MOVE       },
    {"mut",       ETokenType::CAPA_MUT        },
    {"copy",      ETokenType::CAPA_COPY       },
    {"clone",     ETokenType::CAPA_CLONE      },
    {"sys",       ETokenType::SYSTEM          },
    {"with",      ETokenType::WITH            },
    {"on",        ETokenType::ON              },
    // access keys
    {"'",         ETokenType::TICK            },
    {"->",        ETokenType::ARROW           },
    {"::",        ETokenType::STATIC_ACCESS   },
    {"::<",       ETokenType::TURBO_FISH      },
    // avoid key
    {"_",         ETokenType::UNDERSCORE      },
    {"::>",       ETokenType::CALL_SYSTEM     },
    // operator overloading
    {"op",        ETokenType::OP              },
    {"Some",      ETokenType::SOME            },
    {"None",      ETokenType::NONE            },
    {"Ok",        ETokenType::OK              },
    {"Err",       ETokenType::ERR             },
    // logical and bitwise operators keys
    {"and",       ETokenType::AND             },
    {"!and",      ETokenType::NAND            },
    {"nand",      ETokenType::NAND            },
    {"or",        ETokenType::OR              },
    {"!or",       ETokenType::NOR             },
    {"nor",       ETokenType::NOR             },
    {"xor",       ETokenType::XOR             },
    {"!xor",      ETokenType::XNOR            },
    {"xnor",      ETokenType::XNOR            },
    {"not",       ETokenType::NOT             },
    {"not.b",     ETokenType::B_NOT           },
    {"and.b",     ETokenType::B_AND           },
    {"!and.b",    ETokenType::B_NAND          },
    {"nand.b",    ETokenType::B_NAND          },
    {"or.b",      ETokenType::B_OR            },
    {"xor.b",     ETokenType::B_XOR           },
    {"!or.b",     ETokenType::B_NOR           },
    {"nor.b",     ETokenType::B_NOR           },
    {"!xor.b",    ETokenType::B_XNOR          },
    {"xnor.b",    ETokenType::B_XNOR          },
    {"!",         ETokenType::NOT             },
    // comparison keys
    {"==",        ETokenType::OP_EQ           },
    {"===",       ETokenType::OP_EQS          },
    {"!=",        ETokenType::OP_NEQ          },
    {"!==",       ETokenType::OP_NEQS         },
    {">=",        ETokenType::OP_GEQ          },
    {"<=",        ETokenType::OP_LEQ          },
    {"+",         ETokenType::OP_PLUS         },
    {"-",         ETokenType::OP_MINUS        },
    {"*",         ETokenType::OP_ASTERISK     },
    {"**",        ETokenType::OP_POWER        },
    {"^",         ETokenType::OP_CIRCUMFLEX   },
    {"/",         ETokenType::OP_DIVIDE       },
    {"%mod%",     ETokenType::OP_MODULO       },
    {"%quo%",     ETokenType::OP_QUOTIEN      },
    {"%rem%",     ETokenType::OP_REMAIN       },
    {"%divrem%",  ETokenType::OP_DIVREM       },
    {"+=",        ETokenType::ASSIGN_PLUS     },
    {"-=",        ETokenType::ASSIGN_MINUS    },
    {"*=",        ETokenType::ASSIGN_MULTIPLY },
    {"**=",       ETokenType::ASSIGN_POWER    },
    {"/=",        ETokenType::ASSIGN_DIVIDE   },
    {"%mod%=",    ETokenType::ASSIGN_MODULO   },
    {"%quo%=",    ETokenType::ASSIGN_QUOTIEN  },
    {"%rem%=",    ETokenType::ASSIGN_REMAIN   },
    {"%divrem%=", ETokenType::ASSIGN_DIVREM   },
    {"<<[0]",     ETokenType::SHIFT_LEFT_0    },
    {"[0]>>",     ETokenType::SHIFT_RIGHT_0   },
    {"<<[1]",     ETokenType::SHIFT_LEFT_1    },
    {"[1]>>",     ETokenType::SHIFT_RIGHT_1   },
    {"<<[a]",     ETokenType::SHIFT_LEFT_A    },
    {"[a]>>",     ETokenType::SHIFT_RIGHT_A   },
    {"<<[r]",     ETokenType::ROTATE_LEFT     },
    {"[r]>>",     ETokenType::ROTATE_RIGHT    },
    // statement keys
    {"if",        ETokenType::IF              },
    {"else",      ETokenType::ELSE            },
    {"elif",      ETokenType::ELIF            },
    {"for",       ETokenType::FOR             },
    {"in",        ETokenType::IN              },
    {"!in",       ETokenType::NIN             },
    {"nin",       ETokenType::NIN             },
    {"step",      ETokenType::STEP            },
    // while keys
    {"while",     ETokenType::WHILE           },
    {"do",        ETokenType::DO_WHILE        },
    {"loop",      ETokenType::LOOP            },
    // match keys
    {"match",     ETokenType::MATCH           },
    // flow keys
    {"break",     ETokenType::BREAK           },
    {"continue",  ETokenType::CONTINUE        },
    {"return",    ETokenType::RETURN          },
    {"goto",      ETokenType::GOTO            },
    {"label",     ETokenType::GOTO_LABEL      },
    // literal string key
    {"\"",        ETokenType::QUOTE           },
    // variadic def/inst
    {"...",       ETokenType::VARIADIC        },
    // range keys
    {"..",        ETokenType::RANGE           },
    {"..=",       ETokenType::RANGE_INCLUSIVE },
    // punctuation
    {".",         ETokenType::DOT             },
    {"::>",       ETokenType::RUN_SYSTEM      },
    {"%",         ETokenType::PERCENTAGE      },
};

const std::initializer_list<ETokenType> k_variable = {
    ETokenType::LET,
    ETokenType::VAR,
    ETokenType::CONST,
};

const std::initializer_list<ETokenType> k_start_identifier = {ETokenType::STATIC_ACCESS, ETokenType::IDENTIFIER,
                                                              ETokenType::SUPER_MOD, ETokenType::SELF_MOD};

const std::initializer_list<ETokenType> k_access = {ETokenType::STATIC_ACCESS, ETokenType::DOT};

const std::initializer_list<ETokenType> k_parameter_passmode = {ETokenType::CAPA_MUT,  ETokenType::CAPA_REF,
                                                                ETokenType::CAPA_COPY, ETokenType::CAPA_CLONE,
                                                                ETokenType::ADDR,      ETokenType::CAPA_MOVE};

const std::initializer_list<ETokenType> k_capability = {
    ETokenType::CAPA_MUT,
    ETokenType::CAPA_REF,
};

const std::initializer_list<ETokenType> k_expression_passmode = {
    ETokenType::CAPA_MUT_OF,   ETokenType::CAPA_REF_OF,  ETokenType::CAPA_COPY_OF,
    ETokenType::CAPA_CLONE_OF, ETokenType::CAPA_MOVE_OF,
};

const std::initializer_list<ETokenType> k_cast = {
    ETokenType::AS,
    ETokenType::AS_REINTERPRET,
    ETokenType::AS_SAFE,
};

const std::initializer_list<ETokenType> k_operator = {
    ETokenType::OP_PLUS,
    ETokenType::OP_MINUS,
    ETokenType::OP_ASTERISK,
    ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,
    ETokenType::OP_MODULO,
    ETokenType::OP_QUOTIEN,
    ETokenType::OP_REMAIN,
    ETokenType::OP_DIVREM,
    ETokenType::OP_EQ,
    ETokenType::OP_NEQ,
    ETokenType::OP_EQS,
    ETokenType::OP_NEQS,
    ETokenType::OP_GEQ,
    ETokenType::OP_LEQ,
    ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS,
    ETokenType::AND,
    ETokenType::NAND,
    ETokenType::OR,
    ETokenType::XOR,
    ETokenType::NOR,
    ETokenType::XNOR,
    ETokenType::NOT,
    ETokenType::B_AND,
    ETokenType::B_NAND,
    ETokenType::B_OR,
    ETokenType::B_XOR,
    ETokenType::B_NOR,
    ETokenType::B_XNOR,
    ETokenType::B_NOT,
    ETokenType::SHIFT_LEFT_0,
    ETokenType::SHIFT_RIGHT_0,
    ETokenType::SHIFT_LEFT_1,
    ETokenType::SHIFT_RIGHT_1,
    ETokenType::SHIFT_LEFT_A,
    ETokenType::SHIFT_RIGHT_A,
    ETokenType::ROTATE_LEFT,
    ETokenType::ROTATE_RIGHT,
};

const std::initializer_list<ETokenType> k_op_bitwise_shift = {
    ETokenType::SHIFT_LEFT_0, ETokenType::SHIFT_RIGHT_0, ETokenType::SHIFT_LEFT_1, ETokenType::SHIFT_RIGHT_1,
    ETokenType::SHIFT_LEFT_A, ETokenType::SHIFT_RIGHT_A, ETokenType::ROTATE_LEFT,  ETokenType::ROTATE_RIGHT,
};

const std::initializer_list<ETokenType> k_op_bitwise = {
    ETokenType::B_AND,         ETokenType::B_NAND,       ETokenType::B_OR,          ETokenType::B_XOR,
    ETokenType::B_NOR,         ETokenType::B_XNOR,       ETokenType::B_NOT,         ETokenType::SHIFT_LEFT_0,
    ETokenType::SHIFT_RIGHT_0, ETokenType::SHIFT_LEFT_1, ETokenType::SHIFT_RIGHT_1, ETokenType::SHIFT_LEFT_A,
    ETokenType::SHIFT_RIGHT_A, ETokenType::ROTATE_LEFT,  ETokenType::ROTATE_RIGHT,
};

const std::initializer_list<ETokenType> k_op_comparison = {
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_EQ,  ETokenType::OP_NEQ, ETokenType::OP_EQS,
    ETokenType::OP_NEQS,       ETokenType::OP_LEQ,         ETokenType::OP_GEQ, ETokenType::IN,     ETokenType::NIN,
};
const std::initializer_list<ETokenType> kAssignationTokens = {
    ETokenType::MOVE_ASSIGN,   ETokenType::COPY_ASSIGN,     ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,
    ETokenType::ASSIGN_MINUS,  ETokenType::ASSIGN_MULTIPLY, ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_POWER,
    ETokenType::ASSIGN_MODULO, ETokenType::ASSIGN_QUOTIEN,  ETokenType::ASSIGN_REMAIN, ETokenType::ASSIGN_DIVREM,
};
const std::initializer_list<ETokenType> k_pointer = {
    ETokenType::PTR,
    ETokenType::UPTR,
    ETokenType::SPTR,
    ETokenType::WPTR,
};

const std::initializer_list<ETokenType> k_type_primitive = {
    ETokenType::T_U0,     ETokenType::T_BOOL,  ETokenType::T_CUNE,    ETokenType::T_C_STRING, ETokenType::T_STRING,
    ETokenType::T_RUNE,   ETokenType::T_TEXT,  ETokenType::T_USIZE,   ETokenType::L_BIN,      ETokenType::L_HEX,
    ETokenType::L_OCT,    ETokenType::L_I,     ETokenType::L_D,       ETokenType::T_I8,       ETokenType::T_I16,
    ETokenType::T_I32,    ETokenType::T_I64,   ETokenType::T_I128,    ETokenType::T_ISIZE,    ETokenType::T_U8,
    ETokenType::T_U16,    ETokenType::T_U32,   ETokenType::T_U64,     ETokenType::T_U128,     ETokenType::T_USIZE,
    ETokenType::T_B8,     ETokenType::T_B16,   ETokenType::T_B32,     ETokenType::T_B64,      ETokenType::T_B128,
    ETokenType::T_BSIZE,  ETokenType::T_F16,   ETokenType::T_F32,     ETokenType::T_F64,      ETokenType::T_F80,
    ETokenType::T_F128,   ETokenType::T_FSIZE, ETokenType::T_PTRDIFF, ETokenType::T_D32,      ETokenType::T_D64,
    ETokenType::T_D128,   ETokenType::T_DSIZE, ETokenType::T_UD32,    ETokenType::T_UD64,     ETokenType::T_UD128,
    ETokenType::T_UDSIZE,
};
const std::initializer_list<ETokenType> k_type_integral = {
    ETokenType::T_I8, ETokenType::T_I16, ETokenType::T_I32, ETokenType::T_I64, ETokenType::T_I128, ETokenType::T_ISIZE,
    ETokenType::T_U8, ETokenType::T_U16, ETokenType::T_U32, ETokenType::T_U64, ETokenType::T_U128, ETokenType::T_USIZE,
    ETokenType::T_B8, ETokenType::T_B16, ETokenType::T_B32, ETokenType::T_B64, ETokenType::T_B128, ETokenType::T_BSIZE,
};
const std::initializer_list<ETokenType> k_type_integral_signed = {
    ETokenType::T_I8, ETokenType::T_I16, ETokenType::T_I32, ETokenType::T_I64, ETokenType::T_I128, ETokenType::T_ISIZE,
};
const std::initializer_list<ETokenType> k_type_integra_unsigned = {
    ETokenType::T_U8, ETokenType::T_U16, ETokenType::T_U32, ETokenType::T_U64, ETokenType::T_U128, ETokenType::T_USIZE,
};
const std::initializer_list<ETokenType> k_type_integral_binary = {
    ETokenType::T_B8, ETokenType::T_B16, ETokenType::T_B32, ETokenType::T_B64, ETokenType::T_B128, ETokenType::T_BSIZE,
};
const std::initializer_list<ETokenType> k_type_floating_point = {
    ETokenType::T_F16, ETokenType::T_F32, ETokenType::T_F64, ETokenType::T_F80, ETokenType::T_F128, ETokenType::T_FSIZE,
};
const std::initializer_list<ETokenType> k_type_numeric = {
    ETokenType::L_BIN,   ETokenType::L_HEX,   ETokenType::L_OCT,     ETokenType::L_I,   ETokenType::L_D,
    ETokenType::T_I8,    ETokenType::T_I16,   ETokenType::T_I32,     ETokenType::T_I64, ETokenType::T_I128,
    ETokenType::T_ISIZE, ETokenType::T_U8,    ETokenType::T_U16,     ETokenType::T_U32, ETokenType::T_U64,
    ETokenType::T_U128,  ETokenType::T_USIZE, ETokenType::T_B8,      ETokenType::T_B16, ETokenType::T_B32,
    ETokenType::T_B64,   ETokenType::T_B128,  ETokenType::T_BSIZE,   ETokenType::T_F32, ETokenType::T_F64,
    ETokenType::T_F128,  ETokenType::T_FSIZE, ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> k_type_fixed_point = {
    ETokenType::T_D32,  ETokenType::T_D64,  ETokenType::T_D128,  ETokenType::T_DSIZE,
    ETokenType::T_UD32, ETokenType::T_UD64, ETokenType::T_UD128, ETokenType::T_UDSIZE,
};
const std::initializer_list<ETokenType> k_type_boolean = {ETokenType::TRUE, ETokenType::FALSE, ETokenType::T_BOOL};

const std::initializer_list<ETokenType> k_op_assign = {
    ETokenType::ASSIGN_PLUS,    ETokenType::ASSIGN_MINUS,  ETokenType::ASSIGN_MULTIPLY,
    ETokenType::ASSIGN_DIVIDE,  ETokenType::ASSIGN_POWER,  ETokenType::ASSIGN_MODULO,
    ETokenType::ASSIGN_QUOTIEN, ETokenType::ASSIGN_REMAIN, ETokenType::ASSIGN_DIVREM};

const std::initializer_list<ETokenType> k_op_arithmetic = {
    ETokenType::OP_PLUS,   ETokenType::OP_MINUS,   ETokenType::OP_ASTERISK, ETokenType::OP_DIVIDE, ETokenType::OP_POWER,
    ETokenType::OP_MODULO, ETokenType::OP_QUOTIEN, ETokenType::OP_REMAIN,   ETokenType::OP_DIVREM};

const std::initializer_list<ETokenType> k_text_interpolation = {
    ETokenType::S_INTERPOLATION_START,
    ETokenType::S_INTERPOLATION_END,
};
const std::initializer_list<ETokenType> k_type = {
    ETokenType::T_I8,       ETokenType::T_I16,     ETokenType::T_I32,   ETokenType::T_I64,    ETokenType::T_I128,
    ETokenType::T_ISIZE,    ETokenType::T_U8,      ETokenType::T_U16,   ETokenType::T_U32,    ETokenType::T_U64,
    ETokenType::T_U128,     ETokenType::T_USIZE,   ETokenType::T_B8,    ETokenType::T_B16,    ETokenType::T_B32,
    ETokenType::T_B64,      ETokenType::T_B128,    ETokenType::T_BSIZE, ETokenType::T_F16,    ETokenType::T_F32,
    ETokenType::T_F64,      ETokenType::T_F80,     ETokenType::T_F128,  ETokenType::T_FSIZE,  ETokenType::T_BOOL,
    ETokenType::T_RUNE,     ETokenType::T_CUNE,    ETokenType::T_D32,   ETokenType::T_D64,    ETokenType::T_D128,
    ETokenType::T_DSIZE,    ETokenType::T_UD32,    ETokenType::T_UD64,  ETokenType::T_UD128,  ETokenType::T_UDSIZE,
    ETokenType::T_C_STRING, ETokenType::T_STRING,  ETokenType::T_TEXT,  ETokenType::FUNCTION, ETokenType::IDENTIFIER,
    ETokenType::TYPE,       ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> k_hybrid_namespace = {
    ETokenType::T_I8,       ETokenType::T_I16,     ETokenType::T_I32,   ETokenType::T_I64,     ETokenType::T_I128,
    ETokenType::T_ISIZE,    ETokenType::T_U8,      ETokenType::T_U16,   ETokenType::T_U32,     ETokenType::T_U64,
    ETokenType::T_U128,     ETokenType::T_USIZE,   ETokenType::T_B8,    ETokenType::T_B16,     ETokenType::T_B32,
    ETokenType::T_B64,      ETokenType::T_B128,    ETokenType::T_BSIZE, ETokenType::T_F16,     ETokenType::T_F32,
    ETokenType::T_F64,      ETokenType::T_F80,     ETokenType::T_F128,  ETokenType::T_FSIZE,   ETokenType::T_BOOL,
    ETokenType::T_RUNE,     ETokenType::T_CUNE,    ETokenType::T_D32,   ETokenType::T_D64,     ETokenType::T_D128,
    ETokenType::T_DSIZE,    ETokenType::T_UD32,    ETokenType::T_UD64,  ETokenType::T_UD128,   ETokenType::T_UDSIZE,
    ETokenType::T_C_STRING, ETokenType::T_STRING,  ETokenType::T_TEXT,  ETokenType::FUNCTION,  ETokenType::IDENTIFIER,
    ETokenType::TYPE,       ETokenType::T_U0,      ETokenType::CAST,    ETokenType::FUNCTION,  ETokenType::LAMBDA,
    ETokenType::FLAG,       ETokenType::PTR,       ETokenType::UPTR,    ETokenType::SPTR,      ETokenType::WPTR,
    ETokenType::ENTITY,     ETokenType::COMPONENT, ETokenType::GENERIC, ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> k_op_logical = {ETokenType::AND, ETokenType::NAND, ETokenType::OR,
                                                        ETokenType::XOR, ETokenType::NOR,  ETokenType::XNOR,
                                                        ETokenType::NOT};

const std::initializer_list<ETokenType> k_args_ending = {
    ETokenType::CLOSE_BRACKETS, ETokenType::CLOSE_BRACE, ETokenType::OPEN_BRACE, ETokenType::CLOSE_PAREN,
    ETokenType::CLOSE_SQUARE,   ETokenType::SEMICOLON,   ETokenType::ASSIGN,     ETokenType::PIPE,
    ETokenType::PIPE_MUT,       ETokenType::SEMICOLON,   ETokenType::METACODE,
};
const std::initializer_list<ETokenType> k_args_delimitation = {
    ETokenType::CLOSE_BRACKETS, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_PAREN, ETokenType::OPEN_PAREN,
    ETokenType::CLOSE_SQUARE,   ETokenType::OPEN_SQUARE,   ETokenType::SEMICOLON,   ETokenType::PIPE};
const std::initializer_list<ETokenType> k_args_generic_valid = {
    ETokenType::T_I8,          ETokenType::T_I16,          ETokenType::T_I32,      ETokenType::T_I64,
    ETokenType::T_I128,        ETokenType::T_ISIZE,        ETokenType::T_U8,       ETokenType::T_U16,
    ETokenType::T_U32,         ETokenType::T_U64,          ETokenType::T_U128,     ETokenType::T_USIZE,
    ETokenType::T_B8,          ETokenType::T_B16,          ETokenType::T_B32,      ETokenType::T_B64,
    ETokenType::T_B128,        ETokenType::T_BSIZE,        ETokenType::T_F32,      ETokenType::T_F64,
    ETokenType::T_F128,        ETokenType::T_FSIZE,        ETokenType::DOLLAR,     ETokenType::INTERROGATIVE,
    ETokenType::IDENTIFIER,    ETokenType::STATIC_ACCESS,  ETokenType::PTR,        ETokenType::UPTR,
    ETokenType::SPTR,          ETokenType::WPTR,           ETokenType::TICK,       ETokenType::HASHTAG,
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::IDENTIFIER, ETokenType::T_BOOL,
    ETokenType::T_RUNE,        ETokenType::T_CUNE,         ETokenType::T_D32,      ETokenType::T_D64,
    ETokenType::T_D128,        ETokenType::T_DSIZE,        ETokenType::T_UD32,     ETokenType::T_UD64,
    ETokenType::T_UD128,       ETokenType::T_UDSIZE,       ETokenType::T_STRING,   ETokenType::T_TEXT,
    ETokenType::FUNCTION,      ETokenType::IDENTIFIER,     ETokenType::TYPE,       ETokenType::COMMA,
    ETokenType::LET,           ETokenType::T_PTRDIFF,
};

const std::initializer_list<ETokenType> k_lit = {
    ETokenType::IDENTIFIER, ETokenType::L_BIN,  ETokenType::L_OCT,     ETokenType::L_HEX,
    ETokenType::L_I,        ETokenType::L_D,    ETokenType::L_TEXTUAL, ETokenType::TRUE,
    ETokenType::FALSE,      ETokenType::L_CUNE, ETokenType::L_ARRAY,
};
const std::initializer_list<ETokenType> k_invalid_tokens = {
    ETokenType::METACODE,
    ETokenType::S_METACODE_END,
    ETokenType::S_END_OF_FILE,
};

const std::initializer_list<ETokenType> k_op_boolean = {
    ETokenType::AND,    ETokenType::OR,    ETokenType::NOR,    ETokenType::XOR,   ETokenType::XNOR,  ETokenType::NAND,
    ETokenType::OP_NEQ, ETokenType::B_AND, ETokenType::B_OR,   ETokenType::B_NOR, ETokenType::B_XOR, ETokenType::B_XNOR,
    ETokenType::B_NAND, ETokenType::OP_EQ, ETokenType::ASSIGN, ETokenType::NOT,   ETokenType::B_NOT,
};
const std::initializer_list<ETokenType> k_op_integral = {
    ETokenType::OP_PLUS,         ETokenType::OP_MINUS,      ETokenType::OP_ASTERISK,   ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,       ETokenType::OP_MODULO,     ETokenType::OP_QUOTIEN,    ETokenType::OP_REMAIN,
    ETokenType::OP_DIVREM,       ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,   ETokenType::ASSIGN_MINUS,
    ETokenType::ASSIGN_MULTIPLY, ETokenType::ASSIGN_POWER,  ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_MODULO,
    ETokenType::ASSIGN_QUOTIEN,  ETokenType::ASSIGN_REMAIN, ETokenType::ASSIGN_DIVREM, ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS,  ETokenType::OP_GEQ,        ETokenType::OP_LEQ,        ETokenType::OP_EQ,
    ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> k_op_numeric = {
    ETokenType::OP_PLUS,         ETokenType::OP_MINUS,      ETokenType::OP_ASTERISK,   ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,       ETokenType::OP_MODULO,     ETokenType::OP_QUOTIEN,    ETokenType::OP_REMAIN,
    ETokenType::OP_DIVREM,       ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,   ETokenType::ASSIGN_MINUS,
    ETokenType::ASSIGN_MULTIPLY, ETokenType::ASSIGN_POWER,  ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_MODULO,
    ETokenType::ASSIGN_QUOTIEN,  ETokenType::ASSIGN_REMAIN, ETokenType::ASSIGN_DIVREM, ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS,  ETokenType::OP_GEQ,        ETokenType::OP_LEQ,        ETokenType::OP_EQ,
    ETokenType::OP_NEQ,          ETokenType::OP_EQS,        ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> k_op_char = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,  ETokenType::OP_LEQ,
    ETokenType::OP_EQ,  ETokenType::OP_NEQ,        ETokenType::OP_EQS,         ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> k_op_text = {
    ETokenType::OP_PLUS,        ETokenType::ASSIGN, ETokenType::ASSIGN_PLUS, ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ, ETokenType::OP_LEQ,      ETokenType::OP_EQ,
    ETokenType::OP_NEQ,         ETokenType::OP_EQS, ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> k_op_enum = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> k_op_address = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> k_op_array = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> k_op_format = {
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_CIRCUMFLEX,
    ETokenType::TILDE,         ETokenType::ASSIGN,
};

const std::initializer_list<ETokenType> k_identifier_possible = {
    // mod key
    ETokenType::MOD,
    // type keys
    ETokenType::TYPE, ETokenType::T_U0, ETokenType::T_BOOL, ETokenType::ENUM, ETokenType::T_RUNE, ETokenType::T_CUNE,
    ETokenType::T_ISIZE, ETokenType::T_USIZE, ETokenType::T_BSIZE, ETokenType::T_FSIZE, ETokenType::T_I8,
    ETokenType::T_U8, ETokenType::T_B8, ETokenType::T_I16, ETokenType::T_U16, ETokenType::T_B16, ETokenType::T_I32,
    ETokenType::T_U32, ETokenType::T_B32, ETokenType::T_I64, ETokenType::T_U64, ETokenType::T_B64, ETokenType::T_I128,
    ETokenType::T_U128, ETokenType::T_B128, ETokenType::T_F16, ETokenType::T_F32, ETokenType::T_F64, ETokenType::T_F80,
    ETokenType::T_F128, ETokenType::T_C_STRING, ETokenType::T_STRING, ETokenType::T_TEXT, ETokenType::T_D32,
    ETokenType::T_D64, ETokenType::T_D128, ETokenType::T_DSIZE, ETokenType::T_UD32, ETokenType::T_UD64,
    ETokenType::T_UD128, ETokenType::T_UDSIZE, ETokenType::T_TEXT, ETokenType::T_PTRDIFF,
    // boolean
    // literals
    ETokenType::TRUE, ETokenType::FALSE,
    // variable
    // declaration
    ETokenType::LET, ETokenType::VAR, ETokenType::CONST,
    // typeid
    // checking
    ETokenType::IS,
    // class keys
    ETokenType::ENTITY, ETokenType::ROLE, ETokenType::COMPONENT, ETokenType::SYSTEM, ETokenType::SELF,
    // generic keys
    ETokenType::GENERIC, ETokenType::USE,
    // function keys
    ETokenType::FUNCTION, ETokenType::LAMBDA,
    // pointers
    ETokenType::PTR, ETokenType::SPTR, ETokenType::UPTR, ETokenType::WPTR, ETokenType::NEW, ETokenType::DEL,
    // exception keys
    ETokenType::TRY, ETokenType::CATCH, ETokenType::THROW,
    // variable pass mode
    ETokenType::CAPA_REF, ETokenType::ADDR, ETokenType::CAPA_MOVE, ETokenType::WITH, ETokenType::ON,
    // avoid key
    ETokenType::CALL_SYSTEM,
    // operator overloading
    ETokenType::OP, ETokenType::SOME, ETokenType::NONE, ETokenType::OK, ETokenType::ERR,
    // logical operators
    ETokenType::AND, ETokenType::NAND, ETokenType::OR, ETokenType::NOR, ETokenType::XOR, ETokenType::XNOR,
    ETokenType::NOT,
    // statement keys
    ETokenType::IF, ETokenType::ELSE, ETokenType::FOR, ETokenType::IN, ETokenType::NIN, ETokenType::STEP,
    // while keys
    ETokenType::WHILE, ETokenType::DO_WHILE, ETokenType::LOOP,
    // match keys
    ETokenType::MATCH,
    // flow keys
    ETokenType::BREAK, ETokenType::CONTINUE, ETokenType::RETURN,
    // identifier
    ETokenType::IDENTIFIER};

struct Span {
  Span() = default;

  // postprocess_position
  uint32_t pos             = 1;
  // original tokenizer position
  uint32_t anteprocess_pos = 1;
  uint16_t line            = 1;
  uint16_t col             = 1;
  uint16_t size            = 1;

  Span(size_t pos, size_t line, size_t col, size_t size)
    : pos(pos)
    , line(line)
    , col(col)
    , size(size)
  {
  }

  bool operator==(const Span& other) const
  {
    return anteprocess_pos == other.anteprocess_pos && pos == other.pos && line == other.line && col == other.col
           && size == other.size;
  }
};

struct Token {
  Token() = default;

  std::string val;
  ETokenType  type = ETokenType::UNKNOWN;
  Span        span;
  bool        debug_end_of_line = false;

  Token(const std::string& val, ETokenType type, Span span)
    : val(val)
    , type(type)
    , span(span)
  {
  }

  std::string display() const
  {
    if (!val.empty()) return val;

    switch (type) {
    case ETokenType::METACODE:               return "#";
    case ETokenType::S_METACODE_PLACEHOLDER: return "[[" + val + "]]";
    default:                                 break;
    }
    return "";
  }
};


inline bool str_is_identifier(const std::string& s)
{
  if (s.empty()) return false;
  if (!std::isalpha(s[0]) && s[0] != '_') return false;
  for (size_t i = 1; i < s.size(); ++i)
    if (!std::isalnum(s[i]) && s[i] != '_') return false;
  return true;
}

inline bool isKeywordChar(char ch)
{
  return !std::isspace(ch) && !std::iscntrl(ch);
}

inline ETokenType Str_to_ETokenType(const std::string& str)
{
  if (auto it = k_keywords.find(str); it != k_keywords.end()) return it->second;
  return ETokenType::UNKNOWN;
}
