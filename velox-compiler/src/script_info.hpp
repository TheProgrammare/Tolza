#pragma once

#include <memory>
#include <vector>
#include <filesystem>
#include <set>

#include "ast/ast_forward.hpp"
#include "lexer/token.hpp"

namespace ast
{
struct Root;
struct AExpression;
} // namespace ast

namespace fs = std::filesystem;

struct ScriptInfo;

struct Extern_Item {
  enum class Kind {
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
  Kind                     kind;
  std::string              name;
  std::vector<std::string> scope;

  Extern_Item(const std::string& _name, const std::vector<std::string>& _scope, Kind _kind)
    : name(_name)
    , scope(_scope)
    , kind(_kind)
  {
  }
};

Extern_Item::Kind AST_AExpression_to_Extern_Item_Kind(const ast::AExpression& n);

struct Symbols_Manager;

namespace META
{
struct MetablockManager;
} // namespace META

struct ModuleImportation {
  // project = default or ~
  // standard lib = $
  // user lib = @
  enum class EImportSource { Unknown, User, StandardLib, UserLib, Extern, Binding };

  std::string              name;
  std::vector<std::string> path;

  // if from another language or lib
  [[maybe_unused]]
  std::string extern_lib;

  EImportSource import_source = EImportSource::User;

  std::vector<ScriptInfo*> target_modules;

  std::vector<Extern_Item> extern_references;

  fs::path get_path() const;

  bool is_external() const
  {
    return !extern_lib.empty();
  }

  void add_extern_reference(const Extern_Item& ext_item)
  {
    extern_references.push_back(ext_item);
  }
};

struct ModuleExportation {
  // export to external (for other language for conversion)
  [[maybe_unused]]
  std::string extern_lib;

  ScriptInfo* script_exported = nullptr;

  [[maybe_unused]]
  ScriptInfo* script_mirror = nullptr;

  bool is_mirror = false;

  [[maybe_unused]]
  ModuleImportation* mirror = nullptr;

  bool is_external() const
  {
    return !extern_lib.empty();
  }
};

struct ScriptInfo {
  ScriptInfo(const fs::path& _file_path, const std::string& _file_str, const std::vector<std::string>& _file_lines)
    : file_path(_file_path)
    , file_str(_file_str)
    , file_lines(_file_lines)
  {
  }

  enum class Origin { user, vendor_lib, stdlib, lib, binding };

  Origin                  origin = Origin::user;
  fs::path                file_path;
  std::string             file_str;
  std::vector<Token>      tokens;
  META::MetablockManager* m_meta = nullptr;

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

  [[nodiscard]] ModuleExportation* get_export_module(const fs::path& path);
  [[nodiscard]] ModuleImportation* get_import_module(const fs::path& path);

  [[nodiscard]] std::set<ModuleImportation*> get_externs();

  [[nodiscard]] std::set<fs::path> get_extern_languages();

  // start at 1
  [[nodiscard]] std::string get_line(size_t line) const
  {
    if (line - 1 > file_lines.size() || line - 1 < 0) return "LINE OVER MAX SIZE";
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
