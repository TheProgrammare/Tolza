
#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "AST/AST_Base.hpp"

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

  std::shared_ptr<AST::ADeclaration> symbol;

  bool is_exported  = false;
  bool is_external  = false;
  bool is_ex_nihilo = false;
};

struct Symbols_Manager {
  Symbols_Manager(ScriptInfo &_scr_info) : scr_info(_scr_info) {}

  ScriptInfo &scr_info;

  std::vector<std::shared_ptr<Symbol_Data>> declarations;
  std::vector<ScopeData>                    current_scope_path;
  std::vector<std::string>                  decl_errors;

  std::string                            get_current_export_name() const;
  std::shared_ptr<Symbol_Data>           add_decl(std::shared_ptr<AST::ADeclaration> declaration);
  std::shared_ptr<Symbol_Data>           add_decl_ex_nihilo(std::shared_ptr<AST::ADeclaration> declaration);
  void                                   enter_scope(const std::string &name, EScopeType type, size_t depth = 0);
  void                                   exit_scope();
  [[nodiscard]] std::vector<std::string> get_current_path() const;
  [[nodiscard]] std::optional<std::shared_ptr<AST::ADeclaration>> find_symbol(const std::string &full_name);
};
