#include "compiler/file_info.hpp"

#include "common/compiler_options.hpp"
#include "common/environment.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/id_query.hpp"
#include "id/modid.hpp"
#include "lexer/pool.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "module/tool.hpp"

#include <algorithm>
#include <cassert>
#include <common/fileutils.hpp>
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

cu::FileInfo::FileInfo(cu::ID _cuid, std::string_view _path, const std::string& _data,
                       const std::vector<size_t>& _last_offset_line)
  : cuid(_cuid)
  , data(_data)
  , line_end_offset(_last_offset_line)
  , source(file_path_to_EFileSource(_path))
  , path(common::fileutils::get_tolza_file(_path))
  , tokens(new token::Arena(_cuid))
  , is_mod_dir(fs::path(_path).stem() == "mod")
{
  if (cuid.is_temp()) return;

  assert(!path.empty());
  assert(fs::exists(path));
}


std::string cu::FileInfo::get_module_name() const
{
  return is_mod_dir ? fs::path(path).parent_path().stem() : fs::path(path).stem();
}
std::string cu::FileInfo::get_file_name() const
{
  return fs::path(path).filename();
}
std::string cu::FileInfo::get_file_extension() const
{
  return fs::path(path).extension();
}
size_t cu::FileInfo::get_line_from_pos(size_t pos) const
{
  auto line = std::ranges::lower_bound(line_end_offset, pos) - line_end_offset.begin();
  if (line >= line_end_offset.size()) return line_end_offset.size() - 1;
  return line;
}
std::string_view cu::FileInfo::get_line(size_t p_line) const
{
  if (p_line >= line_end_offset.size()) {
    if (line_end_offset.size() > 1) {
      return {data.data() + (line_end_offset.back() - 1) + 1, line_end_offset.back()};
    }
    return data;
  }

  size_t line_size = get_line_size(p_line);

  return {data.data() + get_line_start(p_line) + 1, line_size - 1}; // + 1 and - 1 to avoid \n
}
size_t cu::FileInfo::get_line_start(size_t line) const
{
  if (line == 0) return 0;

  assert(line < line_end_offset.size());

  return line_end_offset[line - 1];
}

// line -> end (\n)
size_t cu::FileInfo::get_line_end(size_t line) const
{
  assert(line < line_end_offset.size());

  return line_end_offset[line];
}
size_t cu::FileInfo::get_line_size(size_t line) const
{
  if (line == 0) return get_line_end(line);

  size_t start = get_line_start(line);
  size_t end   = get_line_end(line);
  return end - start;
}
size_t cu::FileInfo::get_column_from_pos(size_t pos) const
{
  size_t line = get_line_from_pos(pos);

  if (line == 0) return pos;

  return pos - line_end_offset[line - 1] - 1;
}
std::string cu::FileInfo::get_module_path() const
{
  fs::path p = path;
  return p.parent_path() / p.stem();
}


cu::EFileSource cu::file_path_to_EFileSource(std::string_view p_file)
{
  if (common::fileutils::is_sub_path(OPTIONS.dir.get_dir_source(), p_file)) return cu::EFileSource::src;
  if (common::fileutils::is_sub_path(OPTIONS.get_dir_binding_profile(), p_file)) return cu::EFileSource::bind;
  if (common::fileutils::is_sub_path(OPTIONS.dir.get_dir_vendor(), p_file)) return cu::EFileSource::vendor;
  if (common::fileutils::is_sub_path(common::env::get_stdlib_dir(), p_file)) return cu::EFileSource::std;
  if (common::fileutils::is_sub_path(common::env::get_packages_dir(), p_file)) return cu::EFileSource::pkg;
  return cu::EFileSource::self;
}


std::string cu::EFileSource_to_dir(EFileSource p_file_source)
{
  switch (p_file_source) {
  case cu::EFileSource::src:    return OPTIONS.dir.get_dir_source();
  case cu::EFileSource::vendor: return OPTIONS.dir.get_dir_vendor();
  case cu::EFileSource::std:    return OPTIONS.dir.get_dir_stdlib();
  case cu::EFileSource::pkg:    return OPTIONS.dir.get_dir_packages();
  case cu::EFileSource::bind:   return OPTIONS.get_dir_binding_profile();
  case cu::EFileSource::self:   return {};
  }
}

std::string_view cu::EFileSource_to_str(EFileSource p_file_source)
{
  switch (p_file_source) {
  case EFileSource::src:    return "src";
  case EFileSource::vendor: return "vendor";
  case EFileSource::std:    return "std";
  case EFileSource::pkg:    return "pkg";
  case EFileSource::bind:   return "bind";
  case EFileSource::self:   return "self";
  }
}


std::string cu::file_path_to_str(const std::vector<std::string>& path, EFileSource p_file_source)
{
  auto out = EFileSource_to_dir(p_file_source);
  for (const auto& elem : path) {
    out += "/";
    out += elem;
  }

  return out;
}


module::ID cu::resolve_regex_path(module::ID ctx, const std::vector<std::string>& path, EFileSource src) noexcept
{
  const auto start_ctx = ctx;

  switch (src) {
  case EFileSource::src:    ctx = module::get_src().modid; break;
  case EFileSource::vendor: ctx = module::get_vendor().modid; break;
  case EFileSource::std:    ctx = module::get_std().modid; break;
  case EFileSource::pkg:    ctx = module::get_pkg().modid; break;
  case EFileSource::bind:   ctx = module::get_bind().modid; break;
  case EFileSource::self:   break;
  }


  // path resolution
  size_t path_count = 0;

  while (true) {
    bool matched = false;

    for (auto child_modid : module::get_children(ctx)) {
      const auto& child_mod = child_modid.get();

      if (child_mod.name == path[path_count]) {
        ctx = child_mod.modid;
        path_count++;
        matched = true;
        break;
      }
    }

    // return the current module
    // if not matched, it's can be a scope symbol
    if (!matched) return NO_ID;
    if (path_count >= path.size()) return ctx;
  }

  return NO_ID;
}