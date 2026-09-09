#include "module/pool.hpp"

#include "ast/node/base.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/defid.hpp"
#include "id/modid.hpp"
#include "module/module.hpp"
#include "nexus/forward.hpp"
#include "pipeline/pipeline.hpp"
#include "scope/pool.hpp"
#include "scope/scope.hpp"

#include <cassert>
#include <common/fileutils.hpp>
#include <cstddef>
#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace fs = std::filesystem;

module::Graph::Graph(cu::ID _parent_cuid, cu::ID _cuid) :cuid(_cuid)
{
  if (_parent_cuid) {
    auto& parent_mod = _parent_cuid.get().modules->get_file_root();
    auto  mod        = Module::from_script();
    (void)add(parent_mod.modid, mod);
  } else {
    auto mod = Module::from_script();
    (void)add(module::ID::invalid(), mod);
  }
}

[[nodiscard]] module::ID module::Graph::add(ID p_parent, Module& mod_data) noexcept
{
  assert(!freeze && "Pool is immutable after parsing pass");

  const auto new_id = ID::make(cuid, storage.size());

  auto* mod  = storage.create_get<Module>(mod_data);
  mod->modid = new_id;

  if (p_parent) {
    parent[new_id] = p_parent;
    children[p_parent].emplace_back(new_id);
  }

  return new_id;
}


void module::initialization()
{
  const auto& cu = cu::ID::main().get();

  auto root   = Module::from_system("root", "root");
  auto scr    = Module::from_system("scr", "source");
  auto std    = Module::from_system("std", "standard");
  auto pkg    = Module::from_system("pkg", "package");
  auto bind   = Module::from_system("bind", "binding");
  auto vendor = Module::from_system("vendor", "vendor third party");

  (void)cu.modules->add(ID::invalid(), root);
  (void)cu.modules->add(ID::make(cu.cuid, 0), scr);
  (void)cu.modules->add(ID::make(cu.cuid, 0), std);
  (void)cu.modules->add(ID::make(cu.cuid, 0), pkg);
  (void)cu.modules->add(ID::make(cu.cuid, 0), bind);
  (void)cu.modules->add(ID::make(cu.cuid, 0), vendor);
}

module::Module* module::Graph::get_by_name(std::string_view name) noexcept
{
  for (size_t i = 0; i < storage.size(); i++) {
    auto* mod = storage.get<Module>(i);
    if (mod->name == name) return mod;
  }
  return nullptr;
}

module::Module module::Module::from_system(std::string_view p_name, std::string_view p_debug_name) noexcept
{
  Module mod;
  mod.name       = p_name;
  mod.debug_name = p_debug_name;
  mod.kind       = EModuleKind::System;
  return mod;
}
