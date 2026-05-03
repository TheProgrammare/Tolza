#pragma once

#include "nexus/ids.hpp"
#include "nexus/forward.hpp"
#include "nexus/module.hpp"

#include <cassert>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace symbol
{

class DEF_SYMBOL_ID
{
  size_t id;

public:
  explicit constexpr DEF_SYMBOL_ID(size_t value) noexcept
    : id(value)
  {
  }

  auto operator<=>(const DEF_SYMBOL_ID&) const = default;
  auto operator<=>(size_t other) const
  {
    return id <=> other;
  }
};

struct Symbol final {
  _id id;

  ast::_gnid  gnid;
  module::_id module_id; // module source

  type::_id           type;       // resolved
  module::EVisibility visibility; // resolved

  std::string      mangle_name() const;
  std::string_view get_name() const;
};


struct Arena final {
  std::vector<Symbol> symbols;

  _id add(Symbol sym)
  {
    sym.id = _id(symbols.size());
    symbols.push_back(std::move(sym));
    return _id(symbols.size() - 1);
  }

  const Symbol& get(_id id) const
  {
    assert(id < symbols.size());
    return symbols[id.value()];
  }

  Symbol& get_mut(_id id)
  {
    assert(id < symbols.size());
    return symbols[id.value()];
  }

  std::string_view get_sym_name(_id sym_id) const;
};


} // namespace symbol