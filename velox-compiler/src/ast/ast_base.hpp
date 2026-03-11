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

#include <memory>
#include <string>
#include <vector>
#include <span>

#include "ast_data.hpp"
#include "ast_forward.hpp"
#include "lexer/token.hpp"

struct ScriptInfo;
struct Symbol_Data;
struct AType;
struct Visitor_Base;
struct Visitor_Codegen;

std::string mangle_id(const std::string& inId);

namespace llvm
{
class Value;
}

namespace ast
{

struct ICallable {
  [[nodiscard]] virtual type::Function_Proto* get_signature() = 0;
};

// base of
// every
// node in
// AST
struct Node {
  ScriptInfo*              _scr_info = nullptr;
  Token                    _token;
  std::vector<std::string> _scope;

  Node()          = default;
  virtual ~Node() = default;

  [[nodiscard]] size_t get_tok_line() const
  {
    return _token.span.line;
  }
  [[nodiscard]] size_t get_tok_column() const
  {
    return _token.span.col;
  }
  [[nodiscard]] size_t get_tok_size() const
  {
    return _token.span.size;
  }
  [[nodiscard]] size_t get_tok_antepos() const
  {
    return _token.span.anteprocess_pos;
  }
  [[nodiscard]] size_t get_tok_pos() const
  {
    return _token.span.pos;
  }
  [[nodiscard]] std::string         mangle_scope() const;
  [[nodiscard]] bool                is_visible_in(const std::span<const std::string>& other_scope) const;
  [[nodiscard]] virtual std::string debug_str() const       = 0;
  virtual void                      accept(Visitor_Base& v) = 0;
};

inline EPassMode get_defaultParamPassmode(ast::AType& node);

struct AType : virtual Node {
  bool type_isOptional = false;
  bool type_isConst    = false;
  bool type_isVolatile = false;

  [[nodiscard]] virtual std::string mangle_type() const = 0;

  [[nodiscard]] virtual bool compare_with(const AType& other) const = 0;

  [[nodiscard]] bool is_same(const AType& other) const
  {
    if (type_isOptional != other.type_isOptional || type_isConst != other.type_isConst
        || type_isVolatile != other.type_isVolatile || typeid(*this) == typeid(other))
      return false;

    return compare_with(other);
  }

  AType()          = default;
  virtual ~AType() = default;
};

using INFERRED_TYPE = AType*;
using SYM_REF       = Symbol_Data*;

struct ADeclaration : virtual Node {
  std::string name;
  bool        is_exported = false;
  bool        is_extern   = false;

  SYM_REF symbol = nullptr;

  ADeclaration() = default;

  ADeclaration(const ADeclaration&)            = delete;
  ADeclaration& operator=(const ADeclaration&) = delete;
  ADeclaration(ADeclaration&&)                 = delete;
  ADeclaration& operator=(ADeclaration&&)      = delete;

  [[nodiscard]] virtual ESymbolType get_symbol_type() const = 0;

  virtual ~ADeclaration() = default;
};

struct ALocal : ADeclaration {
  virtual ~ALocal() = default;
};

struct AExpression : virtual Node {
  INFERRED_TYPE inferred_type = nullptr;
  virtual ~AExpression()      = default;
};

// for every node who contains a value coded
struct ALiteral : virtual AExpression {
  virtual ~ALiteral() = default;
};

struct AIdentifier : virtual AExpression {
  SYM_REF symbol = nullptr;

  // A::B::C -> C mod A { B } -> B
  [[nodiscard]] virtual const std::string&           get_base_name() const            = 0;
  // mod E { A::B::C } -> 1E1C
  [[nodiscard]] virtual std::string                  mangle_local_name() const        = 0;
  // mod E { A::B::C } -> 1A1B1C
  [[nodiscard]] virtual std::string                  mangle_qualified_name() const    = 0;
  [[nodiscard]] virtual bool                         is_qualified_id() const          = 0;
  [[nodiscard]] virtual std::string                  get_supposed_import_name() const = 0;
  [[nodiscard]] virtual std::span<const std::string> get_qualification_path() const   = 0;

  bool operator==(const AIdentifier& other) const
  {
    return get_base_name() == other.get_base_name() && mangle_qualified_name() == other.mangle_qualified_name();
  }

  virtual ~AIdentifier() = default;
};

struct Expr_ID final : virtual AIdentifier {
  std::string name;

  Expr_ID() = default;
  explicit Expr_ID(const std::string& _name)
    : name(_name)
  {
  }

  std::string get_supposed_import_name() const override
  {
    return "";
  };
  bool is_qualified_id() const override
  {
    return false;
  }
  const std::string& get_base_name() const override
  {
    return name;
  }
  std::string mangle_local_name() const override
  {
    return mangle_scope() + mangle_id(name);
  }
  std::string mangle_qualified_name() const override
  {
    return mangle_id(name);
  }
  std::string debug_str() const override
  {
    return name;
  }
  void accept(Visitor_Base& v) override;


  std::span<const std::string> get_qualification_path() const override
  {
    static const std::span<const std::string> out;
    return out;
  };
};

struct Expr_ID_Qualified final : virtual AIdentifier {
  std::string              name;
  std::vector<std::string> path;

  Expr_ID_Qualified() = default;
  Expr_ID_Qualified(const std::vector<std::string>& p_path, const std::string& _name)
    : name(_name)
  {
    path = p_path;
  }

  bool qualification_at_root_scope    = false; // e.g. ::math::add()
  bool qualification_at_parent_scope  = false; // e.g. super::math::add()
  bool qualification_at_current_scope = false; // e.g. self::math::add()

  bool operator==(const Expr_ID_Qualified& other) const noexcept
  {
    return mangle_local_name() == other.mangle_local_name();
  }

  [[nodiscard]] std::string mangle_path() const;

  std::string get_supposed_import_name() const override
  {
    return path.empty() ? "" : path[0];
  };
  bool is_qualified_id() const override
  {
    return true;
  }
  const std::string& get_base_name() const override
  {
    return name;
  }
  std::string mangle_local_name() const override;
  std::string mangle_qualified_name() const override;

  std::string debug_str() const override;
  void        accept(Visitor_Base& v) override;

  std::span<const std::string> get_qualification_path() const override
  {
    return path;
  };
};

// for every node who need a type resolution
struct Expr_ID_Type final : public AIdentifier, AType {
  std::unique_ptr<AIdentifier>                         name;
  [[maybe_unused]] std::vector<std::unique_ptr<AType>> gen_args;

  [[nodiscard]] std::string mangle_types() const;


  std::string get_supposed_import_name() const override
  {
    return name->get_supposed_import_name();
  };
  bool is_qualified_id() const override
  {
    return name->is_qualified_id();
  }
  const std::string& get_base_name() const override
  {
    return name->get_base_name();
  }
  std::string mangle_local_name() const override
  {
    return name->mangle_local_name();
  }
  std::string mangle_qualified_name() const override
  {
    if (gen_args.empty())
      return name->mangle_qualified_name();
    else
      return name->mangle_qualified_name() + "_" + mangle_type();
  }

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override
  {
    // ID type is compared according to his type inferred
    return inferred_type->is_same(other);
  }

  std::span<const std::string> get_qualification_path() const override
  {
    return name->get_qualification_path();
  };

  std ::string debug_str() const override;

  void accept(Visitor_Base& v) override;
};

struct Root final : public Node {
  std::vector<std::shared_ptr<Node>> global_nodes;

  std::string debug_str() const override
  {
    return "root";
  }
  void accept(Visitor_Base& v) override;
};

} // namespace ast
  // AST
