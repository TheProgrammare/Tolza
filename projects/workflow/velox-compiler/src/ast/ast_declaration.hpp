#pragma once


#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast/ast_data.hpp"
#include "ast/ast_forward.hpp"
#include "ast_base.hpp"

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

  llvm::Type* codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override;

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Enum_Element*>(&other)) {
      return parent_enum == ptr->parent_enum && name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override;
};

struct Enum final : public ADeclaration, AType {
  std::vector<std::unique_ptr<Enum_Element>> variants;

  bool                       isGlobal              = true;
  size_t                     discriminant_max      = 0;
  [[maybe_unused]] EPrimType discriminant_int_type = EPrimType::u8;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

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

  void accept(Visitor_Base& v) override;
};

struct Flag final : public ADeclaration, AType {
  std::vector<std::string> fields;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

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

  void accept(Visitor_Base& v) override;
};

struct Union final : public ADeclaration, AType {
  // field name, field type
  std::vector<std::pair<std::string, std::shared_ptr<ast::AType>>> fields;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "declaration union \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Union;
  }

  std::string mangle_type() const override
  {
    return "uo_" + mangle_id(name);
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Union*>(&other)) {
      return name == ptr->name;
    }
    return false;
  }

  void accept(Visitor_Base& v) override;
};

// e.g. mod name {}
struct Mod : public ADeclaration {
  std::vector<std::shared_ptr<ADeclaration>> declarations;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "declaration mod \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Module;
  }

  void accept(Visitor_Base& v) override;
};

struct Export final : public Mod {
  std::shared_ptr<ModuleExportation> mod_exp_sym;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "export";
  }
  void accept(Visitor_Base& v) override;

  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Export;
  }
};

struct Extern final : public Mod {

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "extern \"" + name + "\"";
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Extern;
  }
  void accept(Visitor_Base& v) override;
};

struct Function final : public ACallable, ADeclaration {
  ~Function();

  llvm::Value*    codegen_pass(Visitor_Codegen& v) override;
  llvm::Function* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override;

  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Function;
  }

  void accept(Visitor_Base& v) override;
};

struct Mod_Alias final : public ADeclaration {
  std::unique_ptr<AIdentifier> module;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  std::string  debug_str() const override
  {
    return "mod " + name + " = " + module->debug_str();
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Mod_Alias;
  }

  void accept(Visitor_Base& v) override;
};

struct Type_Alias final : public ADeclaration, AType {
  std::shared_ptr<AType> type;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

  std::string debug_str() const override
  {
    return "type " + name + " = " + type->debug_str();
  }
  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Type_Alias;
  }
  std::string mangle_type() const override
  {
    return type->mangle_type();
  }
  bool compare_with(const AType& other) const override
  {
    return type->is_same(other);
  }

  void accept(Visitor_Base& v) override;
};

// gen name<T, U,...> { condition }
struct Generic final : public ADeclaration, AType {
  std::vector<std::shared_ptr<AType>>                  gen_args;
  std::set<std::string>                                targetGenericSymbols; // generic typenames
  std::vector<std::shared_ptr<ast::generic::IGenCond>> conditions;           // generic conditions

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Type*  codegen_ty(Visitor_Codegen& v) override;

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

  void accept(Visitor_Base& v) override;
};

// let/var a: ptr'type#tableSize = expression;
struct Global final : public ADeclaration, Trait_LLVM_Value {
  std::shared_ptr<AType>       type;                                      // infered if nullptr
  ETransfertType               assignment = ETransfertType::MoveSemantic; // assign type
  std::unique_ptr<AExpression> expression;                                // affectation
  EVariableKind                kind = EVariableKind::Const;

  llvm::Value* codegen_pass(Visitor_Codegen& v) override;
  llvm::Value* codegen(Visitor_Codegen& v) override;

  std::string debug_str() const override;

  ESymbolType get_symbol_type() const override
  {
    return ESymbolType::Global;
  }

  void accept(Visitor_Base& v) override;


private:
  bool type_already_checked = false;
};

} // namespace declaration
  // Declaration
} // namespace ast
  // AST