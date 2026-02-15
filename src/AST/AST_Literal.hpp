#pragma once

#include <memory>

#include "AST_Base.hpp"
#include "AST_Numeric_128_bits.hpp"

namespace AST {
namespace Literal {

struct Boolean : public ALiteral {
    bool val = false;

    Boolean();

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "bool(" + std::to_string(val) + ")"; }
};

struct Integral : public ALiteral {
    Int128 val;
    EPrimType type = EPrimType::i64;

    Integral();

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return EPrimTy_to_str(type) + "(" + val.i128_to_string() + ")"; }

};

struct Decimal : public ALiteral {
    Int128 val;
    size_t integral_num = 1;
    size_t decimal_num = 1;

    bool is_unsigned = false;

    Decimal();

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Decimal*>(&other))
            return integral_num == ptr->integral_num && decimal_num == ptr->decimal_num;
        return false;
    }

    std::string debug_str() const override { 
        if (is_unsigned) return "udeci(" + val.i128_to_string() + ")";
        return "deci(" + val.i128_to_string() + ")";
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Floating : public ALiteral {
    Float128 val;
    EPrimType type = EPrimType::f64;

    Floating();

    std::string debug_str() const override { return EPrimTy_to_str(type) + "(" + val.float128_to_string() + ")"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// Latin-1 encoding
struct ASCII : public ALiteral {
    char val = 0x0;

    ASCII();

    std::string debug_str() const override { return "ascii('" + std::to_string(val) + "')"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};
    
struct UTF32 : public ALiteral {
    std::string codePoints;

    UTF32();

    std::string debug_str() const override { return "utf32('" + codePoints + "')"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Text : public ALiteral {
    std::u32string val;
    size_t length = 1;
    bool is_ascii = false;

    Text();

    std::string debug_str() const override { return "text(\"" + std::string(val.begin(), val.end()) + "\")"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// format_spec ::= [options][width][grouping]["." precision][type]
// options     ::= [[fill]align] [sign]["z"]["#"]["0"]
// fill        ::= <any character>
// align       ::= "<" | ">" | "=" | "^"
// sign        ::= "+" | "-" | " "
// width       ::= digit +
// grouping    ::= "," | "_"
// precision   ::= digit +
// type        ::= "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g"
// | "G" | "n" | "o" | "s" | "x" | "X" | "%"
struct Format_Specifier : public Node {
    std::string src_Str;
    char fill = '\0';    // char fill, e.g. '0', '*', ' '
    enum class EAlign { Right, Left, Center, Justify };
    EAlign align = EAlign::Right;   // '>', '<', '^', '~', '='

    // Sign
    enum class ESign { None, Pos, Neg, Space };
    ESign sign = ESign::None;    // '+', '-', ' ' (space)
    bool signBeforeFill = false;

    // Numeric
    enum class EPrefix { None, Hex, HEX, Bin, Oct };
    EPrefix prefix = EPrefix::None; // '#' for  0x, 0b, 0o
    bool zero_pad = false;			// '0' fill to left
    std::unique_ptr<AExpression> width;    // min width
    char grouping_char; // ',' or '_' or ''' for thousands 

    // precision and type
    std::unique_ptr<AExpression> precision; // decimal number
    enum class EDisplayFormat { String, Binary, Character, Decimal, Octal, Hex, HEX, Number, e, E, Fixed, FIXED, g, G, Percentage  };
    EDisplayFormat display_format = EDisplayFormat::String;     // 's', 'b', 'c', 'd', 'o', 'x', 'X', 'n', 'e', 'E', 'f', 'F', 'g', 'G', '%'

    std::string debug_str() const override { return "<format> specifier \":" + src_Str + "\""; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// "{expression}" "{expression:spec}"
struct Text_Lerp : public AExpression {
    std::unique_ptr<AExpression> expression;
    std::unique_ptr<Format_Specifier> spec;

    std::string debug_str() const override { return "text_lerp"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Textual_Element {
    enum class Kind { Text, Lerp };

    Kind kind;
    std::unique_ptr<AExpression> val;

    Textual_Element(std::unique_ptr<Text> text) 
        : val(std::move(text))
        , kind(Kind::Text) {}
    Textual_Element(std::unique_ptr<Text_Lerp> lerp)
        : val(std::move(lerp))
        , kind(Kind::Lerp) {}
};

// "format node {formatVariable} can be formated"
struct Textual_Format : public ALiteral {
    std::vector<Textual_Element> values;

    std::string debug_str() const override { return "format_text"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Table_Population : public ALiteral {
    std::vector<std::unique_ptr<AExpression>> ranges;
    std::unique_ptr<AExpression> expression;
    std::unique_ptr<AExpression> map_expression_value;

    // can be a ex nihilo node (for primitive types)
    SYM_DEFINITION element_definition;

    std::string debug_str() const override { return "<table population>"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Table : public ALiteral {
    // for explicit specified values like: { 0, 1, 2, 3 }
    [[maybe_unused]]
    std::vector<std::unique_ptr<AExpression>> values;
    // for procedural generated values like: { 0..4 = rand::gauss() }
    [[maybe_unused]]
    std::unique_ptr<Table_Population> population;

    // can be a ex nihilo node (for primitive types)
    SYM_DEFINITION element_definition;

    std::vector<size_t> resolved_size;

    bool is_matrix() const { return resolved_size.size() > 1; }
    bool is_table_population() const { return population.get(); }

    std::string debug_str() const override;
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Map : public ALiteral {
    std::vector<std::unique_ptr<AExpression>> keys;
    std::vector<std::unique_ptr<AExpression>> values;

    [[maybe_unused]]
    std::unique_ptr<Table_Population> population;

    size_t size = 1;

    SYM_DEFINITION key_definition;
    SYM_DEFINITION value_definition;

    // key + value (no alignment need because it's translated to 2 arrays)
    size_t ty_sizeByte = 0;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    std::string debug_str() const override { return "map[" + std::to_string(keys.size()) + "]"; }
};


struct Tuple : public ALiteral {
    std::vector<std::unique_ptr<AExpression>> values;
    std::vector<std::string> name_fields;

    std::vector<std::shared_ptr<AType>>	tys;		// size for each element type

    std::string debug_str() const override {
        if (name_fields.empty()) return "tuple(" + std::to_string(values.size()) + ")";
        return "named tuple(" + std::to_string(values.size()) + ")";
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};



// first..end or first..=end
struct Range : public ALiteral {
    std::unique_ptr<AExpression> start;
    std::unique_ptr<AExpression> end;
    std::unique_ptr<AExpression> step;
    bool endInclude = false;

    std::string debug_str() const override { return "literal range"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// CIdentity{ name: "Zagreus", age: 25 }
struct Component : public ALiteral {
    std::string name;
    std::vector<std::unique_ptr<Expression::Call_Argument>> field_args;

    std::string debug_str() const override { return "literal component \"" + name + "\""; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// Person{ CIdentity.name: "Zagreus", CIdentity.age: 25 }
// not the same as Person("Zagreus", 32) it's a call of constructor
struct Entity: public ALiteral {
    std::string name;
    std::vector<std::unique_ptr<Component>> comp_args;

    std::string debug_str() const override { return "literal entity \"" + name + "\""; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Iterator : public ALiteral {
    std::unique_ptr<AExpression> collection;

    std::string debug_str() const override { return "literal iterator"; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

}
}