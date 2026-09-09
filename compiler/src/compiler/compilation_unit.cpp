#include "compiler/compilation_unit.hpp"

#include "ast/data.hpp"
#include "ast/forward.hpp"
#include "ast/node/base.hpp"
#include "ast/pool.hpp"
#include "common/environment.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "lexer/pool.hpp"
#include "metacode/pool.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "pool/node_to_def.hpp"
#include "pool/node_to_ext.hpp"
#include "scope/pool.hpp"
#include "scope/scope.hpp"
#include "type/pool.hpp"

#include <algorithm>
#include <cassert>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <cstddef>
#include <filesystem>
#include <format>
#include <llvm/IR/Module.h>
#include <string>
#include <string_view>
#include <vector>


namespace fs = std::filesystem;


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
  , file_info(*new FileInfo())
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
  : file_info(*new FileInfo(_cuid, _file_path, _data, _last_offset_line))
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
  , file_info(*new FileInfo())
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
