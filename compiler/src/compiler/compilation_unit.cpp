#include "compiler/compilation_unit.hpp"

#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/definition.hpp"
#include "nexus/extension.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/metacode/metacode.hpp"
#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "nexus/type/type.hpp"

#include <cassert>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <cstddef>
#include <filesystem>
#include <llvm/IR/Module.h>
#include <string>
#include <string_view>


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


cu::CU::~CU()
{
  delete metacodes;
  delete ast;
  delete file_info.tokens;
}

cu::CU::CU()
  : cuid(cu::ID::main())
  , metacodes(new metacode::Graph())
  , ast(new ast::Arena(cuid))
  , types(new type::Arena(cuid))
  , scopes(new scope::Graph(cuid))
  , definitions(new definition::Arena(cuid))
  , modules(new module::Graph(cu::ID::invalid(), cuid))
  , extensions(new extension::Arena())
{
  auto&       root_mod  = modules->get_file_root();
  auto&       root_scp  = scopes->get_file_root();
  const auto* root_node = ast->get_file_root();
  assert(root_node);

  // init scope of file module
  root_mod.scpid      = root_scp.scpid;
  root_mod.nodeid     = root_node->nodeid();
  root_mod.debug_name = "root compilation unit";
  root_scp.modid      = root_mod.modid;
  root_scp.nodeid     = root_node->nodeid();
  root_scp.debug_name = "root compilation unit";

  status.root = true;
}

cu::CU::CU(cu::ID _parent_cuid, cu::ID _cuid, std::string_view _file_path, const std::string& _data,
           const std::vector<size_t>& _last_offset_line)
  : file_info(_cuid, _file_path, _data, _last_offset_line)
  , parent_cuid(_parent_cuid)
  , cuid(_cuid)
  , metacodes(new metacode::Graph())
  , ast(new ast::Arena(cuid))
  , types(new type::Arena(cuid))
  , scopes(new scope::Graph(cuid))
  , definitions(new definition::Arena(cuid))
  , modules(new module::Graph(_parent_cuid, cuid))
  , extensions(new extension::Arena())
{
  auto&       root_mod  = modules->get_file_root();
  auto&       root_scp  = scopes->get_file_root();
  const auto* root_node = ast->get_file_root();

  // init scope of file module
  root_mod.scpid      = root_scp.scpid;
  root_mod.nodeid     = root_node->nodeid();
  root_mod.debug_name = std::format("root \"{}\"", file_info.get_file_name());
  root_scp.modid      = root_mod.modid;
  root_scp.nodeid     = root_node->nodeid();
  root_scp.debug_name = std::format("root \"{}\"", file_info.get_file_name());
}

cu::CU::CU(cu::ID _cuid)
  : cuid(_cuid)
  , metacodes(new metacode::Graph())
  , ast(new ast::Arena(cuid))
  , types(new type::Arena(cuid))
  , scopes(new scope::Graph(cuid))
  , definitions(new definition::Arena(cuid))
  , modules(new module::Graph(NO_ID, cuid))
  , extensions(new extension::Arena())
{
  auto&       root_mod  = modules->get_file_root();
  const auto* root_node = ast->get_file_root();

  // init scope of file module
  root_mod.nodeid     = root_node->nodeid();
  root_mod.debug_name = std::format("root \"{}\"", file_info.get_file_name());
}


cu::TEMP_CU::TEMP_CU(cu::ID _cuid, metacode::Graph* _metacode, ast::Arena* _ast, type::Arena* _type,
                     scope::Graph* _scope, definition::Arena* _def, module::Graph* _module, extension::Arena* _ext)
  : CU(_cuid)
{
  metacodes   = _metacode;
  ast         = _ast;
  types       = _type;
  scopes      = _scope;
  definitions = _def;
  modules     = _module;
  extensions  = _ext;
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
  return std::ranges::lower_bound(line_end_offset, pos) - line_end_offset.begin();
}
std::string_view cu::FileInfo::get_line(size_t p_line) const
{
  assert(p_line < line_end_offset.size());

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
  if (common::fileutils::is_sub_path(compiler::OPTIONS.dir.get_dir_source(), p_file)) return cu::EFileSource::src;
  if (common::fileutils::is_sub_path(compiler::OPTIONS.get_dir_binding_profile(), p_file))
    return cu::EFileSource::binding;
  if (common::fileutils::is_sub_path(compiler::OPTIONS.dir.get_dir_vendor(), p_file))
    return cu::EFileSource::vendor_lib;
  if (common::fileutils::is_sub_path(common::env::get_stdlib_dir(), p_file)) return cu::EFileSource::stdlib;
  if (common::fileutils::is_sub_path(common::env::get_packages_dir(), p_file)) return cu::EFileSource::pkg_lib;
  return cu::EFileSource::relative;
}


std::string cu::EFileSource_to_dir(EFileSource p_file_source)
{
  switch (p_file_source) {
  case cu::EFileSource::src:        return compiler::OPTIONS.dir.get_dir_source();
  case cu::EFileSource::vendor_lib: return compiler::OPTIONS.dir.get_dir_vendor();
  case cu::EFileSource::stdlib:     return compiler::OPTIONS.dir.get_dir_stdlib();
  case cu::EFileSource::pkg_lib:    return compiler::OPTIONS.dir.get_dir_packages();
  case cu::EFileSource::binding:    return compiler::OPTIONS.get_dir_binding_profile();
  case cu::EFileSource::relative:   return {};
  }
}

std::string_view cu::EFileSource_to_str(EFileSource p_file_source)
{
  switch (p_file_source) {
  case EFileSource::src:        return "src";
  case EFileSource::vendor_lib: return "vendor";
  case EFileSource::stdlib:     return "std";
  case EFileSource::pkg_lib:    return "pkg";
  case EFileSource::binding:    return "bind";
  case EFileSource::relative:   return "self";
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
  case EFileSource::src:        ctx = module::get_src().modid; break;
  case EFileSource::vendor_lib: ctx = module::get_vendor().modid; break;
  case EFileSource::stdlib:     ctx = module::get_std().modid; break;
  case EFileSource::pkg_lib:    ctx = module::get_pkg().modid; break;
  case EFileSource::binding:    ctx = module::get_bind().modid; break;
  case EFileSource::relative:   break;
  }


  // path resolution
  size_t path_count = 0;

  while (true) {
    bool matched = false;

    for (auto child_modid : ctx.children()) {
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