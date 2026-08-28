#include "token.hpp"

#include "compiler/compilation_unit.hpp"

#include <string_view>

token::ID token::Arena::add(Token& tok) noexcept
{
  auto id   = token::ID::make(cuid, tokens.size());
  tok.tokid = id;
  tokens.emplace_back(std::move(tok));
  return id;
}

std::string_view token::Arena::Audit::Token_to_str(ID tokid) const noexcept
{
  auto&       tok = arena.get(tokid);
  const auto& cu  = arena.cuid.get();
  assert(cu.file_info.data.data());
  if (tok.begin == cu.file_info.data.size()) {
    return {cu.file_info.data.data() + tok.begin - 1, 1};
  }
  assert(tok.begin + tok.length <= cu.file_info.data.size());
  return {cu.file_info.data.data() + tok.begin, tok.length};
}
size_t token::Arena::Audit::Token_to_line(ID tokid) const noexcept
{
  auto& tok = arena.get(tokid);
  return arena.cuid.get().file_info.get_line_from_pos(tok.begin);
}
std::string_view token::Arena::Audit::Token_to_line_str(ID tokid) const noexcept
{
  auto&       tok  = arena.get(tokid);
  const auto& cu   = arena.cuid.get();
  size_t      line = cu.file_info.get_line_from_pos(tok.begin);
  return cu.file_info.get_line(line);
}

const std::map<std::string_view, token::ETokenKind> token::k_keywords = {
    // mod key
    {"mod",      ETokenKind::MOD       },
    // type keys
    {"type",     ETokenKind::TYPE      },
    {"u0",       ETokenKind::T_U0      },
    {"bool",     ETokenKind::T_BOOL    },
    {"false",    ETokenKind::FALSE     },
    {"true",     ETokenKind::TRUE      },
    {"cune",     ETokenKind::T_CUNE    },
    {"rune",     ETokenKind::T_RUNE    },
    {"fsize",    ETokenKind::T_FSIZE   },
    {"ssize",    ETokenKind::T_SSIZE   },
    {"usize",    ETokenKind::T_USIZE   },
    {"s8",       ETokenKind::T_S8      },
    {"u8",       ETokenKind::T_U8      },
    {"b8",       ETokenKind::T_B8      },
    {"s16",      ETokenKind::T_S16     },
    {"u16",      ETokenKind::T_U16     },
    {"b16",      ETokenKind::T_B16     },
    {"s32",      ETokenKind::T_S32     },
    {"u32",      ETokenKind::T_U32     },
    {"b32",      ETokenKind::T_B32     },
    {"s64",      ETokenKind::T_S64     },
    {"u64",      ETokenKind::T_U64     },
    {"b64",      ETokenKind::T_B64     },
    {"s128",     ETokenKind::T_S64     },
    {"u128",     ETokenKind::T_U64     },
    {"b128",     ETokenKind::T_B128    },
    {"f16",      ETokenKind::T_F16     },
    {"f32",      ETokenKind::T_F32     },
    {"f64",      ETokenKind::T_F64     },
    {"f80",      ETokenKind::T_F80     },
    {"f128",     ETokenKind::T_F128    },
    {"cstr",     ETokenKind::T_CSTR    },
    {"str",      ETokenKind::T_STR     },
    {"text",     ETokenKind::T_TEXT    },
    {"dsize",    ETokenKind::T_DSIZE   },
    {"d32",      ETokenKind::T_D32     },
    {"d64",      ETokenKind::T_D64     },
    {"d128",     ETokenKind::T_D128    },
    {"udsize",   ETokenKind::T_UDSIZE  },
    {"ud32",     ETokenKind::T_UD32    },
    {"ud64",     ETokenKind::T_UD64    },
    {"ud128",    ETokenKind::T_UD128   },
    {"ptrsize",  ETokenKind::T_PTRDIFF },
    {"uninit",   ETokenKind::L_UNINIT  },
    {"nullptr",  ETokenKind::L_NULLPTR },
    {"opaque",   ETokenKind::T_OPAQUE  },
    // variable declaration
    {"let",      ETokenKind::LET       },
    {"var",      ETokenKind::VAR       },
    {"const",    ETokenKind::CONST     },
    // cast
    {"as",       ETokenKind::AS        },
    // typeid checking
    {"is",       ETokenKind::IS        },
    {"nis",      ETokenKind::NIS       },
    // class keys
    {"form",     ETokenKind::FORM      },
    {"view",     ETokenKind::VIEW      },
    {"facet",    ETokenKind::FACET     },
    {"use",      ETokenKind::USE       },
    {"enum",     ETokenKind::ENUM      },
    {"flag",     ETokenKind::FLAG      },
    {"union",    ETokenKind::UNION     },
    {"rule",     ETokenKind::RULE      },
    // generic keys
    {"gen",      ETokenKind::GENERIC   },
    {"import",   ETokenKind::IMPORT    },
    {"export",   ETokenKind::EXPORT    },
    {"reexport", ETokenKind::REEXPORT  },
    {"extern",   ETokenKind::EXTERN    },
    // function keys
    {"fn",       ETokenKind::FUNCTION  },
    {"extend",   ETokenKind::EXTENSION },
    {"lam",      ETokenKind::LAMBDA    },

    // pointers
    {"ptr",      ETokenKind::PTR       },
    {"new",      ETokenKind::NEW       },
    {"del",      ETokenKind::DEL       },
    {"drop",     ETokenKind::DROP      },
    {"addr",     ETokenKind::ADDR      },
    {"ref",      ETokenKind::CAPA_REF  },
    {"move",     ETokenKind::CAPA_MOVE },
    {"mut",      ETokenKind::CAPA_MUT  },
    {"copy",     ETokenKind::CAPA_COPY },
    {"rule",     ETokenKind::RULE      },
    {"with",     ETokenKind::WITH      },
    {"on",       ETokenKind::ON        },
    // operator overloading
    {"op",       ETokenKind::OP        },
    // logical and bitwise operators keys
    {"and",      ETokenKind::OP_AND    },
    {"nand",     ETokenKind::OP_NAND   },
    {"or",       ETokenKind::OP_OR     },
    {"nor",      ETokenKind::OP_NOR    },
    {"xor",      ETokenKind::OP_XOR    },
    {"xnor",     ETokenKind::OP_XNOR   },
    {"not",      ETokenKind::OP_NOT    },

    // statement keys
    {"if",       ETokenKind::IF        },
    {"else",     ETokenKind::ELSE      },
    {"elif",     ETokenKind::ELIF      },
    {"for",      ETokenKind::FOR       },
    {"in",       ETokenKind::IN        },
    {"nin",      ETokenKind::NIN       },
    {"step",     ETokenKind::STEP      },
    // while keys
    {"while",    ETokenKind::WHILE     },
    {"do",       ETokenKind::DO_WHILE  },
    {"loop",     ETokenKind::LOOP      },
    // match keys
    {"match",    ETokenKind::MATCH     },
    // flow keys
    {"break",    ETokenKind::BREAK     },
    {"continue", ETokenKind::CONTINUE  },
    {"return",   ETokenKind::RETURN    },
    {"goto",     ETokenKind::GOTO      },
    {"label",    ETokenKind::GOTO_LABEL},
};

