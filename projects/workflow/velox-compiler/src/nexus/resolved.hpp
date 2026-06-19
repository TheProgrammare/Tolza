#pragma once

#include "nexus/forward.hpp"


namespace resolved
{
struct Binding final {
  ast::ID    node;
  symbol::ID sym;

  auto operator<=>(const Binding& other) const noexcept
  {
    return sym <=> other.sym;
  }
};

struct Arena final {

  bool freeze = false;

  std::vector<Binding> bindings;
  bool                 sorted = false;

  void add(ast::ID n, symbol::ID sym)
  {
    assert(!freeze && "Pool is immutable after symbol resolution");

    bindings.emplace_back(Binding{.node = n, .sym = sym});
    sorted = false;
  }

  void finalize();

  [[nodiscard]] symbol::ID get_symbol(ast::ID n);

  [[nodiscard]] bool is_resolved(ast::ID n)
  {
    return bool(get_symbol(n));
  }
};

} // namespace resolved