#pragma once

#include <cassert>
#include <cstddef>
#include <string_view>
#include <unordered_map>
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


namespace cu
{

enum class EFileSource : uint8_t {
  src,        // user scripts
  vendor_lib, // 3rd party scripts
  stdlib,     // standard library
  pkg_lib,    // package library
  binding,    // binding library
  relative,   // relative script
};

[[nodiscard]] EFileSource      file_path_to_EFileSource(std::string_view file);
[[nodiscard]] std::string      EFileSource_to_dir(EFileSource p_file_source);
[[nodiscard]] std::string_view EFileSource_to_str(EFileSource p_file_source);

[[nodiscard]] std::string normalize_velox_script_path(std::string_view path);
[[nodiscard]] std::string file_path_to_str(const std::vector<std::string>& path, EFileSource p_file_source);

struct FileInfo {
  explicit FileInfo() = default;

  explicit FileInfo(cu::ID _cuid, std::string_view path, const std::string& _data,
                    const std::vector<size_t>& _last_offset_line);

  FileInfo(const FileInfo&)            = delete;
  FileInfo& operator=(const FileInfo&) = delete;

  const ID                  cuid;
  const EFileSource         source = EFileSource::src;
  const std::string         path;
  const std::string         data;
  const std::vector<size_t> last_offset_line;
  token::Arena* const       tokens     = nullptr;
  const bool                is_mod_dir = false;
  const bool                is_barrel  = false;

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

struct CU {
  CU();
  ~CU();

  explicit CU(cu::ID _parent_cuid, cu::ID _cuid, std::string_view _file_path, const std::string& _data,
              const std::vector<size_t>& _last_offset_line);

  CU(const CU&)            = delete;
  CU& operator=(const CU&) = delete;

  [[nodiscard]] static CU make_root() noexcept;

  struct Status {
    bool root      = false;
    bool prepared  = false;
    bool analyzed  = false;
    bool codegened = false;
  };

  Status status;


  // base
  const ID       cuid;
  const ID       parent_cuid;
  const FileInfo file_info;

  // local pools
  metacode::Graph* const metacodes = nullptr; // preprocessor pass
  ast::Arena* const      nodes     = nullptr; // parsing pass
  type::Arena* const     types     = nullptr; // parsing pass
  scope::Graph* const    scopes    = nullptr; // parsing pass
  symbol::Arena* const   symbols   = nullptr; // parsing pass
  module::Graph* const   modules   = nullptr; // parsing pass


  // dependencies
  std::unordered_map<ast::ID, module::ID, ast::ID::Hash> imports;
  std::unordered_map<ast::ID, module::ID, ast::ID::Hash> reexports;
  ast::ID                                                node_export;

  // extra
  llvm::Module* llvm_module = nullptr; // codegen pass
};

[[nodiscard]] module::ID resolve_regex_path(module::ID ctx, const std::vector<std::string>& path,
                                            EFileSource src) noexcept;

} // namespace cu