#pragma once

// for float128
#include <sstream>
#include <iomanip>
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APInt.h"
#include "llvm/ADT/SmallVector.h"

#include "AST_Base.hpp"

namespace AST {
namespace Literal {

class Float128 {
public:
    llvm::APFloat val;

    // Initialise à zéro
    Float128()
      : val(llvm::APFloatBase::IEEEquad(), llvm::StringRef("0.0"))
    {}

    // Conversion string → APFloat
    void string_to_f128(const std::string &s) {
        llvm::StringRef sr(s);
        llvm::APFloat tmp(llvm::APFloatBase::IEEEquad(), sr);
        val = tmp;
    }

    // Conversion APFloat → string
    std::string float128_to_string(int precision = 36) const {
        llvm::SmallVector<char, 128> buf;
        val.toString(buf, precision);
        return std::string(buf.begin(), buf.end());
    }
};


class Int128 {
public:
    llvm::APInt val;

    Int128() : val(128, 0, true) {} // 128 bits signed, valeur 0

    // Conversion string → APInt
    void string_to_i128(const std::string& s, int base = 10) {
        llvm::APInt tmp(128, 0, true); // 128 bits signed
        bool ok = false;

        try {
            tmp = llvm::APInt(128, llvm::StringRef(s), base); // base 10 ou 16
            ok = true;
        } catch (...) {
            ok = false;
        }

        if (!ok) {
            throw std::invalid_argument("String contains invalid characters for int128");
        }

        val = tmp;
    }

    // Conversion APInt → string
    std::string i128_to_string(int base = 10) const {
        if (base != 10 && base != 16) {
            throw std::invalid_argument("Base must be 10 or 16");
        }
        llvm::SmallVector<char, 128> buf;
		val.toString(buf, base, true);
		return std::string(buf.begin(), buf.end());

    }
};


struct Boolean : public ALiteral {
    bool val = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "bool(" + std::to_string(val) + ")"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::boolean; }
    [[nodiscard]] std::string mangle_type() const override { return "b"; }
};

struct Integral : public ALiteral {
    Int128 val;
    EPrimType type = EPrimType::i64;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return EPrimTy_to_str(type) + "(" + val.i128_to_string() + ")"; }
    [[nodiscard]] EPrimType get_type() const override { return type; }
    [[nodiscard]] std::string mangle_type() const override { return EPrimTy_to_str(type); }

};

struct Decimal : public ALiteral {
    Int128 val;
    size_t integral_num = 1;
    size_t decimal_num = 1;

    bool is_unsigned = false;

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Decimal*>(&other))
            return integral_num == ptr->integral_num && decimal_num == ptr->decimal_num;
        return false;
    }

    [[nodiscard]] std::string debug_str() const override { 
        if (is_unsigned) 
        return "udeci(" + val.i128_to_string() + ")";
        return "deci(" + val.i128_to_string() + ")";
    }
    [[nodiscard]] EPrimType get_type() const override { return is_unsigned ? EPrimType::udeci : EPrimType::deci; }
    [[nodiscard]] std::string mangle_type() const override
    {
        std::string out = is_unsigned ? "ud" : "d";
        return out + "_" + std::to_string(integral_num) + "_" + std::to_string(decimal_num);
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Floating : public ALiteral {
    Float128 val;
    EPrimType type = EPrimType::f64;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return EPrimTy_to_str(type) + "(" + val.float128_to_string() + ")"; }
    [[nodiscard]] EPrimType get_type() const override { return type; }
    [[nodiscard]] std::string mangle_type() const override { return EPrimTy_to_str(type); }
};

// Latin-1 encoding
struct ASCII : public ALiteral {
    char val = 0x0;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "ascii('" + std::to_string(val) + "')"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::ASCII; }
    [[nodiscard]] std::string mangle_type() const override { return "aii"; }
};
    
struct UTF32 : public ALiteral {
    std::string codePoints;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "utf32('" + codePoints + "')"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::UTF32; }
    [[nodiscard]] std::string mangle_type() const override { return "utf"; }
};

