#include "pipeline/module_resolver.hpp"

#include "ast/definition/ast_base.hpp"
#include "binder/binder_ffi.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "pipeline/pipeline.hpp"
#include "pool/module.hpp"
#include "pool/token.hpp"
#include "pool/unresolved.hpp"

#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;


bool module_resolver::resolve_modules(std::unordered_set<cu::ID, cu::ID::Hash>& CUs) noexcept
{
  std::unordered_set<cu::ID, cu::ID::Hash>   imported_nodes;
  std::unordered_set<ast::ID, ast::ID::Hash> exported_nodes;

  for (auto cuid : CUs) {
    auto& cu = cuid.get();

    imported_nodes.insert(cuid);

    exported_nodes.insert(cu.node_export);
  }

  for (auto cuid : imported_nodes) {
    auto& cu = cuid.get();

    for (const auto& [id, modid] : cu.imports) {
      // if (modid) continue;

      const auto* imp = id.as<ast::Import>();
      assert(imp);

      const auto* regex = imp->regex.as<ast::Path_Regex>();
      assert(regex);

      // binding generation query
      if (regex->source == cu::EFileSource::binding) (void)generate_bind(regex->path, imp->alias);

      // resolve import module source
      auto modid_found = cu::resolve_regex_path(modid, regex->path, regex->source);
      if (!modid_found) {
        std::string out_err;
        modid_found = module::build_module_from_path(cuid, regex->path, regex->source, out_err);
        if (!modid_found) {
          auto& tok = cu.file_info.tokens->get(regex->header.start_tokid);

          auto err = Error_Diagnostic(cuid, 249, tok.begin, tok.begin + tok.length, compiler::EPhase::shipowner,
                                      std::format("Impossible to generate the file at \"{}.{}*\"",
                                                  cu::file_path_to_str(regex->path, regex->source),
                                                  std::string(common::fileutils::TOLZA_FILE_EXTENSION)),
                                      "");
          COMPILER.add_error(err);
        }
      }

      cu.imports[id] = modid_found;
    }
  }

  return true;
}

bool module_resolver::generate_bind(const std::vector<std::string>& path, std::string_view alias) noexcept
{
  // path must specify the language, then the file
  assert(path.size() >= 2);
  fs::create_directories(OPTIONS.get_dir_binding_profile());
  size_t bind_count = 0;

  ffi::Bind_Package bind;
  bind.lang = path[0];
  bind.lib  = path[1];

  std::unordered_set<ast::ID, ast::ID::Hash> resolved_nodes;
  resolved_nodes.reserve(COMPILER.unresolved.nodes.size() / PIPELINE.compilation_units.size());


  for (auto id : COMPILER.unresolved.nodes) {
    const auto* n = id.as<ast::Symbol_Qualified>();
    if (!n) continue;

    if (n->path[0] != alias) continue;

    bind.extern_items.try_emplace(std::string(n->path.back()), id);
    resolved_nodes.insert(id);
  }

  fs::path bind_path = OPTIONS.get_dir_binding_profile();
  for (const auto& i : path) bind_path /= i;
  bind_path.replace_extension(common::fileutils::TOLZA_FILE_EXTENSION);
  fs::create_directories(bind_path.parent_path());

  std::fstream f(bind_path);

  auto cu = pipeline::Pipeline::build_CU_from_path(cu::ID::main(), bind_path.string());

  bind_path = common::fileutils::get_tolza_file(bind_path.string());

  PIPELINE.binding_compilation_units_to_prepare.insert(bind_path);


  return true;
}
