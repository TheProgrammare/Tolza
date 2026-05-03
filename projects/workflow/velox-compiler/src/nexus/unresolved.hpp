#pragma once


#include "nexus/ids.hpp"
#include <unordered_set>


namespace unresolved
{

struct Arena final {

  std::unordered_set<ast::_gnid, ast::_gnid_hash> nodes;
  bool                                            sorted = false;

  void add(ast::_gnid n)
  {
    nodes.insert(n);
    sorted = false;
  }

  bool is_unresolved(ast::_gnid n)
  {
    return nodes.contains(n);
  }
};

} // namespace unresolved