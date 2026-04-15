#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "lexer/token.hpp"
#include "misc/module_manager.hpp"

namespace llvm
{
class Module;
}

namespace ast
{
struct Root;
} // namespace ast


namespace meta
{
struct Manager;
} // namespace meta

namespace symbol
{
struct Manager;
}

namespace module
{
struct Module;
}


namespace script
{

using _Script_Id = size_t;

enum class EFileSource { src, vendor_lib, stdlib, pkg_lib, binding, relative };
EFileSource file_path_to_EFileSource(const std::string& file);
std::string EFileSource_to_dir(EFileSource p_file_source);

struct FileInfo {
  _Script_Id               id;
  EFileSource              source = EFileSource::src;
  std::string              path;
  std::string              data;
  std::vector<std::string> lines;
  std::vector<Token>       tokens;

  [[nodiscard]] std::string get_module_name() const;
  [[nodiscard]] std::string get_module_path() const;
  [[nodiscard]] std::string get_file_name() const;
  [[nodiscard]] std::string get_file_extension() const;

  // start at 1
  [[nodiscard]] std::string get_line(size_t p_line) const;
  [[nodiscard]] size_t      get_line_size() const;
};

struct ScriptInfo {
  ScriptInfo(const std::string& _file_path, const std::string& _file_str, const std::vector<std::string>& _file_lines);
  ~ScriptInfo();

  FileInfo file_info;

  std::shared_ptr<symbol::Manager> sym_m;
  std::shared_ptr<module::Module>  module_root;
  std::shared_ptr<meta::Manager>   meta_m;

  std::shared_ptr<ast::Root> root_node;

  std::vector<std::string> semantic_resolveType_errors;
  std::vector<std::string> semantic_checkType_errors;

  std::unique_ptr<llvm::Module> llvm_module;

  // No copy
  ScriptInfo(const ScriptInfo&)            = delete;
  ScriptInfo& operator=(const ScriptInfo&) = delete;

  // can move
  ScriptInfo(ScriptInfo&&)            = default;
  ScriptInfo& operator=(ScriptInfo&&) = default;

private:
};

} // namespace script