struct Text : public ALiteral {
    std::u32string val;
    size_t length = 1;
    bool is_ascii = false;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "text(\"" + std::string(val.begin(), val.end()) + "\")"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::text; }
    [[nodiscard]] std::string mangle_type() const override { return "txt"; }
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
    std::unique_ptr<Node> width;    // min width
    char grouping_char; // ',' or '_' or ''' for thousands 

    // precision and type
    std::unique_ptr<Node> precision; // decimal number
    enum class EDisplayFormat { String, Binary, Character, Decimal, Octal, Hex, HEX, Number, e, E, Fixed, FIXED, g, G, Percentage  };
    EDisplayFormat display_format = EDisplayFormat::String;     // 's', 'b', 'c', 'd', 'o', 'x', 'X', 'n', 'e', 'E', 'f', 'F', 'g', 'G', '%'

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<format> specifier \":" + src_Str + "\""; }
};

// "{expression}" "{expression:spec}"
struct Text_Lerp : public Node {
    std::unique_ptr<AST::Node> expression;
    std::unique_ptr<Format_Specifier> spec;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "text_lerp"; }
};

struct Textual_Element {
    enum class Kind { Text, Lerp };

    Kind kind;
    std::unique_ptr<Node> val;

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

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "format_text"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::text; }
    [[nodiscard]] std::string mangle_type() const override { return "ftxt"; }
};


struct Table_Population : public ALiteral {
    std::vector<std::unique_ptr<Node>> ranges;
    std::unique_ptr<Node> expression;
    std::unique_ptr<Node> map_expression_value;

    std::shared_ptr<AType> resolved_elem_ty;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<table population>"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Array; }
    [[nodiscard]] std::string mangle_type() const override { return "tpop"; }
};

struct Table : public ALiteral {
    // for explicit specified values like: { 0, 1, 2, 3 }
    [[maybe_unused]]
    std::vector<std::unique_ptr<Node>> values;
    // for procedural generated values like: { 0..4 = rand::gauss() }
    [[maybe_unused]]
    std::unique_ptr<Table_Population> population;

    std::shared_ptr<AType> resolved_elem_ty;

    std::vector<size_t> resolved_size;

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Table*>(&other)) {
            if (ptr->resolved_elem_ty.get() != resolved_elem_ty.get()) return false;
            if (ptr->resolved_size.size() != resolved_size.size()) return false;

            for (size_t i = 0; i < resolved_size.size(); i++) {
                const size_t dim = resolved_size[i];
                const size_t other_dim = ptr->resolved_size[i];
                if (dim != other_dim) return false;
            }
            return true;
        }
        return false;
    }

    [[nodiscard]] bool is_matrix() const { return resolved_size.size() > 1; }
    [[nodiscard]] bool is_table_population() const { return population.get(); }

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string mangle_type() const override { 
        std::string out;
        out = "t" + std::to_string(resolved_size.size()) + "_";
        for (size_t i = 0; i < resolved_size.size(); i++) {
            out += std::to_string(resolved_size[i]) + "_";
        }
        out += "_" + resolved_elem_ty->mangle_type();
        return out;
    }
    [[nodiscard]] std::string debug_str() const override { 
        std::string out;
        out = "table[" + std::to_string(resolved_size.size()) + ":";
        for (size_t i = 0; i < resolved_size.size(); i++) {
            out += std::to_string(resolved_size[i]) + "x";
        }
        out += " -> " + resolved_elem_ty->mangle_type() + "]";
        return out;
    }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Array; }
};


struct Map : public ALiteral {
    std::vector<std::unique_ptr<Node>> keys;
    std::vector<std::unique_ptr<Node>> values;

    [[maybe_unused]]
    std::unique_ptr<Table_Population> population;

    size_t size = 1;

    std::shared_ptr<AType> resolved_key_ty;
    std::shared_ptr<AType> resolved_value_ty;

