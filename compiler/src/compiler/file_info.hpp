#pragma once

#include "compiler/compilation_unit.hpp"
#include "id/cuid.hpp"
#include "nexus/forward.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>


namespace cu
{

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
  const std::vector<size_t> line_end_offset;
  token::Arena* const       tokens     = nullptr;
  const bool                is_mod_dir = false;
  const bool                is_barrel  = false;
  const bool                is_dirty   = false;

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


[[nodiscard]] EFileSource      file_path_to_EFileSource(std::string_view file);
[[nodiscard]] std::string      EFileSource_to_dir(EFileSource p_file_source);
[[nodiscard]] std::string_view EFileSource_to_str(EFileSource p_file_source);

[[nodiscard]] std::string normalize_tolza_script_path(std::string_view path);
[[nodiscard]] std::string file_path_to_str(const std::vector<std::string>& path, EFileSource p_file_source);

[[nodiscard]] module::ID resolve_regex_path(module::ID ctx, const std::vector<std::string>& path,
                                            EFileSource src) noexcept;

} // namespace cu