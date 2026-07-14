#pragma once

#include "nexus/forward.hpp"
#include <unordered_map>


namespace resolved
{

struct Arena final {

  bool freeze = false;

  std::unordered_map<ast::ID, definition::ID, ast::ID::Hash> bindings;

  void add(ast::ID nodeid, definition::ID defid) noexcept
  {
    assert(!freeze && "Pool is immutable after symbol resolution");

    bindings.emplace(nodeid, defid);
  }


  [[nodiscard]] definition::ID get_definition(ast::ID nodeid) noexcept;

  [[nodiscard]] bool is_resolved(ast::ID nodeid) const noexcept
  {
    return bindings.contains(nodeid);
  }
};

} // namespace resolved