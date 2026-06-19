#pragma once


#include "nexus/ids.hpp"
#include <unordered_set>


namespace unresolved
{

struct Arena final {

  std::unordered_set<ast::ID, ast::ID::Hash> nodes;
  bool                                       sorted = false;

  void add(ast::ID n) noexcept
  {
    nodes.insert(n);
    sorted = false;
  }

  [[nodiscard]] bool is_unresolved(ast::ID n) const noexcept
  {
    return nodes.contains(n);
  }
};

} // namespace unresolved