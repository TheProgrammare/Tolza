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
  T_VOID,
  T_BOOL,
  // 8 bits
  T_ASCII,
  // 32 bits
  T_UTF32,
  ENUM,
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
  T_F32,
  T_F64,
  T_F128,
  T_FSIZE,
  T_DECIMAL,
  T_UDECIMAL,
  // fat pointer of ASCII
  T_STRING,
  // fat pointer of UTF32
  T_TEXT,
  T_ARRAY,
  T_TUPLE,
  HASHTAG,
  AT,
  STD_LIB,
  THIRD_LIB,
  USR_LIB,
  EXT_LIB,
  // literal values
  L_BIN,
  L_OCT,
  L_HEX,
  L_I,
  L_U,
  L_F,
  L_DECIMAL,
  L_UDECIMAL,
  // format
  S_TEXTUAL_EXPR_START,
  S_TEXTUAL_EXPR_END,
  // text
  L_TEXTUAL,
  TRUE,
  FALSE,
  L_ASCII,
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
  FLAG,
  GENERIC,
  IMPORT,
  EXPORT,
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
  // math affectation
  ASSIGN_PLUS,
  ASSIGN_MINUS,
  ASSIGN_MULTIPLY,
  ASSIGN_POWER,
  ASSIGN_DIVIDE,
  ASSIGN_MODULO,
  ASSIGN_QUOTIEN,
  ASSIGN_REMAIN,
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

