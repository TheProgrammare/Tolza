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

std::string make_error(const Node& n, const ScriptInfo& script, const std::string& code, const std::string& err, const std::string& hint);

inline EPassMode get_defaultParamPassmode(AST::AType &node);


// for every node who contains a value coded
struct ALiteral : virtual Node {
    virtual ~ALiteral() = default;
};

struct AType : virtual Node {
    bool type_isOptional = false;
    bool type_isConst = false;
    bool type_isVolatile = false;

    virtual ~AType() = default;
};

struct ID {

    explicit ID(const std::string &name) 
        : name(std::move(name)) {}
    ID(const std::vector<std::string>& p_path, const std::string &name) 
        : name(std::move(name)) {
            path = p_path;
        }

    ID() = default;
    ID(const ID&) = default;
    ID& operator=(const ID&) = default;
    ID(ID&&) = default;
    ID& operator=(ID&&) = default;

    AST::Node *parent = nullptr;
    
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
};

// for every node who need a symbolic resolution
struct AReference : virtual Node {
    ID id;
    SYM_DEFINITION definition;

    AReference() : id("") {}
    explicit AReference(const std::string& name) : id(name) {}
    AReference(const std::vector<std::string>& path, const std::string& name) :
            id(path, name) {}

    std::string debug_str() const override { return "<ref> id[" + id.debug_str() + "]"; };
    void accept(Visitor_Base& v) override { v.visit(*this); }
};

// for every node who need a type resolution
struct AType_Reference : public AReference, AType {
    [[maybe_unused]] std::vector<std::unique_ptr<AType>> gen_args;

    AType_Reference() : AReference("") {}
    explicit AType_Reference(const std::string& name) : AReference(name) {}
    AType_Reference(const std::vector<std::string>& path, const std::string& name) :
        AReference(path, name) {}

    std::string debug_str() const override {
        if (!gen_args.empty()) {
            std::string out = "<type> id[" + id.debug_str() + "]<";
            for (auto &arg : gen_args)
                out += id.debug_str() + ", ";
            return out + ">";
        }
        return "<type> id[" + id.debug_str() + "]";
    }

    void accept(Visitor_Base& v) override { v.visit(*this); }
};


struct Root : public Node {
    std::vector<std::shared_ptr<Node>> global_nodes;

    std::string debug_str() const override { return "root"; }
  
    void accept(Visitor_Base& v) override { v.visit(*this); }
};




}



