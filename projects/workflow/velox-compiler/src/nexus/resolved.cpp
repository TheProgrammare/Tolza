#include "resolved.hpp"
#include "nexus/ast/ast.hpp"
#include <algorithm>
#include <bits/ranges_algo.h>

void resolved::Arena::finalize()
{
  if (!sorted) {
    std::sort(bindings.begin(), bindings.end(), [](const Binding& a, const Binding& b) { return a.node < b.node; });
    sorted = true;
  }
}

symbol::ID resolved::Arena::get_symbol(ast::ID n)
{
  if (!sorted) finalize();
  sorted = true;

  // assume finalize() called before
  auto it = std::ranges::lower_bound(bindings, n, std::less{}, &Binding::node);
  if (it != bindings.end() && it->node == n) return it->sym;

  return NO_ID;
}