#pragma once

#include <memory>

#include "ast_base.hpp"
#include "ast_numeric_128_bits.hpp"

namespace ast
{
namespace literal
{

struct Boolean final : public ALiteral {
  bool val = false;

  Boolean();

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "literal bool(" + std::to_string(val) + ")";
  }
};

struct Integral final : public ALiteral {
  Int128    val;
  EPrimType type = EPrimType::i64;

  Integral();

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "literal " + EPrimTy_to_str(type) + "(" + val.i128_to_string() + ")";
  }
};

struct Decimal final : public ALiteral {
  Int128 val;
  size_t integral_num = 1;
  size_t decimal_num  = 1;

  bool is_unsigned = false;

  Decimal();

  bool operator==(const ALiteral& other) const
  {
    if (auto ptr = dynamic_cast<const Decimal*>(&other))
      return integral_num == ptr->integral_num && decimal_num == ptr->decimal_num;
    return false;
  }

  std::string debug_str() const override
  {
    if (is_unsigned) return "literal udeci(" + val.i128_to_string() + ")";
    return "literal deci(" + val.i128_to_string() + ")";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Floating final : public ALiteral {
  Float128  val;
  EPrimType type = EPrimType::f64;

  Floating();

  std::string debug_str() const override
  {
    return "literal " + EPrimTy_to_str(type) + "(" + val.float128_to_string() + ")";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

// Latin-1 encoding
struct ASCII final : public ALiteral {
  char val = 0x0;

  ASCII();

  std::string debug_str() const override
  {
    return "literal ascii('" + std::to_string(val) + "')";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct UTF32 final : public ALiteral {
  std::string codePoints;

  UTF32();

  std::string debug_str() const override
  {
    return "literal utf32('" + codePoints + "')";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Text final : public ALiteral {
  std::u32string val;
  size_t         length   = 1;
  bool           is_ascii = false;

  Text();

  std::string debug_str() const override
  {
    return "\"" + std::string(val.begin(), val.end()) + "\"";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
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

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

// "{expression}"
// "{expression:spec}"
struct Text_Interpolation final : public AExpression {
  std::unique_ptr<AExpression>      expression;
  std::unique_ptr<Format_Specifier> spec;

  std::string debug_str() const override
  {
    return "text interpolation";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Textual_Element final {
  enum class Kind { Text, Lerp };

  Kind                         kind;
  std::unique_ptr<AExpression> val;

  Textual_Element(std::unique_ptr<Text> text)
    : val(std::move(text))
    , kind(Kind::Text)
  {
  }
  Textual_Element(std::unique_ptr<Text_Interpolation> lerp)
    : val(std::move(lerp))
    , kind(Kind::Lerp)
  {
  }
};

// "format node {formatVariable} can be formated"
struct Textual_Format final : public ALiteral {
  std::vector<Textual_Element> values;

  std::string debug_str() const override;

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Table_Population final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> ranges;
  std::unique_ptr<AExpression>              expression;
  std::unique_ptr<AExpression>              map_expression_value;

  // can be a ex nihilo node (for primitive types)
  INFERRED_TYPE element_definition;

  std::string debug_str() const override
  {
    return "table population";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Table final : public ALiteral, AType {
  // for explicit specified values like: { 0, 1, 2, 3 }
  [[maybe_unused]]
  std::vector<std::unique_ptr<AExpression>> values;
  // for procedural generated values like: { 0..4 = rand::gauss() }
  [[maybe_unused]]
  std::unique_ptr<Table_Population> population;

  // can be a ex nihilo node (for primitive types)
  INFERRED_TYPE element_type;

  std::vector<size_t> resolved_size;

  bool is_matrix() const
  {
    return resolved_size.size() > 1;
  }
  bool is_table_population() const
  {
    return population.get();
  }

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Table*>(&other)) {
      return element_type->is_same(*ptr->element_type) && resolved_size == ptr->resolved_size;
    }
    return false;
  }

  std::string debug_str() const override;

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
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

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "literal map[" + std::to_string(keys.size()) + "]";
  }
};

struct Enum final : public ALiteral {
  // path = enum name
  // base_name = enum element
  std::unique_ptr<ast::AIdentifier>         name;
  std::vector<std::unique_ptr<AExpression>> member_values;

  std::string debug_str() const override
  {
    return "literal enum[" + name->debug_str() + "]";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Tuple final : public ALiteral {
  std::vector<std::unique_ptr<AExpression>> values;
  std::vector<std::string>                  name_fields;

  std::vector<std::shared_ptr<AType>> tys; // size for each element type

  std::string debug_str() const override
  {
    if (name_fields.empty()) return "literal tuple(" + std::to_string(values.size()) + ")";
    return "literal named tuple(" + std::to_string(values.size()) + ")";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

// first..end or first..=end
struct Range final : public ALiteral {
  std::unique_ptr<AExpression> start;
  std::unique_ptr<AExpression> end;
  std::unique_ptr<AExpression> step;
  bool                         endInclude = false;

  std::string debug_str() const override
  {
    return "literal range";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

// CIdentity{ name: "Zagreus", age: 25 }
struct Component final : public ALiteral {
  std::unique_ptr<ast::AIdentifier>                       name;
  std::vector<std::unique_ptr<expression::Call_Argument>> field_args;

  std::string debug_str() const override
  {
    return "literal component[" + name->debug_str() + "]";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

// Person{ CIdentity.name: "Zagreus", CIdentity.age: 25 }
// not the same as Person("Zagreus", 32) it's a call of constructor
struct Entity final : public ALiteral {
  std::unique_ptr<ast::AIdentifier>       name;
  std::vector<std::unique_ptr<Component>> comp_args;

  std::string debug_str() const override
  {
    return "literal entity[" + name->debug_str() + "]";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

struct Iterator final : public ALiteral {
  std::unique_ptr<AExpression> collection;

  std::string debug_str() const override
  {
    return "literal iterator";
  }

  void         accept(Visitor_Base& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;
};

} // namespace literal
  // Literal
} // namespace ast
  // AST