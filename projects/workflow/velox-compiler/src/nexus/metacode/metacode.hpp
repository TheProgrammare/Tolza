#pragma once

#include <cassert>
#include <cstddef>
#include <unordered_map>
#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


#include "definition.hpp"


namespace metacode
{


struct Graph final {
  struct Audit final {
    Graph& graph;

    [[nodiscard]] bool               contains(ID id, MetaKey s) const noexcept;
    [[nodiscard]] bool               contains(ID id, token::ETokenKind tok) const noexcept;
    [[nodiscard]] const Instruction* get_instruction(ID start_id, MetaPattern pattern) const noexcept;
    [[nodiscard]] const Metacode*    get_metacode(ID start_id, MetaPattern pattern) const noexcept;
    // will returns the closets metacode to file_pos
    [[nodiscard]] const Metacode*    get_metacode_from_pos(size_t file_pos) const noexcept;
  };

  Graph()
  {
    (void)add<metacode::Root>(NO_ID);
  }

  bool freeze = false;

  Audit audit{*this};

  std::vector<Metacode*> metacodes;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  ~Graph()
  {
    for (const auto* m : metacodes) delete m;
  }

  template <DerivedMetacode T>
  [[nodiscard]] ID add(ID p_parent)
  {
    assert(!freeze && "Pool is immutable after preprocessor pass");

    const auto new_id = ID::make(metacodes.size());

    T* obj      = new T();
    obj->metaid = new_id;
    metacodes.emplace_back(obj);

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].emplace_back(new_id);
    }

    return new_id;
  }

  [[nodiscard]] Metacode& get(ID id) noexcept
  {
    assert(id.raw() < metacodes.size());
    return *metacodes[id.raw()];
  }

  template <DerivedMetacode T>
  [[nodiscard]] T* as(ID id) noexcept
  {
    Metacode* m = &get(id);
    if (!m) return nullptr;

    if (m->kind() != T::static_kind) return nullptr;

    return static_cast<T*>(m);
  }

  [[nodiscard]] Root& get_file_root() noexcept
  {
    return *as<Root>(ID::make(0));
  }
};


} // namespace metacode