const std::unordered_map<std::string_view, token::ETokenKind> token::k_DFA = {
    {"b.not",    ETokenKind::OP_B_NOT            },
    {"b.and",    ETokenKind::OP_B_AND            },
    {"b.nand",   ETokenKind::OP_B_NAND           },
    {"b.or",     ETokenKind::OP_B_OR             },
    {"b.xor",    ETokenKind::OP_B_XOR            },
    {"b.nor",    ETokenKind::OP_B_NOR            },
    {"b.xnor",   ETokenKind::OP_B_XNOR           },
    {"b.shl.0",  ETokenKind::OP_SHIFT_LEFT_0     },
    {"b.shl.1",  ETokenKind::OP_SHIFT_LEFT_1     },
    {"b.shl.a",  ETokenKind::OP_SHIFT_LEFT_A     },
    {"b.shr.0",  ETokenKind::OP_SHIFT_RIGHT_0    },
    {"b.shr.1",  ETokenKind::OP_SHIFT_RIGHT_1    },
    {"b.shr.a",  ETokenKind::OP_SHIFT_RIGHT_A    },
    {"b.rol",    ETokenKind::OP_ROTATE_LEFT      },
    {"b.ror",    ETokenKind::OP_ROTATE_RIGHT     },
    {"+",        ETokenKind::OP_PLUS             },
    {"-",        ETokenKind::OP_MINUS            },
    {"*",        ETokenKind::OP_MULTIPLY         },
    {"**",       ETokenKind::OP_POWER            },
    {"^",        ETokenKind::OP_CIRCUMFLEX       },
    {"/",        ETokenKind::OP_DIVIDE           },
    {"%m",       ETokenKind::OP_MODULO           },
    {"%q",       ETokenKind::OP_QUOTIEN          },
    {"%r",       ETokenKind::OP_REMAIN           },
    {"d.dre",    ETokenKind::OP_DIVREM           },
    {"m.add",    ETokenKind::OP_MEM_ADD          },
    {"m.sub",    ETokenKind::OP_MEM_SUB          },
    {"m.dist",   ETokenKind::OP_MEM_DIST         },
    {"+=",       ETokenKind::ASSIGN_PLUS         },
    {"-=",       ETokenKind::ASSIGN_MINUS        },
    {"*=",       ETokenKind::ASSIGN_MULTIPLY     },
    {"**=",      ETokenKind::ASSIGN_POWER        },
    {"/=",       ETokenKind::ASSIGN_DIVIDE       },
    {"%m=",      ETokenKind::ASSIGN_MODULO       },
    {"%q=",      ETokenKind::ASSIGN_QUOTIEN      },
    {"%r=",      ETokenKind::ASSIGN_REMAIN       },
    {"d.dre=",   ETokenKind::ASSIGN_DIVREM       },
    {"m.add=",   ETokenKind::ASSIGN_MEM_ADD      },
    {"m.sub=",   ETokenKind::ASSIGN_MEM_SUB      },
    {"m.dist=",  ETokenKind::ASSIGN_MEM_DIST     },
    {"b.not=",   ETokenKind::ASSIGN_B_NOT        },
    {"b.and=",   ETokenKind::ASSIGN_B_AND        },
    {"b.nand=",  ETokenKind::ASSIGN_B_NAND       },
    {"b.or=",    ETokenKind::ASSIGN_B_OR         },
    {"b.xor=",   ETokenKind::ASSIGN_B_XOR        },
    {"b.nor=",   ETokenKind::ASSIGN_B_NOR        },
    {"b.xnor=",  ETokenKind::ASSIGN_B_XNOR       },
    {"b.shl.0=", ETokenKind::ASSIGN_SHIFT_LEFT_0 },
    {"b.shl.1=", ETokenKind::ASSIGN_SHIFT_LEFT_1 },
    {"b.shl.a=", ETokenKind::ASSIGN_SHIFT_LEFT_A },
    {"b.shr.0=", ETokenKind::ASSIGN_SHIFT_RIGHT_0},
    {"b.shr.1=", ETokenKind::ASSIGN_SHIFT_RIGHT_1},
    {"b.shr.a=", ETokenKind::ASSIGN_SHIFT_RIGHT_A},
    {"b.rol=",   ETokenKind::ASSIGN_ROTATE_LEFT  },
    {"b.ror=",   ETokenKind::ASSIGN_ROTATE_RIGHT },
    {"@",        ETokenKind::AT                  },
    {"&",        ETokenKind::AMPERSAND           },
    {"!",        ETokenKind::EXCLAMATION         },
    {"?",        ETokenKind::INTERROGATIVE       },
    {"$",        ETokenKind::DOLLAR              },
    {"_",        ETokenKind::UNDERSCORE          },
    {"=",        ETokenKind::ASSIGN              },
    {"=>",       ETokenKind::INJECT              },
    {"->",       ETokenKind::ARROW               },
    {"==",       ETokenKind::OP_EQ               },
    {"!=",       ETokenKind::OP_NEQ              },
    {">=",       ETokenKind::OP_GEQ              },
    {"<=",       ETokenKind::OP_LEQ              },
    {"===",      ETokenKind::OP_EQS              },
    {"!==",      ETokenKind::OP_NEQS             },
    {"<",        ETokenKind::L_ANGLE             },
    {">",        ETokenKind::R_ANGLE             },
    {"{",        ETokenKind::L_CURLY             },
    {"}",        ETokenKind::R_CURLY             },
    {"(",        ETokenKind::L_PAREN             },
    {")",        ETokenKind::R_PAREN             },
    {"[",        ETokenKind::L_SQUARE            },
    {"]",        ETokenKind::R_SQUARE            },
    {"'",        ETokenKind::TICK                },
    {",",        ETokenKind::COMMA               },
    {"~",        ETokenKind::TILDE               },
    {".",        ETokenKind::DOT                 },
    {"..",       ETokenKind::RANGE               },
    {"..=",      ETokenKind::RANGE_INCLUSIVE     },
    {"...",      ETokenKind::VARIADIC            },
    {":",        ETokenKind::COLON               },
    {";",        ETokenKind::SEMICOLON           },
    {"::",       ETokenKind::STATIC_ACCESS       },
    {"::<",      ETokenKind::TURBO_FISH          },
    {"#",        ETokenKind::METACODE            },
};