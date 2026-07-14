#pragma once

#include "nexus/ids.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <string>
#include <vector>

namespace definition
{

class DEF_SYMBOL_ID
{
  size_t id;

public:
  explicit constexpr DEF_SYMBOL_ID(size_t value) noexcept
    : id(value)
  {
  }

  [[nodiscard]] auto operator<=>(const DEF_SYMBOL_ID&) const noexcept = default;
  [[nodiscard]] auto operator<=>(size_t other) const noexcept
  {
    return id <=> other;
  }
};

struct Definition final {
  ID defid;

  ast::ID nodeid;

  EVisibility visibility; // resolved

  [[nodiscard]] std::string mangle_name() const noexcept;
  [[nodiscard]] std::string get_name() const noexcept;
};

[[nodiscard]] Definition& get(ID defid) noexcept;

struct Arena final {
  Arena() = delete;

  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
  }
  bool freeze = false;

  const cu::ID cuid;

  std::vector<Definition> definitions;

  [[nodiscard]] ID add(Definition defid) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    auto new_id = ID::make(cuid, definitions.size());
    defid.defid = new_id;

    definitions.emplace_back(defid);
    return new_id;
  }

  // if nullptr : node is not a declaration or not declared
  [[nodiscard]] Definition* from_node(ast::ID nodeid) noexcept;

  [[nodiscard]] const Definition& get(ID defid) const noexcept
  {
    assert(defid.cu() == cuid && "Must be the same script");
    assert(defid.offset() < definitions.size());
    return definitions[defid.offset()];
  }

  [[nodiscard]] Definition& get(ID defid) noexcept
  {
    assert(defid.cu() == cuid && "Must be the same script");
    assert(defid.offset() < definitions.size());
    return definitions[defid.offset()];
  }

  [[nodiscard]] std::string get_def_name(ID defid) const noexcept;

  void inject_to_resolved_def() const noexcept;
};


} // namespace definition