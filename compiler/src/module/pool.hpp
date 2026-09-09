#pragma once

#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "nexus/forward.hpp"
#include "pool/stable_storage.hpp"

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


namespace module
{

struct Module;
struct Graph;


struct Graph final {
  Graph() = delete;

  Graph(cu::ID _parent_cuid, cu::ID _cuid);

  bool         freeze = false;
  const cu::ID cuid;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  // origin, target
  std::unordered_map<ast::ID, ID, ast::ID::Hash> shortcuts;

  [[nodiscard]] Module* get_by_name(std::string_view name) noexcept;


  [[nodiscard]] Module& get(ID modid) noexcept
  {
    assert(modid && "Must be valid id");
    assert(modid.index() < storage.size());
    return *storage.get<Module>(modid.index());
  }
  [[nodiscard]] const Module& get(ID modid) const noexcept
  {
    assert(modid && "Must be valid id");
    assert(modid.index() < storage.size());
    return const_cast<Graph*>(this)->get(modid);
  }

  [[nodiscard]] ID add(ID p_parent, Module& mod_data) noexcept;

  [[nodiscard]] Module& get_file_root() noexcept
  {
    return *storage.get<Module>(0);
  }

private:
  StableStorage storage;
};


void initialization();

// index : 0
[[nodiscard]] static Module& get_root() noexcept
{
  return ID::make(cu::ID::main(), 0).get();
}
// index : 1
[[nodiscard]] static Module& get_src() noexcept
{
  return ID::make(cu::ID::main(), 1).get();
}
// index : 2
[[nodiscard]] static Module& get_std() noexcept
{
  return ID::make(cu::ID::main(), 2).get();
}
// index : 3
[[nodiscard]] static Module& get_pkg() noexcept
{
  return ID::make(cu::ID::main(), 3).get();
}
// index : 4
[[nodiscard]] static Module& get_bind() noexcept
{
  return ID::make(cu::ID::main(), 4).get();
}
// index : 5
[[nodiscard]] static Module& get_vendor() noexcept
{
  return ID::make(cu::ID::main(), 5).get();
}


} // namespace module