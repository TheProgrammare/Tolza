#include "nexus/script.hpp"

#include <cassert>
#include <cstddef>
#include <filesystem>

#include <fstream>
#include <llvm/IR/Module.h>


#include "nexus/ast/ast.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/metacode/metacode.hpp"
#include "compiler/compiler.hpp"
#include "nexus/lexer/token.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include "common.hpp"
#include "compiler_options.hpp"
#include "nexus/resolved.hpp"
#include "nexus/inference.hpp"

namespace fs = std::filesystem;

script::FileInfo::FileInfo(std::string_view _path, const std::string& _data,
                           const std::vector<size_t>& _last_offset_line)
  : data(_data)
  , last_offset_line(_last_offset_line)
  , path(_path)
{
  assert(fs::exists(path));
}


script::ScriptInfo::~ScriptInfo()
{
  delete metacodes;
  delete nodes;
  delete file_info.tokens;
}


script::ScriptInfo::ScriptInfo(std::string_view _file_path, const std::string& _data,
                               const std::vector<size_t>& _last_offset_line)
  : file_info(_file_path, _data, _last_offset_line)
{
}

std::string script::FileInfo::get_module_name() const
{
  return fs::path(path).stem();
}
std::string script::FileInfo::get_file_name() const
{
  return fs::path(path).filename();
}
std::string script::FileInfo::get_file_extension() const
{
  return fs::path(path).extension();
}
size_t script::FileInfo::get_line_from_pos(size_t pos) const
{
  return std::lower_bound(last_offset_line.begin(), last_offset_line.end(), pos) - last_offset_line.begin();
}
std::string_view script::FileInfo::get_line(size_t p_line) const
{
  assert(p_line < last_offset_line.size());

  size_t line_size = get_line_size(p_line) - 1; // - 1 to avoid the \n
  return std::string_view(data.data() + get_line_start(p_line), line_size);
}
// ligne -> début
size_t script::FileInfo::get_line_start(size_t line) const
{
  if (line == 0) return 0;

  assert(line < last_offset_line.size());

  return last_offset_line[line - 1] + 1;
}

// ligne -> fin (position du \n)
size_t script::FileInfo::get_line_end(size_t line) const
{
  assert(line < last_offset_line.size());

  return last_offset_line[line];
}
size_t script::FileInfo::get_line_size(size_t line) const
{
  size_t start = get_line_start(line);
  size_t end   = get_line_end(line);
  return end - start;
}
size_t script::FileInfo::get_column_from_pos(size_t pos) const
{
  size_t line = get_line_from_pos(pos);

  if (line == 0) return pos;

  return pos - last_offset_line[line - 1] - 1;
}
std::string script::FileInfo::get_module_path() const
{
  fs::path p = path;
  return p.parent_path() / p.stem();
}


script::EFileSource script::file_path_to_EFileSource(std::string_view p_file)
{
  if (fs::path(p_file).filename() == fs::path(compiler::COMPILER_OPTIONS.get_dir_source()).filename())
    return script::EFileSource::src;
  if (fs::path(p_file).filename() == fs::path(compiler::COMPILER_OPTIONS.get_dir_binding()).filename())
    return script::EFileSource::binding;
  if (fs::path(p_file).filename() == fs::path(compiler::COMPILER_OPTIONS.get_dir_vendor()).filename())
    return script::EFileSource::vendor_lib;
  if (p_file == common::get_stdlib_dir()) return script::EFileSource::stdlib;
  if (p_file == common::get_packages_dir()) return script::EFileSource::pkg_lib;
  return script::EFileSource::relative;
}

std::string script::EFileSource_to_dir(EFileSource p_file_source)
{
  switch (p_file_source) {
  case script::EFileSource::src:        return compiler::COMPILER_OPTIONS.get_dir_source();
  case script::EFileSource::vendor_lib: return compiler::COMPILER_OPTIONS.get_dir_vendor();
  case script::EFileSource::stdlib:     return compiler::COMPILER_OPTIONS.get_dir_stdlib();
  case script::EFileSource::pkg_lib:    return compiler::COMPILER_OPTIONS.get_dir_packages();
  case script::EFileSource::binding:    return compiler::COMPILER_OPTIONS.get_dir_binding();
  case script::EFileSource::relative:   return "";
  }
}

std::string_view script::EFileSource_to_str(EFileSource p_file_source)
{
  switch (p_file_source) {
  case EFileSource::src:        return "src";
  case EFileSource::vendor_lib: return "vendor";
  case EFileSource::stdlib:     return "std";
  case EFileSource::pkg_lib:    return "pkg";
  case EFileSource::binding:    return "bind";
  case EFileSource::relative:   return "src";
  }
}

std::string script::file_path_to_str(std::vector<std::string_view> path, EFileSource p_file_source)
{
  auto out = EFileSource_to_dir(p_file_source);
  for (auto& elem : path) {
    out += "/";
    out += elem;
  }

  return out;
}