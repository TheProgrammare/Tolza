#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace llvm
{
class Module;
}


namespace meta
{
struct Manager;
} // namespace meta


namespace script
{

enum class EFileSource { src, vendor_lib, stdlib, pkg_lib, binding, relative };

EFileSource      file_path_to_EFileSource(std::string_view file);
std::string      EFileSource_to_dir(EFileSource p_file_source);
std::string_view EFileSource_to_str(EFileSource p_file_source);

std::string file_path_to_str(std::vector<std::string_view> path, EFileSource p_file_source);

struct FileInfo {
  FileInfo(std::string_view path, const std::string& _data, const std::vector<size_t>& _last_offset_line);

  FileInfo(const FileInfo&)            = delete;
  FileInfo& operator=(const FileInfo&) = delete;

  FileInfo(FileInfo&&)            = default;
  FileInfo& operator=(FileInfo&&) = default;

  EFileSource         source = EFileSource::src;
  std::string         path;
  const std::string   data;
  std::vector<size_t> last_offset_line;
  // token::FileArena*   file_tokens = nullptr;
  token::Arena*       tokens = nullptr;

  [[nodiscard]] std::string get_module_name() const;
  [[nodiscard]] std::string get_module_path() const;
  [[nodiscard]] std::string get_file_name() const;
  [[nodiscard]] std::string get_file_extension() const;

  [[nodiscard]] size_t           get_line_from_pos(size_t pos) const;
  [[nodiscard]] size_t           get_column_from_pos(size_t pos) const;
  // start at 0
  [[nodiscard]] std::string_view get_line(size_t line) const;
  // start at 0
  [[nodiscard]] size_t           get_line_end(size_t line) const;
  // start at 0
  [[nodiscard]] size_t           get_line_start(size_t line) const;
  // start at 0
  [[nodiscard]] size_t           get_line_size(size_t line) const;
};

struct ScriptInfo {
  ScriptInfo() = delete;
  ~ScriptInfo();

  ScriptInfo(std::string_view _file_path, const std::string& _data, const std::vector<size_t>& _last_offset_line);

  // base
  _id      id;
  FileInfo file_info;

  // pooling shortcuts
  metacode::_id root_metacode_id; // preprocessor pass
  module::_id   module_id;        // parsing pass
  ast::_gnid    root_node_id;     // parsing pass

  // local pools
  metacode::ScriptGraph* metacodes = nullptr; // preprocessor pass
  ast::ScriptArena*      nodes     = nullptr; // parsing pass


  // script dependencies
  std::unordered_map<ast::_gnid, module::_id, ast::_gnid_hash> imports;
  ast::_gnid                                                   node_export;

  // extra
  llvm::Module* llvm_module = nullptr; // codegen pass

  ScriptInfo(ScriptInfo&&) noexcept            = default;
  ScriptInfo& operator=(ScriptInfo&&) noexcept = default;

  ScriptInfo(const ScriptInfo&)            = delete;
  ScriptInfo& operator=(const ScriptInfo&) = delete;
};

} // namespace script