    // key + value (no alignment need because it's translated to 2 arrays)
    size_t ty_sizeByte = 0;

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Map*>(&other)) {
            return size == ptr->size 
                && resolved_key_ty != ptr->resolved_key_ty 
                && resolved_value_ty != ptr->resolved_value_ty;
        }
        return false;
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "map[" + std::to_string(keys.size()) + "]"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::map; }
    [[nodiscard]] std::string mangle_type() const override { return "map"; }
};


struct Tuple : public ALiteral {
    std::vector<std::unique_ptr<Node>> values;
    std::vector<std::string> name_fields;

    std::vector<std::shared_ptr<AType>>	tys;		// size for each element type

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Tuple*>(&other)) {
            if (tys.size() != ptr->tys.size()) return false;
            for (size_t i = 0; i < tys.size(); i++) {
                if (tys[i] != ptr->tys[i]) return false;
            }
            return true;
        }
        return false;
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string mangle_type() const override {
        std::string outStr = "tu" + std::to_string(tys.size()); 
        for (auto& elem : tys) {
            outStr += "_" + elem->mangle_type();
        }
        return outStr;
    }
    [[nodiscard]] std::string debug_str() const override {
        if (name_fields.empty()) return "tuple(" + std::to_string(values.size()) + ")";
        return "named tuple(" + std::to_string(values.size()) + ")";
    }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::tuple; }
};



// first..end or first..=end
struct Range : public ALiteral {
    std::unique_ptr<Node> start;
    std::unique_ptr<Node> end;
    std::unique_ptr<Node> step;
    bool endInclude = false;

    std::shared_ptr<AType> resolved_type_start;
    std::shared_ptr<AType> resolved_type_end;
    std::shared_ptr<AType> resolved_type;

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Range*>(&other)) {
            return endInclude == ptr->endInclude 
                && resolved_type == ptr->resolved_type;
        }
        return false;
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string mangle_type() const override { return "rng_" + resolved_type->mangle_type(); };
    [[nodiscard]] std::string debug_str() const override { return "<lit> range"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Range; }
};

// CIdentity{ name: "Zagreus", age: 25 }
struct Component : public AReference, ALiteral {
    std::unique_ptr<Type_Arguments> gen_args;
    std::vector<std::unique_ptr<Reference::Call_Argument>> field_args;

    // symbol resolution
    std::shared_ptr<Declaration::ECS::Component> resolved_sym;

    [[nodiscard]] std::shared_ptr<ADeclaration> get_symbol_resolution() override { return std::static_pointer_cast<ADeclaration>(resolved_sym); }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<lit> comp[" + id.debug_str() + "]"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Component; }
    [[nodiscard]] std::string mangle_type() const override { return "cp_" + id.debug_str(); };
};

// Person{ CIdentity.name: "Zagreus", CIdentity.age: 25 }
// not the same as Person("Zagreus", 32) it's a call of constructor
struct Entity: public AReference, ALiteral {
    // before callee name
    std::unique_ptr<Type_Arguments> gen_args;
    std::vector<std::unique_ptr<Component>> comp_args;

    // symbol resolution
    std::shared_ptr<Declaration::ECS::Entity> resolved_sym;

    std::shared_ptr<ADeclaration> get_symbol_resolution() override { return std::dynamic_pointer_cast<ADeclaration>(resolved_sym); }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<lit> entity[" + id.debug_str() + "]"; }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Entity; }
    [[nodiscard]] std::string mangle_type() const override { return "et_" + id.debug_str(); }
};


struct Iterator : public ALiteral {
    std::unique_ptr<Node> collection;

    std::shared_ptr<AType> resolved_ty;

    bool operator==(const ALiteral& other) const {
        if (auto ptr = dynamic_cast<const Iterator*>(&other)) {
            return resolved_ty == ptr->resolved_ty;
        }
        return false;
    }

    [[nodiscard]] std::string debug_str() const override { return "<lit> iter"; }
    [[nodiscard]] std::string mangle_type() const override { return "iter_" + resolved_ty->mangle_type(); }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::Iterator; }
    
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

}
}