#pragma once

#include <memory>
#include <vector>
#include <set>

#include "lexer/token.hpp"

namespace ast
{
struct Root;
struct AExpression;
} // namespace ast

struct ScriptInfo;

enum class EExtern_Kind {
  Function,
  Type,
  Global,
  Enum,
  Union,
  Component,
  System,
  Entity,
  Generic,
  Metacode,
};

struct Extern_Item {

  EExtern_Kind             kind;
  std::string              name;
  std::vector<std::string> scope;

  Extern_Item(const std::string& _name, const std::vector<std::string>& _scope, EExtern_Kind _kind)
    : name(_name)
    , scope(_scope)
    , kind(_kind)
  {
  }
};

EExtern_Kind AST_AExpression_to_Extern_Item_Kind(const ast::AExpression& n);

struct Symbols_Manager;

namespace meta
{
struct MetablockManager;
} // namespace meta

struct ModuleImportation {
  // project = default or ~
  // standard lib = std:
  // user lib = lib:
  // extern = ext:
  // binding = ext:
  enum class EImportSource { Unknown, User, StandardLib, Package, Extern, Binding };

  std::string              name;
  std::vector<std::string> path;

  // if from another language or lib
  [[maybe_unused]]
  std::string extern_lib;

  EImportSource import_source = EImportSource::User;

  std::vector<std::shared_ptr<ScriptInfo>> target_modules;

  std::vector<Extern_Item> extern_references;

  std::string get_path() const;
  std::string get_normalized_path() const;

  bool is_external() const
  {
    return !extern_lib.empty();
  }

  void add_extern_reference(const Extern_Item& ext_item)
  {
    extern_references.push_back(ext_item);
  }

  std::string debug_name() const
  {
    std::string out;
    for (auto& elem : path) out += elem + "::";
    return out + name;
  }
};

struct ModuleExportation {
  // export to external (for other language for conversion)
  [[maybe_unused]]
  std::string extern_lib;

  std::shared_ptr<ScriptInfo> script_exported = nullptr;

  [[maybe_unused]]
  std::shared_ptr<ScriptInfo> script_mirror = nullptr;

  bool is_mirror = false;

  [[maybe_unused]]
  ModuleImportation* mirror = nullptr;

  bool is_external() const
  {
    return !extern_lib.empty();
  }
};

struct ScriptInfo {
  ScriptInfo(const std::string& _file_path, const std::string& _file_str, const std::vector<std::string>& _file_lines)
    : file_path(_file_path)
    , file_str(_file_str)
    , file_lines(_file_lines)
  {
  }

  enum class Origin { user, vendor_lib, stdlib, lib, binding };

  Origin                  origin = Origin::user;
  std::string             file_path;
  std::string             file_str;
  std::vector<Token>      tokens;
  meta::MetablockManager* m_meta = nullptr;

  // names of modules exported
  std::vector<std::shared_ptr<ModuleExportation>> exported_mod;
  // names of modules imported
  std::vector<std::shared_ptr<ModuleImportation>> imported_mod;

  ast::Root* rootNode = nullptr;

  std::vector<std::string> semantic_resolveType_errors;
  std::vector<std::string> semantic_checkType_errors;

  Symbols_Manager* m_sym = nullptr;

  void add_export(const ModuleExportation& exp);
  void add_import(const ModuleImportation& imp);

  [[nodiscard]] std::shared_ptr<ModuleExportation> get_export_module(const std::string& path);
  [[nodiscard]] std::shared_ptr<ModuleImportation> get_import_module(std::span<const std::string> path);

  [[nodiscard]] std::set<ModuleImportation*> get_externs();

  [[nodiscard]] std::set<std::string> get_extern_languages();

  [[nodiscard]] std::string get_normalized_path() const;

  // start at 1
  [[nodiscard]] std::string get_line(size_t line) const
  {
    if (line - 1 > file_lines.size() - 1) return file_lines.back();
    if (line - 1 < 0) return file_lines[0];
    return file_lines[line - 1];
  }

  [[nodiscard]] size_t get_line_size() const
  {
    return file_lines.size();
  }

  // No copy
  ScriptInfo(const ScriptInfo&)            = delete;
  ScriptInfo& operator=(const ScriptInfo&) = delete;

  // can move
  ScriptInfo(ScriptInfo&&)            = default;
  ScriptInfo& operator=(ScriptInfo&&) = default;

private:
  std::vector<std::string> file_lines;
};
