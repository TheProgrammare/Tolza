#include "symbol.hpp"

#include <algorithm>

#include "compiler/compilation_unit.hpp"
#include "nexus/module.hpp"
#include "nexus/ast/ast.hpp"

std::string symbol::Symbol::mangle_name() const
{
  auto modid = symid.module();

  return module::mangle_canonical_module_path(modid) + "." + std::string(get_name());
}

std::string symbol::Symbol::get_name() const
{
  return ast::get_decl_name(nodeid);
}

std::string symbol::Arena::get_sym_name(ID symid) const
{
  const auto& sym = get(symid);

  return ast::get_decl_name(sym.nodeid);
}

symbol::Symbol* symbol::Arena::from_node(ast::ID nodeid) noexcept
{
  const auto it = std::ranges::find_if(symbols, [&](const symbol::Symbol& sym) { return sym.nodeid == nodeid; });
  if (it == symbols.end()) return nullptr;
  return &(*it);
}


symbol::Symbol& symbol::get(ID symid) noexcept
{
  auto cu = symid.cu();
  assert(cu && "Must be a valid script");
  return cu.get().symbols->get(symid);
}