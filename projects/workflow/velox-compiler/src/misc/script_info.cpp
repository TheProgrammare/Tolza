#include "misc/script_info.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>

#include <llvm/IR/Module.h>

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"

#include "compiler/compiler.hpp"

#include <compiler_context.hpp>
#include "common.hpp"

namespace fs = std::filesystem;


ScriptInfo::ScriptInfo(const std::string& _file_path, const std::string& _file_str,
                       const std::vector<std::string>& _file_lines)
  : file_info(FileInfo{.path = _file_path, .data = _file_str, .lines = _file_lines})
{
}

std::string FileInfo::get_module_name() const
{
  return fs::path(path).stem();
}
std::string FileInfo::get_file_name() const
{
  return fs::path(path).filename();
}
std::string FileInfo::get_file_extension() const
{
  return fs::path(path).extension();
}
std::string FileInfo::get_line(size_t p_line) const
{
  if (p_line - 1 > lines.size() - 1) return lines.back();
  if (p_line - 1 < 0) return lines[0];
  return lines[p_line - 1];
}
size_t FileInfo::get_line_size() const
{
  return lines.size();
}
std::string FileInfo::get_module_path() const
{
  fs::path p = path;
  return p.parent_path() / p.stem();
}

ScriptInfo::~ScriptInfo()
{
}

EFileSource file_path_to_EFileSource(const std::string& p_file)
{
  if (fs::path(p_file).filename() == fs::path(compiler::COMP_CTX.get_dir_source()).filename()) return EFileSource::src;
  if (fs::path(p_file).filename() == fs::path(compiler::COMP_CTX.get_dir_binding()).filename())
    return EFileSource::binding;
  if (fs::path(p_file).filename() == fs::path(compiler::COMP_CTX.get_dir_vendor()).filename())
    return EFileSource::vendor_lib;
  if (p_file == common::get_stdlib_dir()) return EFileSource::stdlib;
  if (p_file == common::get_packages_dir()) return EFileSource::pkg_lib;
  return EFileSource::relative;
}

std::string EFileSource_to_dir(EFileSource p_file_source)
{
  switch (p_file_source) {
  case EFileSource::src:        return compiler::COMP_CTX.get_dir_source();
  case EFileSource::vendor_lib: return compiler::COMP_CTX.get_dir_vendor();
  case EFileSource::stdlib:     return compiler::COMP_CTX.get_dir_stdlib();
  case EFileSource::pkg_lib:    return compiler::COMP_CTX.get_dir_packages();
  case EFileSource::binding:    return compiler::COMP_CTX.get_dir_binding();
  case EFileSource::relative:   return "";
  }
}
