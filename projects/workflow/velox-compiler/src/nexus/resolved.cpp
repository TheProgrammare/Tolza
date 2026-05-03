#include "resolved.hpp"
#include "nexus/ast/ast.hpp"
#include <algorithm>

void resolved::Arena::finalize()
{
  if (!sorted) {
    std::sort(bindings.begin(), bindings.end(), [](const Binding& a, const Binding& b) { return a.node < b.node; });
    sorted = true;
  }
}

symbol::_id resolved::Arena::get_symbol(ast::_gnid n)
{
  if (!sorted) finalize();
  sorted = true;

  // assume finalize() called before
  auto it = std::lower_bound(bindings.begin(), bindings.end(), n,
                             [](const Binding& b, ast::_gnid value) { return b.node < value; });

  if (it != bindings.end() && it->node == n) return it->sym;

  return NO_ID;
}