#pragma once

#include "nexus/ids.hpp"
#include "nexus/forward.hpp"
#include "nexus/module.hpp"

#include <cassert>
#include <string>
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

  [[nodiscard]] auto operator<=>(const DEF_SYMBOL_ID&) const = default;
  [[nodiscard]] auto operator<=>(size_t other) const
  {
    return id <=> other;
  }
};

struct Symbol final {
  ID symid;

  ast::ID nodeid;

  EVisibility visibility; // resolved

  [[nodiscard]] std::string mangle_name() const;
  [[nodiscard]] std::string get_name() const;
};

[[nodiscard]] Symbol& get(ID symid) noexcept;

struct Arena final {
  Arena() = delete;

  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
  }
  bool freeze = false;

  const cu::ID cuid;

  std::vector<Symbol> symbols;

  [[nodiscard]] ID add(Symbol sym) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    auto new_id = ID::make(cuid, symbols.size());
    sym.symid   = new_id;

    symbols.emplace_back(sym);
    return new_id;
  }

  // if nullptr : node is not a declaration or not declared
  [[nodiscard]] Symbol* from_node(ast::ID nodeid) noexcept;

  [[nodiscard]] const Symbol& get(ID symid) const noexcept
  {
    assert(symid.cu() == cuid && "Must be the same script");
    assert(symid.offset() < symbols.size());
    return symbols[symid.offset()];
  }

  [[nodiscard]] Symbol& get(ID symid) noexcept
  {
    assert(symid.cu() == cuid && "Must be the same script");
    assert(symid.offset() < symbols.size());
    return symbols[symid.offset()];
  }

  [[nodiscard]] std::string get_sym_name(ID symid) const;
};


} // namespace symbol