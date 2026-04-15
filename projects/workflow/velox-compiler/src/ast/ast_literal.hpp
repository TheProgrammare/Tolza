#pragma once


#include <memory>
#include <string>

#include "ast/ast_data.hpp"
#include "ast_base.hpp"
#include "ast_numeric_128_bits.hpp"

namespace ast
{
namespace literal
{

struct Boolean final : public ALiteral {
  bool val = false;

  Boolean();
  Boolean(bool value);

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "bool(" + std::to_string(val) + ")";
  }
};

struct Integral final : public ALiteral {
  Int128    val;
  EPrimType type = EPrimType::NONE;

  Integral();
  Integral(const Int128& value);


  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return EPrimType_to_str(type) + "(" + val.i128_to_string() + ")";
  }

  bool is_signed() const;
};

struct Fixed_Point final : public ALiteral {
  Int128    val;
  EPrimType raw_type = EPrimType::NONE;
  size_t    scale    = 1;

  Fixed_Point();
  Fixed_Point(const Int128& value, size_t _scale, EPrimType _raw_type);

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return EPrimType_to_str(raw_type) + "(" + val.i128_to_string() + ")";
  }

  VISTOR_ACCEPT
};

struct Floating_Point final : public ALiteral {
  Float128  val;
  EPrimType type = EPrimType::fSize;

  Floating_Point();
  Floating_Point(const Float128& value);

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return EPrimType_to_str(type) + "(" + val.float128_to_string() + ")";
  }

  VISTOR_ACCEPT
};

// Latin-1 encoding
struct CUNE final : public ALiteral {
  char val = 0x0;

  CUNE();
  CUNE(char value);

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "cunei(\"" + std::to_string(val) + "\")";
  }

  VISTOR_ACCEPT
};

struct RUNE final : public ALiteral {
  std::string code_points;

  RUNE();
  RUNE(std::string codePoints_value);

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "rune(\"" + code_points + "\")";
  }

  VISTOR_ACCEPT
};


struct Text_Pure final : public ALiteral {
  std::string val;
  EPrimType   text_type = EPrimType::text;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "\"" + std::string(val.begin(), val.end()) + "\"";
  }

  VISTOR_ACCEPT
};

// format_spec ::= [options][width][grouping]["." precision][type]
// options ::= [[fill]align] [sign]["z"]["#"]["0"]
// fill ::= <any character>
// align ::= "<" | ">" | "=" | "^"
// sign ::= "+" | "-" | " "
// width ::= digit + grouping ::= "," | "_"
// precision ::= digit + type ::= "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" | "s" | "x" | "X" |
// "%"
struct Format_Specifier final : public Node {
  std::string src_Str;
  char        fill = '\0'; // char fill, e.g. '0', '*', ' '
  enum class EAlign { Right, Left, Center, Justify };
  EAlign align = EAlign::Right; // '>', '<', '^', '~', '='

  // Sign
  enum class ESign { None, Pos, Neg, Space };
  ESign sign           = ESign::None; // '+', '-', ' ' (space)
  bool  signBeforeFill = false;

  // Numeric
  enum class EPrefix { None, Hex, HEX, Bin, Oct };
  EPrefix                      prefix   = EPrefix::None; // '#' for  0x, 0b, 0o
  bool                         zero_pad = false;         // '0' fill to left
  std::unique_ptr<AExpression> width;                    // min width
  char                         grouping_char;            // ',' or '_' or ''' for thousands

  // precision
  // and
  // type
  std::unique_ptr<AExpression> precision; // decimal number
  enum class EDisplayFormat {
    String,
    Binary,
    Character,
    Decimal,
    Octal,
    Hex,
    HEX,
    Number,
    e,
    E,
    Fixed,
    FIXED,
    g,
    G,
    Percentage
  };
  EDisplayFormat display_format = EDisplayFormat::String; // 's', 'b', 'c', 'd', 'o', 'x', 'X', 'n',
                                                          // 'e', 'E', 'f', 'F', 'g', 'G', '%'

  std::string debug_str() const override
  {
    return "format specifier \":" + src_Str + "\"";
  }

