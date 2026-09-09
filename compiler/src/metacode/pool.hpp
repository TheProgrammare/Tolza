#pragma once

#include "id/base.hpp"
#include "metacode/data.hpp"
#include "nexus/forward.hpp"
#include "pool/stable_storage.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace metacode
{

struct MetacodeEntry {
  MetacodeEntry(void* p_data, ID p_metaid, EMetacodeKind p_kind)
    : data(p_data)
    , metaid(p_metaid)
    , kind(p_kind)
  {
  }

  void*         data = nullptr;
  ID            metaid;
  EMetacodeKind kind = EMetacodeKind::Root;
};

struct Graph final {
  struct Audit final {
    Graph& graph;

    [[nodiscard]] bool                  contains(ID id, std::string_view s) const noexcept;
    [[nodiscard]] bool                  contains(ID id, token::ETokenKind tok) const noexcept;
    [[nodiscard]] const Instruction*    get_instruction(ID                                      start_id,
                                                        std::initializer_list<std::string_view> pattern) const noexcept;
    [[nodiscard]] const MetacodeHeader* get_metacode(ID                                      start_id,
                                                     std::initializer_list<std::string_view> pattern) const noexcept;
    // will returns the closets metacode to file_pos
    [[nodiscard]] const MetacodeHeader* get_metacode_from_pos(size_t file_pos) const noexcept;
  };

  Graph();


  bool freeze = false;

  Audit audit{*this};

  std::vector<MetacodeEntry>                        entries;
  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;


  template <metacode::Generic T>
  [[nodiscard]] ID add(ID p_parent = NO_ID)
  {
    assert(!freeze && "Pool is immutable after preprocessor pass");

    const auto new_id = ID::make(storage.size());

    auto* meta          = storage.create_get<T>();
    meta->header.metaid = new_id;
    entries.emplace_back(MetacodeEntry(meta, new_id, T::static_kind));

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].emplace_back(new_id);
    }

    return new_id;
  }

  [[nodiscard]] MetacodeHeader& get(ID id) noexcept
  {
    assert(id && "Must be valid id");
    assert(id.raw() < storage.size());
    return *storage.get<MetacodeHeader>(id.index());
  }

  template <Generic T>
  [[nodiscard]] T* as(ID id) noexcept
  {
    assert(id && "Must be valid id");
    assert(id.raw() < entries.size());

    MetacodeEntry* entry = &entries[id.index()];
    if (entry->kind != T::static_kind) return nullptr;

    return static_cast<T*>(entry->data);
  }

  template <Generic T>
  [[nodiscard]] const T* as(ID id) const noexcept
  {
    assert(id && "Must be valid id");
    assert(id.raw() < entries.size());

    return const_cast<Graph*>(this)->as<T>(id);
  }


  [[nodiscard]] Root& get_file_root() noexcept
  {
    return *as<Root>(ID::make(0));
  }

private:
  StableStorage storage;
};


} // namespace metacode