const std::map<std::string, ETokenType> kKeywords = {
    // mod key
    {"mod",      ETokenType::MOD             },
    {"super::",  ETokenType::SUPER_MOD       },
    {"self::",   ETokenType::SELF_MOD        },
    // type keys
    {"type",     ETokenType::TYPE            },
    {"$",        ETokenType::DOLLAR          },
    {"?",        ETokenType::INTERROGATIVE   },
    {"void",     ETokenType::T_VOID          },
    {"u0",       ETokenType::T_VOID          },
    {"bool",     ETokenType::T_BOOL          },
    {"enum",     ETokenType::ENUM            },
    {"ascii",    ETokenType::T_ASCII         },
    {"utf32",    ETokenType::T_UTF32         },
    {"isize",    ETokenType::T_ISIZE         },
    {"usize",    ETokenType::T_USIZE         },
    {"i8",       ETokenType::T_I8            },
    {"u8",       ETokenType::T_U8            },
    {"b8",       ETokenType::T_B8            },
    {"i16",      ETokenType::T_I16           },
    {"u16",      ETokenType::T_U16           },
    {"b16",      ETokenType::T_B16           },
    {"i32",      ETokenType::T_I32           },
    {"u32",      ETokenType::T_U32           },
    {"b32",      ETokenType::T_B32           },
    {"i64",      ETokenType::T_I64           },
    {"u64",      ETokenType::T_U64           },
    {"b64",      ETokenType::T_B64           },
    {"i128",     ETokenType::T_I64           },
    {"u128",     ETokenType::T_U64           },
    {"b128",     ETokenType::T_B128          },
    {"f32",      ETokenType::T_F32           },
    {"f64",      ETokenType::T_F64           },
    {"f128",     ETokenType::T_F128          },
    {"str",      ETokenType::T_STRING        },
    {"text",     ETokenType::T_TEXT          },
    {"deci",     ETokenType::T_DECIMAL       },
    {"udeci",    ETokenType::T_UDECIMAL      },
    {"ptrdiff",  ETokenType::T_PTRDIFF       },
    // metacode or static table key
    {"#",        ETokenType::HASHTAG         },
    {"@",        ETokenType::AT              },
    {"std:",     ETokenType::STD_LIB         },
    {"lib:",     ETokenType::THIRD_LIB       },
    {"usr:",     ETokenType::USR_LIB         },
    {"ext:",     ETokenType::EXT_LIB         },
    // boolean litteral key
    {"true",     ETokenType::TRUE            },
    {"false",    ETokenType::FALSE           },
    // variable declaration
    {"let",      ETokenType::LET             },
    {"var",      ETokenType::VAR             },
    {"const",    ETokenType::CONST           },
    // cast
    {"as!",      ETokenType::AS_REINTERPRET  },
    {"as?",      ETokenType::AS_SAFE         },
    {"as",       ETokenType::AS              },
    {"cast",     ETokenType::CAST            },
    // typeid checking
    {"is",       ETokenType::IS              },
    {"nis",      ETokenType::NIS             },
    {"!is",      ETokenType::NIS             },
    // class keys
    {"entity",   ETokenType::ENTITY          },
    {"::{",      ETokenType::ENTITY_START_LIT},
    {"role",     ETokenType::ROLE            },
    {"comp",     ETokenType::COMPONENT       },
    {"flag",     ETokenType::FLAG            },
    {"sys",      ETokenType::SYSTEM          },
    {"self",     ETokenType::SELF            },
    // execution from trait
    {"~",        ETokenType::TILDE           },
    // generic keys
    {"gen",      ETokenType::GENERIC         },
    {"import",   ETokenType::IMPORT          },
    {"export",   ETokenType::EXPORT          },
    {"extern",   ETokenType::EXTERN          },
    {"use",      ETokenType::USE             },
    // function keys
    {"fn",       ETokenType::FUNCTION        },
    {"lam",      ETokenType::LAMBDA          },
    {"|",        ETokenType::PIPE            },
    {"<-|",      ETokenType::PIPE_MUT        },

    // pointers
    {"ptr",      ETokenType::PTR             },
    {"sptr",     ETokenType::SPTR            },
    {"uptr",     ETokenType::UPTR            },
    {"wptr",     ETokenType::WPTR            },
    {"'at(",     ETokenType::PTR_AT          },
    {"'offset(", ETokenType::PTR_OFFSET      },
    {"new",      ETokenType::NEW             },
    {"del",      ETokenType::DEL             },
    {"drop",     ETokenType::DROP            },
    // exception keys
    {"try",      ETokenType::TRY             },
    {"catch",    ETokenType::CATCH           },
    {"throw",    ETokenType::THROW           },
    // section keys
    {"(",        ETokenType::OPEN_PAREN      },
    {")",        ETokenType::CLOSE_PAREN     },
    {"{",        ETokenType::OPEN_BRACE      },
    {"}",        ETokenType::CLOSE_BRACE     },
    {"<",        ETokenType::OPEN_BRACKETS   },
    {">",        ETokenType::CLOSE_BRACKETS  },
    {"[",        ETokenType::OPEN_SQUARE     },
    {"]",        ETokenType::CLOSE_SQUARE    },
    // separator keys
    {":",        ETokenType::COLON           },
    {";",        ETokenType::SEMICOLON       },
    {",",        ETokenType::COMMA           },
    {"=>",       ETokenType::INJECT          },
    // assingation key
    {"=",        ETokenType::ASSIGN          },
    {"move=",    ETokenType::MOVE_ASSIGN     },
    {"copy=",    ETokenType::COPY_ASSIGN     },
    {"clone=",   ETokenType::CLONE_ASSIGN    },
    // variable pass mode
    {"mut'",     ETokenType::CAPA_MUT_OF     },
    {"ref'",     ETokenType::CAPA_REF_OF     },
    {"size'",    ETokenType::SIZE_OF         },
    {"val'",     ETokenType::VAL_OF          },
    {"addr'",    ETokenType::ADDR_OF         },
    {"move'",    ETokenType::CAPA_MOVE_OF    },
    {"copy'",    ETokenType::CAPA_COPY_OF    },
    {"clone'",   ETokenType::CAPA_CLONE_OF   },
    {"<->",      ETokenType::MEM_DIST        },
    {"ref",      ETokenType::CAPA_REF        },
    {"addr",     ETokenType::ADDR            },
    {"move",     ETokenType::CAPA_MOVE       },
    {"mut",      ETokenType::CAPA_MUT        },
    {"copy",     ETokenType::CAPA_COPY       },
    {"clone",    ETokenType::CAPA_CLONE      },
    {"sys",      ETokenType::SYSTEM          },
    {"with",     ETokenType::WITH            },
    {"on",       ETokenType::ON              },
    // access keys
    {"'",        ETokenType::TICK            },
    {"->",       ETokenType::ARROW           },
    {"::",       ETokenType::STATIC_ACCESS   },
    {"::<",      ETokenType::TURBO_FISH      },
    // avoid key
    {"_",        ETokenType::UNDERSCORE      },
    {"::>",      ETokenType::CALL_SYSTEM     },
    // operator overloading
    {"op",       ETokenType::OP              },
    {"Some",     ETokenType::SOME            },
    {"None",     ETokenType::NONE            },
    {"Ok",       ETokenType::OK              },
    {"Err",      ETokenType::ERR             },
    // logical and bitwise operators keys
    {"and",      ETokenType::AND             },
    {"!and",     ETokenType::NAND            },
    {"nand",     ETokenType::NAND            },
    {"or",       ETokenType::OR              },
    {"!or",      ETokenType::NOR             },
    {"nor",      ETokenType::NOR             },
    {"xor",      ETokenType::XOR             },
    {"!xor",     ETokenType::XNOR            },
    {"xnor",     ETokenType::XNOR            },
    {"not",      ETokenType::NOT             },
    {"not.b",    ETokenType::B_NOT           },
    {"and.b",    ETokenType::B_AND           },
    {"!and.b",   ETokenType::B_NAND          },
    {"nand.b",   ETokenType::B_NAND          },
    {"or.b",     ETokenType::B_OR            },
    {"xor.b",    ETokenType::B_XOR           },
    {"!or.b",    ETokenType::B_NOR           },
    {"nor.b",    ETokenType::B_NOR           },
    {"!xor.b",   ETokenType::B_XNOR          },
    {"xnor.b",   ETokenType::B_XNOR          },
    {"!",        ETokenType::NOT             },
    // comparison keys
    {"==",       ETokenType::OP_EQ           },
    {"===",      ETokenType::OP_EQS          },
    {"!=",       ETokenType::OP_NEQ          },
    {"!==",      ETokenType::OP_NEQS         },
    {">=",       ETokenType::OP_GEQ          },
    {"<=",       ETokenType::OP_LEQ          },
    {"+",        ETokenType::OP_PLUS         },
    {"-",        ETokenType::OP_MINUS        },
    {"*",        ETokenType::OP_ASTERISK     },
    {"**",       ETokenType::OP_POWER        },
    {"^",        ETokenType::OP_CIRCUMFLEX   },
    {"/",        ETokenType::OP_DIVIDE       },
    {"%mod%",    ETokenType::OP_MODULO       },
    {"%quo%",    ETokenType::OP_QUOTIEN      },
    {"%rem%",    ETokenType::OP_REMAIN       },
    {"+=",       ETokenType::ASSIGN_PLUS     },
    {"-=",       ETokenType::ASSIGN_MINUS    },
    {"*=",       ETokenType::ASSIGN_MULTIPLY },
    {"**=",      ETokenType::ASSIGN_POWER    },
    {"/=",       ETokenType::ASSIGN_DIVIDE   },
    {"%mod%=",   ETokenType::ASSIGN_MODULO   },
    {"%quo%=",   ETokenType::ASSIGN_QUOTIEN  },
    {"%rem%=",   ETokenType::ASSIGN_REMAIN   },
    {"<<[0]",    ETokenType::SHIFT_LEFT_0    },
    {"[0]>>",    ETokenType::SHIFT_RIGHT_0   },
    {"<<[1]",    ETokenType::SHIFT_LEFT_1    },
    {"[1]>>",    ETokenType::SHIFT_RIGHT_1   },
    {"<<[a]",    ETokenType::SHIFT_LEFT_A    },
    {"[a]>>",    ETokenType::SHIFT_RIGHT_A   },
    {"<<[r]",    ETokenType::ROTATE_LEFT     },
    {"[r]>>",    ETokenType::ROTATE_RIGHT    },
    // statement keys
    {"if",       ETokenType::IF              },
    {"else",     ETokenType::ELSE            },
    {"elif",     ETokenType::ELIF            },
    {"for",      ETokenType::FOR             },
    {"in",       ETokenType::IN              },
    {"!in",      ETokenType::NIN             },
    {"nin",      ETokenType::NIN             },
    {"step",     ETokenType::STEP            },
    // while keys
    {"while",    ETokenType::WHILE           },
    {"do",       ETokenType::DO_WHILE        },
    {"loop",     ETokenType::LOOP            },
    // match keys
    {"match",    ETokenType::MATCH           },
    // flow keys
    {"break",    ETokenType::BREAK           },
    {"continue", ETokenType::CONTINUE        },
    {"return",   ETokenType::RETURN          },
    {"goto",     ETokenType::GOTO            },
    {"label",    ETokenType::GOTO_LABEL      },
    // literal string key
    {"\"",       ETokenType::QUOTE           },
    // variadic def/inst
    {"...",      ETokenType::VARIADIC        },
    // range keys
    {"..",       ETokenType::RANGE           },
    {"..=",      ETokenType::RANGE_INCLUSIVE },
    // punctuation
    {".",        ETokenType::DOT             },
    {"::>",      ETokenType::RUN_SYSTEM      },
    {"%",        ETokenType::PERCENTAGE      },
};

