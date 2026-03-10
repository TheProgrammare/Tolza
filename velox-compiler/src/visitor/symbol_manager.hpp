
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ast/ast_forward.hpp"
#include "script_info.hpp"

struct ScriptInfo;

enum class EScopeType;
enum class ESymbolType;

struct ScopeData {
  std::string name;
  EScopeType  type;
  size_t      depth = 0;
};

struct Symbol_Data {
  ESymbolType type;

  std::string mangling;
  std::string mangling_convention_name = "Velox";

  std::shared_ptr<ast::ADeclaration> symbol;

  bool is_exported  = false;
  bool is_external  = false;
  bool is_ex_nihilo = false;
};


struct Symbols_Manager {
  Symbols_Manager(ScriptInfo& _scr_info)
    : scr_info(_scr_info)
  {
  }

  ScriptInfo& scr_info;

  std::vector<std::shared_ptr<Symbol_Data>> declarations;
  std::vector<std::shared_ptr<Symbol_Data>> exportations;
  std::vector<ScopeData>                    current_scope_path;
  std::vector<std::string>                  decl_errors;

  bool in_export = false;

  std::string                  get_current_export_name() const;
  std::shared_ptr<Symbol_Data> add_decl(std::shared_ptr<ast::ADeclaration> declaration);
  bool                         is_external_symbol(const ast::AIdentifier& sym) const;
  void                         try_add_extern_sym_to_generate(const ast::AIdentifier& sym, Extern_Item::Kind kind);
  void                         enter_scope(const std::string& name, EScopeType type, size_t depth = 0);
  void                         exit_scope();
  [[nodiscard]] std::vector<std::string> get_current_path() const;
  [[nodiscard]] Symbol_Data* find_local_symbol(const std::span<const std::string>& scope, const std::string& name);
  [[nodiscard]] Symbol_Data* find_exported_symbol(const std::span<const std::string>& scope, const std::string& name);
};
