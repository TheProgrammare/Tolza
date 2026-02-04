/*
 *	The Velox programming language - Apache License, Version 2.0 
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <set>

#include "AST_Data.hpp"
#include "AST_Forward.hpp"
#include "Lexer/Token.hpp"
#include "Visitor/Visitor_Base.hpp"

struct ScriptInfo;


inline std::string mangle_id(const std::string& inId) {
	if (inId.empty()) return ""; 	// no name, must return empty string and no 0
	return std::to_string(inId.size()) + inId;
}

namespace AST {

struct ICallable {
    [[nodiscard]] virtual Type::Function_Proto* get_signature() = 0;
};

// base of every node in AST
struct Node {
    Token _token;
    std::vector<std::string> _scope;

    Node() = default;
    virtual void accept(Visitor_Base& v) = 0;
    [[nodiscard]] virtual std::string debug_str() const = 0;
    [[nodiscard]] size_t get_tok_line() const { return _token.span.line; }
    [[nodiscard]] size_t get_tok_column() const { return _token.span.col; }
    [[nodiscard]] size_t get_tok_size() const { return _token.span.size; }
    [[nodiscard]] size_t get_tok_antepos() const { return _token.span.anteprocess_pos; }
    [[nodiscard]] size_t get_tok_pos() const { return _token.span.pos; }
    [[nodiscard]] std::string mangle_scope() const;

    virtual ~Node() = default;
};

std::string make_error(const Node& n, const ScriptInfo& script, const std::string& code, const std::string& err, const std::string& hint);


// for every node with a type
struct AType : virtual Node {
    virtual ~AType() = default;
    bool type_isOptional = false;
    bool type_isConst = false;
    bool type_isVolatile = false;

    virtual bool operator==(const AType& other) const;

    [[nodiscard]] virtual std::string mangle_type() const = 0;
    [[nodiscard]] virtual EPrimType get_type() const = 0;
};

inline EPassMode get_defaultParamPassmode(AST::AType &node);


// for every node who contains a value coded
struct ALiteral : virtual AType {
    virtual ~ALiteral() = default;

    [[nodiscard]] EPrimType get_type() const override = 0;
};


struct ID {
    explicit ID(const std::string &name) 
        : name(std::move(name)) {}
    ID(const std::vector<std::string>& p_path, const std::string &name) 
        : name(std::move(name)) {
            path = p_path;
        }

    ID(const ID&) = default;
    ID& operator=(const ID&) = default;
    ID(ID&&) = default;
    ID& operator=(ID&&) = default;

    AST::Node *parent;
    
    std::string name;
    std::vector<std::string> path;

    // number of path segment before name
    [[maybe_unused]] size_t depth = 0;

    // e.g. ::math::add()
    bool qualification_at_root_scope = false;
    // e.g. super::math::add()
    bool qualification_at_parent_scope = false;
    // e.g. self::math::add()
    bool qualification_at_current_scope = false;

    bool operator==(const ID& other) const noexcept {
        return mangle_local_name() == other.mangle_local_name();
    }

    [[nodiscard]]
    bool is_qualified_id() const { return !path.empty(); }

    [[nodiscard]] std::string debug_str() const;
    [[nodiscard]] virtual std::string mangle_path() const;
    [[nodiscard]] virtual std::string mangle_local_name() const;
    [[nodiscard]] virtual std::string mangle_absolute_name() const;
    [[nodiscard]] virtual std::string mangle_name() const { return mangle_id(name); }
};

struct ADeclaration : virtual Node {
    ID id;

    ADeclaration(const ADeclaration&) = delete;
    ADeclaration& operator=(const ADeclaration&) = delete;
    ADeclaration(ADeclaration&&) = delete;
    ADeclaration& operator=(ADeclaration&&) = delete;

    [[nodiscard]] virtual ESymbolType get_symbol_type() const = 0;

protected:
    ADeclaration() : id("") {}
    explicit ADeclaration(const std::string& name) : id(name) {}
    ADeclaration(const std::vector<std::string>& path, const std::string& name) :
            id(path, name) {}
};

struct ALocal : ADeclaration {

    [[nodiscard]] virtual ESymbolType get_symbol_type() const = 0;
};

// for every node who need a symbolic resolution
struct AReference : virtual Node {
    ID id;

    AReference() : id("") {}
    explicit AReference(const std::string& name) : id(name) {}
    AReference(const std::vector<std::string>& path, const std::string& name) :
            id(path, name) {}
    virtual std::shared_ptr<ADeclaration> get_symbol_resolution() = 0;
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Type_Arguments : public Node {
    std::vector<std::unique_ptr<AType>> arguments;

    bool operator==(const Type_Arguments& other) const {
        if (arguments.size() != other.arguments.size()) return false;
        for (size_t i = 0; i < arguments.size(); i++) {
            const AType& _local_gen = *arguments[i];
            const AType& _other_gen = *other.arguments[i];
            if (&_local_gen != &_other_gen) return false;
        }
        return true;
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] [[nodiscard]] std::string debug_str() const override { 
        if (arguments.empty()) return "";
        std::string outStr;
        for (size_t i = 0; i < arguments.size(); i++) {
            outStr += arguments[i]->debug_str();
            if (i < arguments.size() - 1) outStr += ", ";
        }

        return "<" + outStr + ">";
    }
    [[nodiscard]] std::string ty_mangling() const {
        if (arguments.empty()) return "";
        std::string outStr = "_G" + std::to_string(arguments.size()) + "_";
        for (auto& arg : arguments) {
            outStr += arg->mangle_type() + "_";
        }
        return outStr;
    }
};

struct Identifier_Reference final : public AReference {
    Identifier_Reference() : AReference("") {}
    explicit Identifier_Reference(const std::string& name) : AReference(name) {}
    Identifier_Reference(const std::vector<std::string>& path, const std::string& name) :
        AReference(path, name) {}

    std::shared_ptr<ADeclaration> resolved_sym;

    [[nodiscard]] std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym; }
    [[nodiscard]] std::string debug_str() const override { return id.debug_str(); }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Type_Reference final : public AReference, AType {
    [[maybe_unused]] std::unique_ptr<Type_Arguments> gen_args;

    std::shared_ptr<ADeclaration> resolved_sym;

    Type_Reference() : AReference("") {}
    explicit Type_Reference(const std::string& name) : AReference(name) {}
    Type_Reference(const std::vector<std::string>& path, const std::string& name) :
        AReference(path, name) {}

    [[nodiscard]] std::shared_ptr<ADeclaration> get_symbol_resolution() override { return resolved_sym; }
    [[nodiscard]] std::string debug_str() const override {
        if (gen_args) {
            return "<ty> id[" + id.debug_str() + "]<" + gen_args->debug_str() + ">";
        }
        return "<ty> id[" + id.debug_str() + "]";
    }
    [[nodiscard]] std::string mangle_type() const override { return debug_str(); }
    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] EPrimType get_type() const override { return EPrimType::COUNT; };
};


struct Root : public Node {
    std::vector<std::shared_ptr<Node>> global_nodes;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] [[nodiscard]] std::string debug_str() const override { return "root"; }
};


}



