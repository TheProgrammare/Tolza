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

#include "AST_Data.hpp"
#include "AST_Forward.hpp"
#include "Lexer/Token.hpp"
#include "Visitor/Visitor_Base.hpp"

struct Visitor_Base;

struct ScriptInfo;
struct Symbol_Data;
struct AType;

using SYM_DEFINITION = 
    std::weak_ptr<Symbol_Data>;



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
    virtual ~Node() = default;
    
    [[nodiscard]] size_t get_tok_line() const { return _token.span.line; }
    [[nodiscard]] size_t get_tok_column() const { return _token.span.col; }
    [[nodiscard]] size_t get_tok_size() const { return _token.span.size; }
    [[nodiscard]] size_t get_tok_antepos() const { return _token.span.anteprocess_pos; }
    [[nodiscard]] size_t get_tok_pos() const { return _token.span.pos; }
    [[nodiscard]] std::string mangle_scope() const;
    
    [[nodiscard]] virtual std::string debug_str() const = 0;
    virtual void accept(Visitor_Base& v) = 0;
    
};

inline EPassMode get_defaultParamPassmode(AST::AType &node);



struct AType : virtual Node {
    bool type_isOptional = false;
    bool type_isConst = false;
    bool type_isVolatile = false;
    
    virtual ~AType() = default;
};

using INFERRED_TYPE = 
    AType*;

struct ADeclaration : virtual Node {
    std::string name;

    SYM_DEFINITION symbol;
    
    ADeclaration(const ADeclaration&) = delete;
    ADeclaration& operator=(const ADeclaration&) = delete;
    ADeclaration(ADeclaration&&) = delete;
    ADeclaration& operator=(ADeclaration&&) = delete;
    
    virtual ESymbolType get_symbol_type() const = 0;

    virtual ~ADeclaration() = default;
};

struct ALocal : ADeclaration {
    virtual ~ALocal() = default;
};

struct AExpression : virtual Node {
    INFERRED_TYPE inferred_type;
    virtual ~AExpression() = default;
};

// for every node who contains a value coded
struct ALiteral : virtual AExpression {
    virtual ~ALiteral() = default;
};

struct Expr_ID final : virtual AExpression {
    std::string name;

     explicit Expr_ID(const std::string &_name) 
        : name(_name) 
    {}

    std::string debug_str() const override { return "identifier \"" + name + "\""; }
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

struct Expr_ID_Qualified final : virtual AExpression {
    std::string name;
    std::vector<std::string> path;

    Expr_ID_Qualified(const std::vector<std::string>& p_path, const std::string &_name) 
        : name(_name) {
        path = p_path;
    }

    // e.g. ::math::add()
    bool qualification_at_root_scope = false;
    // e.g. super::math::add()
    bool qualification_at_parent_scope = false;
    // e.g. self::math::add()
    bool qualification_at_current_scope = false;

    bool operator==(const Expr_ID_Qualified& other) const noexcept {
        return mangle_local_name() == other.mangle_local_name();
    }

    [[nodiscard]]
    bool is_qualified_id() const { return !path.empty(); }

    [[nodiscard]] std::string debug_str() const override;
    [[nodiscard]] virtual std::string mangle_path() const;
    [[nodiscard]] virtual std::string mangle_local_name() const;
    [[nodiscard]] virtual std::string mangle_absolute_name() const;
    [[nodiscard]] virtual std::string mangle_name() const { return mangle_id(name); }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// for every node who need a type resolution
struct Expr_ID_Generic final : public AExpression {
    std::unique_ptr<AExpression> base_name;
    [[maybe_unused]] std::vector<std::unique_ptr<AType>> gen_args;

    std::string debug_str() const override { return "identifier type"; }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Root final : public Node {
    std::vector<std::shared_ptr<Node>> global_nodes;

    std::string debug_str() const override { return "root"; }
  
    void accept(Visitor_Base& v) override { v.visit(*this); }
};


}

