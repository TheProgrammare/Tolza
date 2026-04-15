#pragma once


#include <memory>
#include <set>
#include <string>
#include <vector>

#include "ast/ast_data.hpp"
#include "ast/ast_forward.hpp"
#include "ast_base.hpp"

enum class EFileSource;

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

  CODEGEN_TY

  std::string debug_str() const override;

  std::string mangle_type() const override;
  bool        compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Enum_Element*>(&other)) {
      return parent_enum == ptr->parent_enum && name == ptr->name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

struct Enum final : public ADeclaration, AType {
  std::vector<std::unique_ptr<Enum_Element>> variants;

  bool                       is_global             = true;
  size_t                     discriminant_max      = 0;
  [[maybe_unused]] EPrimType discriminant_int_type = EPrimType::u8;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "declaration enum \"" + declaration_name + "\"";
  }

  std::string mangle_type() const override
  {
    return "en." + declaration_name;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Enum*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

struct Flag final : public ADeclaration, AType {
  std::vector<std::string> fields;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "declaration flag \"" + declaration_name + "\"";
  }

  std::string mangle_type() const override
  {
    return "fg." + declaration_name;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Flag*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

struct Union final : public ADeclaration, AType {
  // field name, field type
  std::vector<std::pair<std::string, std::shared_ptr<ast::AType>>> fields;

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "declaration union \"" + declaration_name + "\"";
  }

  std::string mangle_type() const override
  {
    return "uo." + declaration_name;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Union*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

// e.g. mod name {}
struct Mod : public ADeclaration {
  std::vector<std::shared_ptr<ADeclaration>> declarations;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "declaration mod \"" + declaration_name + "\"";
  }

  VISTOR_ACCEPT
};

struct Import final : public ADeclaration {
  std::vector<std::string> path;
  EFileSource              file_source;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "import";
  }
  VISTOR_ACCEPT
};

struct Export final : public Mod {
  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "export";
  }
  VISTOR_ACCEPT
};

struct ReExport final : public ADeclaration {
  std::vector<std::string> path;
  EFileSource              file_source;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "re export";
  }
  VISTOR_ACCEPT
};

struct Extern final : public Mod {
  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "extern \"" + declaration_name + "\"";
  }
  VISTOR_ACCEPT
};

struct Function final : public ACallable, ADeclaration {
  ~Function();

  CODEGEN_PASS
  CODEGEN_CALL

  std::string debug_str() const override;
  VISTOR_ACCEPT
};

struct Mod_Alias final : public ADeclaration {
  std::unique_ptr<AIdentifier> module;

  CODEGEN_PASS
  std::string debug_str() const override
  {
    return "mod " + declaration_name + " = " + module->debug_str();
  }
  VISTOR_ACCEPT
};

struct Type_Alias final : public ADeclaration {
  std::shared_ptr<AType> type;

  CODEGEN_PASS

  std::string debug_str() const override
  {
    return "type " + declaration_name + " = " + type->debug_str();
  }
  VISTOR_ACCEPT
};

// gen name<T, U,...> { condition }
struct Generic final : public ADeclaration, AType {
  std::vector<std::shared_ptr<AType>>                  gen_args;
  std::set<std::string>                                target_gen_sym; // generic typenames
  std::vector<std::shared_ptr<ast::generic::IGenCond>> conditions;     // generic conditions

  CODEGEN_PASS
  CODEGEN_TY

  std::string debug_str() const override
  {
    return "generic \"" + declaration_name + "\"";
  }
  std::string mangle_type() const override
  {
    return "gn." + declaration_name;
  }
  bool compare_with(const AType& other) const override
  {
    if (auto ptr = dynamic_cast<const Generic*>(&other)) {
      return declaration_name == ptr->declaration_name;
    }
    return false;
  }

  VISTOR_ACCEPT
};

// let/var a: ptr'type#tableSize = expression;
struct Global final : public ADeclaration, Trait_LLVM_Value {
  std::shared_ptr<AType>       type;                                      // infered if nullptr
  ETransfertType               assignment = ETransfertType::MoveSemantic; // assign type
  std::unique_ptr<AExpression> expression;                                // affectation
  EVariableKind                kind = EVariableKind::Const;

  CODEGEN_PASS
  CODEGEN_VALUE

  std::string debug_str() const override;
  VISTOR_ACCEPT

private:
  bool type_already_checked = false;
};

} // namespace declaration
  // Declaration
} // namespace ast
  // AST