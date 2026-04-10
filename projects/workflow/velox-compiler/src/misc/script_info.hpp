#pragma once

#include <memory>
#include <vector>
#include <set>

#include "lexer/token.hpp"

namespace llvm
{
class Module;
}

namespace ast
{
struct Root;
struct AExpression;
struct AIdentifier;
} // namespace ast

struct ScriptInfo;

enum class EExtern_Kind {
  Function,
  Type,
  Global,
  Enum,
  Union,
  Flag,
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
  const ast::AIdentifier*  id_node;

  Extern_Item(const std::string& _name, const std::vector<std::string>& _scope, EExtern_Kind _kind,
              const ast::AIdentifier& _id_node);
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
  // user = usr:
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

  std::vector<Extern_Item> extern_fn;
  std::vector<Extern_Item> extern_ty;
  std::vector<Extern_Item> extern_glo;
  std::vector<Extern_Item> extern_enum;
  std::vector<Extern_Item> extern_union;
  std::vector<Extern_Item> extern_flag;
  std::vector<Extern_Item> extern_comp;
  std::vector<Extern_Item> extern_sys;
  std::vector<Extern_Item> extern_entity;
  std::vector<Extern_Item> extern_gen;
  std::vector<Extern_Item> extern_metacode;

  std::string get_path() const;
  std::string get_normalized_path() const;

  bool is_external() const
  {
    return !extern_lib.empty();
  }

  void add_extern_reference(const Extern_Item& ext_item)
  {
    switch (ext_item.kind) {
    case EExtern_Kind::Function:  extern_fn.push_back(ext_item); return;
    case EExtern_Kind::Type:      extern_ty.push_back(ext_item); return;
    case EExtern_Kind::Global:    extern_glo.push_back(ext_item); return;
    case EExtern_Kind::Enum:      extern_enum.push_back(ext_item); return;
    case EExtern_Kind::Union:     extern_union.push_back(ext_item); return;
    case EExtern_Kind::Flag:      extern_flag.push_back(ext_item); return;
    case EExtern_Kind::Component: extern_comp.push_back(ext_item); return;
    case EExtern_Kind::System:    extern_sys.push_back(ext_item); return;
    case EExtern_Kind::Entity:    extern_entity.push_back(ext_item); return;
    case EExtern_Kind::Generic:   extern_gen.push_back(ext_item); return;
    case EExtern_Kind::Metacode:  extern_metacode.push_back(ext_item); return;
    }
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
  ScriptInfo(const std::string& _file_path, const std::string& _file_str, const std::vector<std::string>& _file_lines);
  ~ScriptInfo();

  enum class Origin { src, vendor_lib, stdlib, pkg_lib, binding };
  Origin Origin_from_file(const std::string& file);

  Origin                  origin = Origin::src;
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

  std::unique_ptr<llvm::Module> llvm_module;

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