const std::initializer_list<ETokenType> kVariableKind = {
    ETokenType::LET,
    ETokenType::VAR,
    ETokenType::CONST,
};

const std::initializer_list<ETokenType> kStartIdentifier = {ETokenType::STATIC_ACCESS, ETokenType::IDENTIFIER,
                                                            ETokenType::SUPER_MOD, ETokenType::SELF_MOD};

const std::initializer_list<ETokenType> kAccessTokens = {ETokenType::STATIC_ACCESS, ETokenType::DOT};

const std::initializer_list<ETokenType> kParameterPassMode = {ETokenType::CAPA_MUT,  ETokenType::CAPA_REF,
                                                              ETokenType::CAPA_COPY, ETokenType::CAPA_CLONE,
                                                              ETokenType::ADDR,      ETokenType::CAPA_MOVE};

const std::initializer_list<ETokenType> kCapabilityKind = {
    ETokenType::CAPA_MUT,
    ETokenType::CAPA_REF,
};

const std::initializer_list<ETokenType> kExpressionPassMode = {
    ETokenType::CAPA_MUT_OF,   ETokenType::CAPA_REF_OF,  ETokenType::CAPA_COPY_OF,
    ETokenType::CAPA_CLONE_OF, ETokenType::CAPA_MOVE_OF,
};

