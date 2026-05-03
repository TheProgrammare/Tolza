#pragma once

#include "nexus/forward.hpp"


namespace resolved
{
struct Binding final {
  ast::_gnid  node;
  symbol::_id sym;
};

struct Arena final {

  std::vector<Binding> bindings;
  bool                 sorted = false;

  void add(ast::_gnid n, symbol::_id sym)
  {
    bindings.push_back({n, sym});
    sorted = false;
  }

  void finalize();

  symbol::_id get_symbol(ast::_gnid n);

  bool is_resolved(ast::_gnid n)
  {
    return bool(get_symbol(n));
  }
};

} // namespace resolved