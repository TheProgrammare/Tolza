#pragma once

#include "id/cuid.hpp"
#include "id/defid.hpp"
#include "id/scpid.hpp"
#include "nexus/forward.hpp"
#include "pool/stable_storage.hpp"

#include <cassert>
#include <string>
#include <unordered_map>
#include <vector>


namespace scope
{

[[nodiscard]] definition::ID find_lexical_symbol(scope::ID ctx, const std::string& name) noexcept;
[[nodiscard]] definition::ID find_in_chain_scope(scope::ID ctx, const std::string& name) noexcept;

[[nodiscard]] std::vector<scope::ID>& get_children(ID id) noexcept;

struct Graph final {
  Graph() = delete;

  Graph(cu::ID _cuid);

  bool freeze = false;

  const cu::ID cuid;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  std::unordered_map<ID, module::ID, ID::Hash> module;


  [[nodiscard]] Scope& get(ID scpid) noexcept
  {

    assert(scpid.cu() == cuid && "Must be the same script");
    assert(scpid.index() < storage.size());
    return *storage.get<Scope>(scpid.index());
  }

  [[nodiscard]] const Scope& get(ID id) const noexcept
  {
    assert(id.cu() == cuid && "Must be the same script");
    assert(id.index() < storage.size());
    return const_cast<Graph*>(this)->get(id);
  }

  [[nodiscard]] ID add(ID p_parent, Scope& p_scp) noexcept;

  [[nodiscard]] Scope& get_file_root() noexcept
  {
    return get(ID::make(cuid, 0));
  }

private:
  StableStorage storage;
};


} // namespace scope