const std::initializer_list<ETokenType> kCastType = {
    ETokenType::AS,
    ETokenType::AS_REINTERPRET,
    ETokenType::AS_SAFE,
};

const std::initializer_list<ETokenType> kOperatorTokens = {
    ETokenType::OP_PLUS,
    ETokenType::OP_MINUS,
    ETokenType::OP_ASTERISK,
    ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,
    ETokenType::OP_MODULO,
    ETokenType::OP_QUOTIEN,
    ETokenType::OP_REMAIN,
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

const std::initializer_list<ETokenType> kBitwiseTokens = {
    ETokenType::B_AND,         ETokenType::B_NAND,       ETokenType::B_OR,          ETokenType::B_XOR,
    ETokenType::B_NOR,         ETokenType::B_XNOR,       ETokenType::B_NOT,         ETokenType::SHIFT_LEFT_0,
    ETokenType::SHIFT_RIGHT_0, ETokenType::SHIFT_LEFT_1, ETokenType::SHIFT_RIGHT_1, ETokenType::SHIFT_LEFT_A,
    ETokenType::SHIFT_RIGHT_A, ETokenType::ROTATE_LEFT,  ETokenType::ROTATE_RIGHT,
};

const std::initializer_list<ETokenType> kComparatorTokens = {
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_EQ,  ETokenType::OP_NEQ,
    ETokenType::OP_EQS,        ETokenType::OP_NEQS,        ETokenType::OP_LEQ, ETokenType::OP_GEQ};
const std::initializer_list<ETokenType> kAssignationTokens = {
    ETokenType::MOVE_ASSIGN,   ETokenType::COPY_ASSIGN,     ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,
    ETokenType::ASSIGN_MINUS,  ETokenType::ASSIGN_MULTIPLY, ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_POWER,
    ETokenType::ASSIGN_MODULO, ETokenType::ASSIGN_QUOTIEN,  ETokenType::ASSIGN_REMAIN,
};
const std::initializer_list<ETokenType> kPointerTokens = {
    ETokenType::PTR,
    ETokenType::UPTR,
    ETokenType::SPTR,
    ETokenType::WPTR,
};

const std::initializer_list<ETokenType> kPrimitiveTypeTokens = {
    ETokenType::T_VOID,    ETokenType::T_BOOL,    ETokenType::T_ASCII,    ETokenType::T_STRING, ETokenType::T_UTF32,
    ETokenType::T_TEXT,    ETokenType::T_USIZE,   ETokenType::L_BIN,      ETokenType::L_HEX,    ETokenType::L_OCT,
    ETokenType::L_I,       ETokenType::L_U,       ETokenType::L_F,        ETokenType::T_I8,     ETokenType::T_I16,
    ETokenType::T_I32,     ETokenType::T_I64,     ETokenType::T_I128,     ETokenType::T_ISIZE,  ETokenType::T_U8,
    ETokenType::T_U16,     ETokenType::T_U32,     ETokenType::T_U64,      ETokenType::T_U128,   ETokenType::T_USIZE,
    ETokenType::T_B8,      ETokenType::T_B16,     ETokenType::T_B32,      ETokenType::T_B64,    ETokenType::T_B128,
    ETokenType::T_BSIZE,   ETokenType::T_F32,     ETokenType::T_F64,      ETokenType::T_F128,   ETokenType::T_FSIZE,
    ETokenType::T_PTRDIFF, ETokenType::T_DECIMAL, ETokenType::T_UDECIMAL,
};
const std::initializer_list<ETokenType> kIntegerTypeTokens = {
    ETokenType::T_I8, ETokenType::T_I16, ETokenType::T_I32, ETokenType::T_I64, ETokenType::T_I128, ETokenType::T_ISIZE,
    ETokenType::T_U8, ETokenType::T_U16, ETokenType::T_U32, ETokenType::T_U64, ETokenType::T_U128, ETokenType::T_USIZE,
    ETokenType::T_B8, ETokenType::T_B16, ETokenType::T_B32, ETokenType::T_B64, ETokenType::T_B128, ETokenType::T_BSIZE,
};
const std::initializer_list<ETokenType> kSignedIntegerTypTokens = {
    ETokenType::T_I8, ETokenType::T_I16, ETokenType::T_I32, ETokenType::T_I64, ETokenType::T_I128, ETokenType::T_ISIZE,
};
const std::initializer_list<ETokenType> kUnsignedIntegerTypTokens = {
    ETokenType::T_U8, ETokenType::T_U16, ETokenType::T_U32, ETokenType::T_U64, ETokenType::T_U128, ETokenType::T_USIZE,
};
const std::initializer_list<ETokenType> kBinTypTokens = {
    ETokenType::T_B8, ETokenType::T_B16, ETokenType::T_B32, ETokenType::T_B64, ETokenType::T_B128, ETokenType::T_BSIZE,
};
const std::initializer_list<ETokenType> kFloatingTypeTokens = {
    ETokenType::T_F32,
    ETokenType::T_F64,
    ETokenType::T_F128,
    ETokenType::T_FSIZE,
};
const std::initializer_list<ETokenType> kNumericTypeTokens = {
    ETokenType::L_BIN,  ETokenType::L_HEX,   ETokenType::L_OCT,   ETokenType::L_I,       ETokenType::L_U,
    ETokenType::L_F,    ETokenType::T_I8,    ETokenType::T_I16,   ETokenType::T_I32,     ETokenType::T_I64,
    ETokenType::T_I128, ETokenType::T_ISIZE, ETokenType::T_U8,    ETokenType::T_U16,     ETokenType::T_U32,
    ETokenType::T_U64,  ETokenType::T_U128,  ETokenType::T_USIZE, ETokenType::T_B8,      ETokenType::T_B16,
    ETokenType::T_B32,  ETokenType::T_B64,   ETokenType::T_B128,  ETokenType::T_BSIZE,   ETokenType::T_F32,
    ETokenType::T_F64,  ETokenType::T_F128,  ETokenType::T_FSIZE, ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> kDecimalTypeTokens = {ETokenType::L_DECIMAL, ETokenType::L_UDECIMAL,
                                                              ETokenType::T_DECIMAL, ETokenType::T_UDECIMAL};
const std::initializer_list<ETokenType> kBooleanTypeTokens = {ETokenType::TRUE, ETokenType::FALSE, ETokenType::T_BOOL};

const std::initializer_list<ETokenType> kModificatorOpTokens = {
    ETokenType::ASSIGN_PLUS,  ETokenType::ASSIGN_MINUS,  ETokenType::ASSIGN_MULTIPLY, ETokenType::ASSIGN_DIVIDE,
    ETokenType::ASSIGN_POWER, ETokenType::ASSIGN_MODULO, ETokenType::ASSIGN_QUOTIEN,  ETokenType::ASSIGN_REMAIN,
};

const std::initializer_list<ETokenType> kComparisonOpTokens = {
    ETokenType::OP_EQ,          ETokenType::OP_NEQ, ETokenType::OP_EQS, ETokenType::OP_NEQS, ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ, ETokenType::OP_LEQ, ETokenType::IN,      ETokenType::NIN,
};
const std::initializer_list<ETokenType> kArithmeticOpTokens = {
    ETokenType::OP_PLUS,  ETokenType::OP_MINUS,  ETokenType::OP_ASTERISK, ETokenType::OP_DIVIDE,
    ETokenType::OP_POWER, ETokenType::OP_MODULO, ETokenType::OP_QUOTIEN,  ETokenType::OP_REMAIN,
};

const std::initializer_list<ETokenType> kFormatypeTokens = {
    ETokenType::S_TEXTUAL_EXPR_START,
    ETokenType::S_TEXTUAL_EXPR_END,
};
const std::initializer_list<ETokenType> kTypeTokens = {
    ETokenType::T_I8,       ETokenType::T_I16,      ETokenType::T_I32,     ETokenType::T_I64,   ETokenType::T_I128,
    ETokenType::T_ISIZE,    ETokenType::T_U8,       ETokenType::T_U16,     ETokenType::T_U32,   ETokenType::T_U64,
    ETokenType::T_U128,     ETokenType::T_USIZE,    ETokenType::T_B8,      ETokenType::T_B16,   ETokenType::T_B32,
    ETokenType::T_B64,      ETokenType::T_B128,     ETokenType::T_BSIZE,   ETokenType::T_F32,   ETokenType::T_F64,
    ETokenType::T_F128,     ETokenType::T_FSIZE,    ETokenType::T_BOOL,    ETokenType::T_UTF32, ETokenType::T_ASCII,
    ETokenType::T_DECIMAL,  ETokenType::T_UDECIMAL, ETokenType::T_STRING,  ETokenType::T_TEXT,  ETokenType::FUNCTION,
    ETokenType::IDENTIFIER, ETokenType::TYPE,       ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> kHybridKeyNamespace = {
    ETokenType::T_I8,       ETokenType::T_I16,      ETokenType::T_I32,     ETokenType::T_I64,   ETokenType::T_I128,
    ETokenType::T_ISIZE,    ETokenType::T_U8,       ETokenType::T_U16,     ETokenType::T_U32,   ETokenType::T_U64,
    ETokenType::T_U128,     ETokenType::T_USIZE,    ETokenType::T_B8,      ETokenType::T_B16,   ETokenType::T_B32,
    ETokenType::T_B64,      ETokenType::T_B128,     ETokenType::T_BSIZE,   ETokenType::T_F32,   ETokenType::T_F64,
    ETokenType::T_F128,     ETokenType::T_FSIZE,    ETokenType::T_BOOL,    ETokenType::T_UTF32, ETokenType::T_ASCII,
    ETokenType::T_DECIMAL,  ETokenType::T_UDECIMAL, ETokenType::T_STRING,  ETokenType::T_TEXT,  ETokenType::FUNCTION,
    ETokenType::IDENTIFIER, ETokenType::TYPE,       ETokenType::T_VOID,    ETokenType::CAST,    ETokenType::FUNCTION,
    ETokenType::LAMBDA,     ETokenType::FLAG,       ETokenType::PTR,       ETokenType::UPTR,    ETokenType::SPTR,
    ETokenType::WPTR,       ETokenType::ENTITY,     ETokenType::COMPONENT, ETokenType::GENERIC, ETokenType::T_PTRDIFF,
};
const std::initializer_list<ETokenType> kLogicalTokens = {ETokenType::AND, ETokenType::NAND, ETokenType::OR,
                                                          ETokenType::XOR, ETokenType::NOR,  ETokenType::XNOR,
                                                          ETokenType::NOT};

const std::initializer_list<ETokenType> kEndArgsListokens = {
    ETokenType::CLOSE_BRACKETS, ETokenType::CLOSE_BRACE, ETokenType::OPEN_BRACE, ETokenType::CLOSE_PAREN,
    ETokenType::CLOSE_SQUARE,   ETokenType::SEMICOLON,   ETokenType::ASSIGN,     ETokenType::PIPE,
    ETokenType::PIPE_MUT,       ETokenType::SEMICOLON,   ETokenType::METACODE,
};
const std::initializer_list<ETokenType> kArgsDelimitationTokens = {
    ETokenType::CLOSE_BRACKETS, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_PAREN, ETokenType::OPEN_PAREN,
    ETokenType::CLOSE_SQUARE,   ETokenType::OPEN_SQUARE,   ETokenType::SEMICOLON,   ETokenType::PIPE};
const std::initializer_list<ETokenType> kGenArgsValidTokens = {
    ETokenType::T_I8,          ETokenType::T_I16,          ETokenType::T_I32,      ETokenType::T_I64,
    ETokenType::T_I128,        ETokenType::T_ISIZE,        ETokenType::T_U8,       ETokenType::T_U16,
    ETokenType::T_U32,         ETokenType::T_U64,          ETokenType::T_U128,     ETokenType::T_USIZE,
    ETokenType::T_B8,          ETokenType::T_B16,          ETokenType::T_B32,      ETokenType::T_B64,
    ETokenType::T_B128,        ETokenType::T_BSIZE,        ETokenType::T_F32,      ETokenType::T_F64,
    ETokenType::T_F128,        ETokenType::T_FSIZE,        ETokenType::DOLLAR,     ETokenType::INTERROGATIVE,
    ETokenType::IDENTIFIER,    ETokenType::STATIC_ACCESS,  ETokenType::PTR,        ETokenType::UPTR,
    ETokenType::SPTR,          ETokenType::WPTR,           ETokenType::TICK,       ETokenType::HASHTAG,
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::IDENTIFIER, ETokenType::T_BOOL,
    ETokenType::T_UTF32,       ETokenType::T_ASCII,        ETokenType::T_DECIMAL,  ETokenType::T_UDECIMAL,
    ETokenType::T_STRING,      ETokenType::T_TEXT,         ETokenType::FUNCTION,   ETokenType::IDENTIFIER,
    ETokenType::TYPE,          ETokenType::COMMA,          ETokenType::LET,        ETokenType::T_PTRDIFF,
};

const std::initializer_list<ETokenType> kLiteralTokens = {
    ETokenType::IDENTIFIER, ETokenType::L_BIN, ETokenType::L_OCT,     ETokenType::L_HEX,      ETokenType::L_I,
    ETokenType::L_U,        ETokenType::L_F,   ETokenType::L_DECIMAL, ETokenType::L_UDECIMAL, ETokenType::L_TEXTUAL,
    ETokenType::TRUE,       ETokenType::FALSE, ETokenType::L_ASCII,   ETokenType::L_ARRAY,
};
const std::initializer_list<ETokenType> kInvalidCodeTokens = {
    ETokenType::METACODE,
    ETokenType::S_METACODE_END,
    ETokenType::S_END_OF_FILE,
};

const std::initializer_list<ETokenType> boolOpHandled = {
    ETokenType::AND,    ETokenType::OR,    ETokenType::NOR,    ETokenType::XOR,   ETokenType::XNOR,  ETokenType::NAND,
    ETokenType::OP_NEQ, ETokenType::B_AND, ETokenType::B_OR,   ETokenType::B_NOR, ETokenType::B_XOR, ETokenType::B_XNOR,
    ETokenType::B_NAND, ETokenType::OP_EQ, ETokenType::ASSIGN, ETokenType::NOT,   ETokenType::B_NOT,
};
const std::initializer_list<ETokenType> integerOpHandled = {
    ETokenType::OP_PLUS,       ETokenType::OP_MINUS,      ETokenType::OP_ASTERISK,    ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,     ETokenType::OP_MODULO,     ETokenType::OP_QUOTIEN,     ETokenType::OP_REMAIN,
    ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,   ETokenType::ASSIGN_MINUS,   ETokenType::ASSIGN_MULTIPLY,
    ETokenType::ASSIGN_POWER,  ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_MODULO,  ETokenType::ASSIGN_QUOTIEN,
    ETokenType::ASSIGN_REMAIN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ,        ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> floatingOpHandled = {
    ETokenType::OP_PLUS,       ETokenType::OP_MINUS,      ETokenType::OP_ASTERISK,    ETokenType::OP_POWER,
    ETokenType::OP_DIVIDE,     ETokenType::OP_MODULO,     ETokenType::OP_QUOTIEN,     ETokenType::OP_REMAIN,
    ETokenType::ASSIGN,        ETokenType::ASSIGN_PLUS,   ETokenType::ASSIGN_MINUS,   ETokenType::ASSIGN_MULTIPLY,
    ETokenType::ASSIGN_POWER,  ETokenType::ASSIGN_DIVIDE, ETokenType::ASSIGN_MODULO,  ETokenType::ASSIGN_QUOTIEN,
    ETokenType::ASSIGN_REMAIN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ,        ETokenType::OP_EQ,         ETokenType::OP_NEQ,         ETokenType::OP_EQS,
    ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> charOpHandled = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,  ETokenType::OP_LEQ,
    ETokenType::OP_EQ,  ETokenType::OP_NEQ,        ETokenType::OP_EQS,         ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> strOpHandled = {
    ETokenType::OP_PLUS,        ETokenType::ASSIGN, ETokenType::ASSIGN_PLUS, ETokenType::OPEN_BRACKETS,
    ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ, ETokenType::OP_LEQ,      ETokenType::OP_EQ,
    ETokenType::OP_NEQ,         ETokenType::OP_EQS, ETokenType::OP_NEQS,
};
const std::initializer_list<ETokenType> enumOpHandled = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> addressOpHandled = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> arrayOpHandled = {
    ETokenType::ASSIGN, ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_GEQ,
    ETokenType::OP_LEQ, ETokenType::OP_EQ,         ETokenType::OP_NEQ,
};
const std::initializer_list<ETokenType> kFormatSpecAlign = {
    ETokenType::OPEN_BRACKETS, ETokenType::CLOSE_BRACKETS, ETokenType::OP_CIRCUMFLEX,
    ETokenType::TILDE,         ETokenType::ASSIGN,
};

const std::initializer_list<ETokenType> kIndentifiable = {
    // mod key
    ETokenType::MOD,
    // type keys
    ETokenType::TYPE, ETokenType::T_VOID, ETokenType::T_BOOL, ETokenType::ENUM, ETokenType::T_UTF32,
    ETokenType::T_ASCII, ETokenType::T_ISIZE, ETokenType::T_USIZE, ETokenType::T_BSIZE, ETokenType::T_I8,
    ETokenType::T_U8, ETokenType::T_B8, ETokenType::T_I16, ETokenType::T_U16, ETokenType::T_B16, ETokenType::T_I32,
    ETokenType::T_U32, ETokenType::T_B32, ETokenType::T_I64, ETokenType::T_U64, ETokenType::T_B64, ETokenType::T_I128,
    ETokenType::T_U128, ETokenType::T_B128, ETokenType::T_F32, ETokenType::T_F64, ETokenType::T_F128,
    ETokenType::T_STRING, ETokenType::T_TEXT, ETokenType::T_DECIMAL, ETokenType::T_UDECIMAL, ETokenType::T_TEXT,
    ETokenType::T_PTRDIFF,
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
  if (auto it = kKeywords.find(str); it != kKeywords.end()) return it->second;
  return ETokenType::UNKNOWN;
}
