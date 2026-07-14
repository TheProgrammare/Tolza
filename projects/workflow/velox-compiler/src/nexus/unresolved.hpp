#pragma once


#include "nexus/ids.hpp"
#include <unordered_set>


namespace unresolved
{

struct Arena final {

  std::unordered_set<ast::ID, ast::ID::Hash> nodes;
  bool                                       sorted = false;

  void add(ast::ID nodeid) noexcept
  {
    nodes.insert(nodeid);
    sorted = false;
  }

  [[nodiscard]] bool is_unresolved(ast::ID nodeid) const noexcept
  {
    return nodes.contains(nodeid);
  }
};

} // namespace unresolved