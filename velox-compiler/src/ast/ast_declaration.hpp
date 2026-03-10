#pragma once

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ast_base.hpp"
#include "ast_forward.hpp"
#include "ast/ast_data.hpp"

struct ModuleExportation;

namespace ast
{
namespace declaration
{

struct Enum_Element final : public AType {
  // if empty : it's a simple enum key element
  std::string                         name;
  std::vector<std::shared_ptr<AType>> types;
  size_t                              position = 0;

  std::shared_ptr<Enum> parent_enum;

  std::string debug_str() const override;

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Enum_Element*>(&other)) {
      return parent_enum == ptr->parent_enum && name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Enum final : public ADeclaration, AType {
  std::vector<std::unique_ptr<Enum_Element>> variants;

  bool                       isGlobal              = true;
  size_t                     discriminant_max      = 0;
  [[maybe_unused]] EPrimType discriminant_int_type = EPrimType::u8;

  std::string debug_str() const override
  {
    return "declaration enum \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Enum;
  }

  std::string mangle_type() const override
  {
    return "en_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Enum*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Flag final : public ADeclaration, AType {
  std::vector<std::string> fields;

  std::string debug_str() const override
  {
    return "declaration flag \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Flag;
  }

  std::string mangle_type() const override
  {
    return "fg_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Flag*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

// e.g. mod name {}
struct Mod : public ADeclaration {
  std::vector<std::shared_ptr<ADeclaration>> declarations;

  std::string debug_str() const override
  {
    return "declaration mod \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Module;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Export final : public Mod {
  std::shared_ptr<ModuleExportation> mod_exp_sym;

  std::string debug_str() const override
  {
    return "export";
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Export;
  }
};

struct Extern final : public Mod {

  std::string debug_str() const override
  {
    return "extern \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Extern;
  }
  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Function final : public ADeclaration, ICallable {
  std::shared_ptr<type::Function_Proto> prototype;
  std::unique_ptr<local::CodeBlock>     codeblock;
  bool                                  isDefinition  = false;
  bool                                  isConst       = false;
  bool                                  isPure        = false;
  bool                                  isCompileTime = false;
  bool                                  isExtern      = false;
  std::string                           extern_call_convention;

  std::string debug_str() const override;

  type::Function_Proto* get_signature() override
  {
    return prototype.get();
  };
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Function;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Mod_Alias final : public ADeclaration {
  std::unique_ptr<AIdentifier> module;

  std::string debug_str() const override
  {
    return "mod " + name + " = " + module->debug_str();
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Mod_Alias;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

struct Type_Alias final : public ADeclaration {
  std::shared_ptr<AType> type;

  std::string debug_str() const override
  {
    return "type " + name + " = " + type->debug_str();
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Type_Alias;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

// gen name<T, U,...> { condition }
struct Generic final : public ADeclaration, AType {
  std::vector<std::shared_ptr<AType>>                  gen_args;
  std::set<std::string>                                targetGenericSymbols; // generic typenames
  std::vector<std::shared_ptr<ast::generic::IGenCond>> conditions;           // generic conditions

  std::string debug_str() const override
  {
    return "generic \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Generic;
  }

  std::string mangle_type() const override
  {
    return "gn_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Generic*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }
};

// let/var a: ptr'type#tableSize = expression;
struct Global final : public ADeclaration {
  std::shared_ptr<AType> type;                                       // infered if nullptr
  EAssignmentType        assignment = EAssignmentType::MoveSemantic; // assign type
  std::unique_ptr<Node>  expression;                                 // affectation
  EVariableKind          kind = EVariableKind::Const;

  bool isExtern = false;

  std::string debug_str() const override;

  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Global;
  }

  void accept(Visitor_Base& v) override
  {
    v.visit(*this);
  }

private:
  bool type_already_checked = false;
};

} // namespace declaration
  // Declaration
} // namespace ast
  // AST