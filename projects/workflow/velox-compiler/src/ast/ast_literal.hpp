#pragma once

#include "nexus/ast/ast.hpp"
#include "nexus/type.hpp"
#include "ast_numeric_128_bits.hpp"
#include <vector>

namespace ast
{

AST_NODE(Literal_Boolean)
{
  bool val = false;
};

AST_NODE(Literal_Integral)
{
  Int128                     val;
  ::type::EPrimitiveTypeKind type = ::type::EPrimitiveTypeKind::NONE;
};

AST_NODE(Literal_Fixed_Point)
{
  Int128                     val;
  ::type::EPrimitiveTypeKind raw_type = ::type::EPrimitiveTypeKind::NONE;
  size_t                     scale    = 1;
};

AST_NODE(Literal_Floating_Point)
{
  Float128                   val;
  ::type::EPrimitiveTypeKind type = ::type::EPrimitiveTypeKind::NONE;
};

// Latin-1 encoding
AST_NODE(Literal_Cune)
{
  char val = 0x0;
};

AST_NODE(Literal_Rune)
{
  std::string_view code_points;
};


AST_NODE(Literal_Text_Pure)
{
  std::string_view  val;
  ::type::ETextType text_type = ::type::ETextType::str;
};

// format_spec ::= [options][width][grouping]["." precision][type]
// options ::= [[fill]align] [sign]["z"]["#"]["0"]
// fill ::= <any character>
// align ::= "<" | ">" | "=" | "^"
// sign ::= "+" | "-" | " "
// width ::= digit + grouping ::= "," | "_"
// precision ::= digit + type ::= "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" | "s" | "x" | "X" |
// "%"
AST_NODE(Literal_Format_Specifier)
{
  std::string_view src_Str;
  char             fill = '\0'; // char fill, e.g. '0', '*', ' '
  enum class EAlign { Right, Left, Center, Justify };
  EAlign align = EAlign::Right; // '>', '<', '^', '~', '='

  // Sign
  enum class ESign { None, Pos, Neg, Space };
  ESign sign           = ESign::None; // '+', '-', ' ' (space)
  bool  signBeforeFill = false;

  // Numeric
  enum class EPrefix { None, Hex, HEX, Bin, Oct };
  EPrefix prefix   = EPrefix::None; // '#' for  0x, 0b, 0o
  bool    zero_pad = false;         // '0' fill to left
  SET_NODE(width);
  char grouping_char; // ',' or '_' or ''' for thousands

  // precision
  // and
  // type
  SET_NODE(precision); // decimal number
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
  // 's', 'b', 'c', 'd', 'o', 'x', 'X', 'n',
  // 'e', 'E', 'f', 'F', 'g', 'G', '%'
  EDisplayFormat display_format = EDisplayFormat::String;
};

// "{expression}" "{expression:spec}"
AST_NODE(Literal_Text_Interpolation)
{
  SET_NODE(expression);
  SET_NODE(specifier);
};

// "format node {formatVariable} can be formated"
AST_NODE(Literal_Textual_Format)
{
  SET_VECTOR_NODE(values);
};


AST_NODE(Literal_Table_Population)
{
  SET_VECTOR_NODE(ranges);
  SET_NODE(expression);
  SET_NODE(map_expression_value);
};

AST_NODE(Literal_Table)
{ // for explicit specified values like: { 0, 1, 2, 3 }
  SET_VECTOR_NODE(values);

  // for procedural generated values like: { 0..4 = rand::gauss() }
  SET_NODE(population);

  // can be a ex nihilo node (for primitive types)
  SET_INFERRED_TYPE

  std::vector<size_t> resolved_size;
};

AST_NODE(Literal_Map)
{
  struct Association {
    SET_NODE(key);
    ;
    SET_NODE(value);
    ;
  };

  std::vector<Association> associations;

  SET_NODE(population);
  size_t size = 1;

  SET_INFERRED_TYPE_(key_type)
  SET_INFERRED_TYPE_(value_type)
  // key + value (no alignment need because it's translated to 2 arrays)
  size_t ty_sizeByte = 0;
};

AST_NODE(Literal_Enum)
{
  // path = enum name
  // base_name = enum element
  SET_NODE(name);
  SET_VECTOR_NODE(member_values);
};

AST_NODE(Literal_Tuple)
{
  struct Field {
    std::string_view name;
    SET_NODE(value);
    ;
  };

  std::vector<Field> fields;
};

// first..end or first..=end
AST_NODE(Literal_Range)
{
  SET_NODE(start);
  SET_NODE(end);
  SET_NODE(step);
  bool endInclude = false;
};

// CIdentity{ name: "Zagreus", age: 25 }
// can be component, union, enum, flag
AST_NODE(Literal_Structured_Data)
{
  SET_NODE(name);
  SET_VECTOR_NODE(fields_args);
};

// Person{ CIdentity.name: "Zagreus", CIdentity.age: 25 }
// not the same as Person("Zagreus", 32) it's a call of constructor
AST_NODE(Literal_Entity)
{
  SET_NODE(name);
  SET_VECTOR_NODE(component_args);
};

AST_NODE(Literal_Iterator)
{
  SET_NODE(collection);
};

} // namespace ast
  // AST