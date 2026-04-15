#pragma once


#include <memory>
#include <string>
#include <vector>

#include "ast_base.hpp"
#include "ast_codeblock_instruction.hpp"
#include "visitor/visitor_base.hpp"

namespace ast
{
namespace declaration
{
namespace local
{

struct CodeBlock final : public Node, Trait_LLVM_Passage {
  std::vector<CodeBlock_instruction> elements;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "codeblock {...}";
  }
};

// inside of Enum/Tuple pattern
// e.g. Some(a) = value
// e.g. Player { CId.name: name, CId.age: age } = value
// e.g. Player { CId { name: name, age: age } } = value
// e.g. (a, b, c) = triple
// e.g. match val { Some(a) => ... }
// e.g. sys name() { Component(c) => ... }
struct Variable_Binding final : public ALocal, Trait_LLVM_Value {
  ECapability capability = ECapability::NONE;

  // parser resolved
  Pattern*      parent_pattern;
  INFERRED_TYPE type;

  CODEGEN_PASS
  CODEGEN_VALUE

  std::string debug_str() const override
  {
    return "bind " + ECapability_to_str(capability) + " " + declaration_name + "";
  }

  VISTOR_ACCEPT
};

struct Pattern_Element final {
  enum class Kind { Ignore, Binding, Literal };

  std::shared_ptr<Variable_Binding> bind;
  std::unique_ptr<ALiteral>         literal;

  Kind kind = Kind::Ignore;

  Pattern_Element(std::shared_ptr<Variable_Binding> _bind)
    : kind(Kind::Binding)
    , bind(_bind)
  {
  }
  Pattern_Element(std::unique_ptr<ALiteral> _literal)
    : kind(Kind::Literal)
    , literal(std::move(_literal))
  {
  }
  Pattern_Element()
    : kind(Kind::Ignore)
  {
  }

  Node* node()
  {
    switch (kind) {
    case Kind::Binding: return static_cast<Node*>(bind.get());
    case Kind::Literal: return static_cast<Node*>(literal.get());
    case Kind::Ignore:  return nullptr;
    }
  }
};

struct Pattern : public AExpression {
  ECapability                  capability = ECapability::Ref;
  // shared because can be from a match case base reference (so a reference mirror)
  std::shared_ptr<AExpression> right;

  SET_R_VAL

  [[maybe_unused]]
  std::shared_ptr<Node> additive_evaluator;
};

// e.g. [if/elif/while] let Some(a) = value {...}
struct Pattern_Enum final : public Pattern {
  std::unique_ptr<AIdentifier>                  name;
  std::vector<std::unique_ptr<Pattern_Element>> mapping;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "enum pattern[" + name->debug_str() + "]";
  }
};

// e.g. [if/while/for] let (a, b, 10) in triple_collection {...}
struct Pattern_Tuple final : public Pattern {
  std::vector<std::unique_ptr<Pattern_Element>> mapping;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "tuple pattern";
  }
};

// e.g. [if/while] let Player{ CId.name: name, CId.id: 10 }
// e.g. [if/while] let Player{ CId{ name: name, id: 10 } }
struct Pattern_Entity final : public Pattern {
  std::unique_ptr<AIdentifier> name;

  // component identifier, field_name, pattern_element
  std::vector<std::unique_ptr<Pattern_Component>> mapping;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "entity pattern[" + name->debug_str() + "]";
  }
};

struct Pattern_System_Component final : public Pattern {
  std::unique_ptr<AIdentifier>      name;
  std::unique_ptr<Variable_Binding> bind;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "sys component pattern[" + name->debug_str() + "]";
  }
};

// e.g. [if/while] let CId{ name: name, id: 10 }
struct Pattern_Component final : public Pattern {
  std::unique_ptr<AIdentifier> name;

  // field_name,
  // pattern_element
  std::vector<std::tuple<std::string, std::unique_ptr<Pattern_Element>>> mapping;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "component pattern[" + name->debug_str() + "]";
  }
};

// var (a, b, _, d) = call(); var (a, _, c, d) = tupleVariable;
struct Tuple_Destructuring final : public Pattern {
  std::vector<std::shared_ptr<Variable_Binding>> elements;

  EVariableKind kind      = EVariableKind::Const;
  bool          is_static = false;

  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

struct Lambda final : public ACallable, ALocal {
  std::unique_ptr<Lambda_Capture> capture;

  CODEGEN_PASS
  CODEGEN_CALL
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

// let/var a: ptr'type?$ = expression;
struct Variable final : public ALocal, Trait_LLVM_Value {
  [[maybe_unused]] std::shared_ptr<AType>       type;       // infered if nullptr
  [[maybe_unused]] std::unique_ptr<AExpression> expression; // affectation

  ETransfertType assignment = ETransfertType::Copy; // assign type
  EVariableKind  kind       = EVariableKind::Const;
  bool           isStatic   = false;

  CODEGEN_PASS
  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

// ref/mut name = expression
struct Capability final : public ALocal, Trait_LLVM_Value {
  std::unique_ptr<AExpression> right;

  ECapability kind = ECapability::NONE;

  CODEGEN_PASS
  CODEGEN_VALUE
  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    std::string str_kind = kind == ECapability::Mut ? "mut " : "ref ";
    return "capability " + str_kind + declaration_name;
  }
};

struct Capture_Member final : public Node {
  std::unique_ptr<AExpression> name;

  ECapability capability = ECapability::Ref;

  std::string debug_str() const override
  {
    return ECapability_to_str(capability) + " [" + name->debug_str() + "]";
  }

  VISTOR_ACCEPT
};

struct Lambda_Capture final : public Node {
  std::vector<std::unique_ptr<Capture_Member>> elements;

  bool is_all_ref      = false;
  bool is_capture_self = false;

  VISTOR_ACCEPT

  std::string debug_str() const override
  {
    return "capture";
  }
};

// (a: str, copy b: i32 = 10, mut c: f32 = nullptr, args: ...)
struct Parameter final : public ALocal {
  std::shared_ptr<AType>                                type;
  [[maybe_unused]] std::unique_ptr<Node>                defaultValue;
  [[maybe_unused]] std::shared_ptr<Trait_LLVM_Callable> parent_function;

  EPassMode passmode    = EPassMode::Copy;
  bool      is_variadic = false;

  llvm::Argument* llvm_arg = nullptr;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override;
  bool        is_same(const Parameter& other) const
  {
    if (is_variadic != other.is_variadic) return false;
    if (passmode != other.passmode) return false;
    return type->is_same(*other.type);
  }
};

struct Generic_Parameter_Element final : public ALocal {
  std::vector<std::shared_ptr<AType>> generic_references;

  std::string name;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

struct Generic_Parameters final : public ALocal {
  std::vector<std::shared_ptr<Generic_Parameter_Element>> parameters;

  CODEGEN_PASS
  VISTOR_ACCEPT

  std::string debug_str() const override;
};

} // namespace local
  // Local
} // namespace declaration
  // Declaration
} // namespace ast
  // AST