  VISTOR_ACCEPT
};

// "{expression}"
// "{expression:spec}"
struct Text_Interpolation final : public AExpression {
  std::unique_ptr<AExpression>      expression;
  std::unique_ptr<Format_Specifier> spec;

  SET_R_VAL

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "text interpolation";
  }

  VISTOR_ACCEPT
};

// "format node {formatVariable} can be formated"
struct Textual_Format final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> values;

  [[nodiscard]] ast::literal::Text_Pure* get_if_pure_text() const;

  CODEGEN_VALUE
  std::string debug_str() const override;
  VISTOR_ACCEPT
};


struct Table_Population final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> ranges;
  std::unique_ptr<AExpression>              expression;
  std::unique_ptr<AExpression>              map_expression_value;

  // can be a ex nihilo node (for primitive types)
  INFERRED_TYPE element_definition;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "table population";
  }

  VISTOR_ACCEPT
};

struct Table final : public ALiteral {
  // for explicit specified values like: { 0, 1, 2, 3 }
  [[maybe_unused]]
  std::vector<std::unique_ptr<AExpression>> values;
  // for procedural generated values like: { 0..4 = rand::gauss() }
  [[maybe_unused]]
  std::unique_ptr<Table_Population> population;

  // can be a ex nihilo node (for primitive types)
  INFERRED_TYPE element_type;

  std::vector<size_t> resolved_size;

  CODEGEN_VALUE

  bool is_matrix() const
  {
    return resolved_size.size() > 1;
  }
  bool is_table_population() const
  {
    return population.get();
  }


  std::string debug_str() const override;

  VISTOR_ACCEPT
};

struct Map final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> keys;
  std::vector<std::unique_ptr<AExpression>> values;

  [[maybe_unused]]
  std::unique_ptr<Table_Population> population;

  size_t size = 1;

  INFERRED_TYPE key_type;
  INFERRED_TYPE value_type;

  // key + value (no alignment need because it's translated to 2 arrays)
  size_t ty_sizeByte = 0;

  CODEGEN_VALUE

  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "literal map[" + std::to_string(keys.size()) + "]";
  }
};

struct Enum final : public ALiteral {
  // path = enum name
  // base_name = enum element
  std::unique_ptr<ast::AIdentifier>         name;
  std::vector<std::unique_ptr<AExpression>> member_values;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "literal enum[" + std::to_string(expression_in_type_position) + " - " + name->debug_str() + "]";
  }

  VISTOR_ACCEPT
};

struct Tuple final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> values;
  std::vector<std::string>                  name_fields;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    if (name_fields.empty()) return "literal tuple(" + std::to_string(values.size()) + ")";
    return "literal named tuple(" + std::to_string(values.size()) + ")";
  }

  VISTOR_ACCEPT
};

// first..end or first..=end
struct Range final : public ALiteral {
  std::unique_ptr<AExpression> start;
  std::unique_ptr<AExpression> end;
  std::unique_ptr<AExpression> step;
  bool                         endInclude = false;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "literal range";
  }

  VISTOR_ACCEPT
};

// CIdentity{ name: "Zagreus", age: 25 }
// can be component, union, enum, flag
struct Structured_Data final : public ALiteral {
  std::unique_ptr<ast::AIdentifier>                       name;
  std::vector<std::unique_ptr<expression::Call_Argument>> field_args;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "literal structured data[" + name->debug_str() + "]";
  }

  VISTOR_ACCEPT
};

// Person{ CIdentity.name: "Zagreus", CIdentity.age: 25 }
// not the same as Person("Zagreus", 32) it's a call of constructor
struct Entity final : public ALiteral {
  std::unique_ptr<ast::AIdentifier>             name;
  std::vector<std::unique_ptr<Structured_Data>> comp_args;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "literal entity[" + name->debug_str() + "]";
  }

  VISTOR_ACCEPT
};

struct Iterator final : public ALiteral {
  std::unique_ptr<AExpression> collection;

  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "literal iterator";
  }

  VISTOR_ACCEPT
};

} // namespace literal
  // Literal
} // namespace ast
  // AST