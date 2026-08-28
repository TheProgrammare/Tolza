#pragma once

#include "ast_numeric_128_bits.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/type.hpp"

#include <vector>

namespace ast
{

struct Literal_Boolean final {
  NODE_HEADER(Literal_Boolean);

  bool val = false;
};

struct Literal_Integral final {
  NODE_HEADER(Literal_Integral);

  Int128                     val;
  ::type::EPrimitiveTypeKind type = ::type::EPrimitiveTypeKind::NONE;
};

struct Literal_Fixed_Point final {
  NODE_HEADER(Literal_Fixed_Point);

  Int128                     val;
  ::type::EPrimitiveTypeKind raw_type = ::type::EPrimitiveTypeKind::NONE;
  size_t                     scale    = 1;
};

struct Literal_Floating_Point final {
  NODE_HEADER(Literal_Floating_Point);

  Float128                   val;
  ::type::EPrimitiveTypeKind type = ::type::EPrimitiveTypeKind::NONE;
};

// Latin-1 encoding
struct Literal_Cune final {
  NODE_HEADER(Literal_Cune);

  char val = 0x0;
};

struct Literal_Rune final {
  NODE_HEADER(Literal_Rune);

  std::string code_points;
};


struct Literal_Text_Pure final {
  NODE_HEADER(Literal_Text_Pure);

  std::string       val;
  ::type::ETextType text_type = ::type::ETextType::NONE;
};

// format_spec ::= [options][width][grouping]["." precision][type]
// options ::= [[fill]align] [sign]["z"]["#"]["0"]
// fill ::= <any character>
// align ::= "<" | ">" | "=" | "^"
// sign ::= "+" | "-" | " "
// width ::= digit + grouping ::= "," | "_"
// precision ::= digit + type ::= "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" | "s" | "x" | "X" |
// "%"
struct Literal_Format_Specifier final {
  NODE_HEADER(Literal_Format_Specifier);

  std::string src_Str;
  char        fill = '\0'; // char fill, e.g. '0', '*', ' '
  enum class EAlign : uint8_t { Right, Left, Center, Justify };
  EAlign align = EAlign::Right; // '>', '<', '^', '~', '='

  // Sign
  enum class ESign : uint8_t { None, Pos, Neg, Space };
  ESign sign           = ESign::None; // '+', '-', ' ' (space)
  bool  signBeforeFill = false;

  // Numeric
  enum class EPrefix : uint8_t { None, Hex, HEX, Bin, Oct };
  EPrefix prefix   = EPrefix::None; // '#' for  0x, 0b, 0o
  bool    zero_pad = false;         // '0' fill to left
  SET_NODE(width);
  char grouping_char; // ',' or '_' or ''' for thousands

  // precision
  // and
  // type
  SET_NODE(precision); // decimal number
  enum class EDisplayFormat : uint8_t {
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
  // 's', 'b', 'c', 'd', 'o', 'x', 'X', 'n',
  // 'e', 'E', 'f', 'F', 'g', 'G', '%'
  EDisplayFormat display_format = EDisplayFormat::String;
};

// "{expression}" "{expression:spec}"
struct Literal_Text_Interpolation final {
  NODE_HEADER(Literal_Text_Interpolation);

  SET_NODE(expression);
  SET_NODE(specifier);
};

// "format node {formatVariable} can be formated"
struct Literal_Textual_Format final {
  NODE_HEADER(Literal_Textual_Format);

  SET_VECTOR_NODE(values);
};


struct Literal_Table_Population final {
  NODE_HEADER(Literal_Table_Population);

  SET_VECTOR_NODE(ranges);
  SET_NODE(expression);
  SET_NODE(map_expression_value);
};

struct Literal_NullPtr final {
  NODE_HEADER(Literal_NullPtr);
};

struct Literal_Table final {
  NODE_HEADER(Literal_Table);
  // for explicit specified values like: { 0, 1, 2, 3 }
  SET_VECTOR_NODE(values);

  // for procedural generated values like: { 0..4 = rand::gauss() }
  SET_NODE(population);
  SET_TYPE(type);

  // can be a ex nihilo node (for primitive types)
  SET_INFERRED_TYPE

  std::vector<size_t> resolved_size;
};

struct Literal_Map final {
  NODE_HEADER(Literal_Map);

  struct Association {
    SET_NODE(key);
    SET_NODE(value);
  };

  std::vector<Association> associations;

  SET_NODE(population);
  size_t size = 1;

  SET_INFERRED_TYPE_(key_type)
  SET_INFERRED_TYPE_(value_type)
  // key + value (no alignment need because it's translated to 2 arrays)
  size_t ty_sizeByte = 0;
};

struct Literal_Tuple final {
  NODE_HEADER(Literal_Tuple);

  struct Field {
    std::string_view name;
    SET_NODE(value);
  };

  std::vector<Field> fields;
};

// first..end or first..=end
struct Literal_Range final {
  NODE_HEADER(Literal_Range);

  SET_NODE(start);
  SET_NODE(end);
  SET_NODE(step);
  bool endInclude = false;
};

// Person{@CIdentity{.name= "Zagreus", .age= 25}} : form/view
// CIdentity{.name= "Zagreus", .age= 25} : facet
struct Literal_Record final {
  NODE_HEADER(Literal_Record);

  SET_NODE(name);
  std::vector<std::string> fields_names; // empty : it's a literal form
  SET_VECTOR_NODE(fields_args);
};

} // namespace ast
  // AST