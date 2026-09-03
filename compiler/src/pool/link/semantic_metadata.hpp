#pragma once

#include "nexus/forward.hpp"

#include <unordered_map>


namespace semantic
{

struct Metadata final {
  EBuiltin_Member member          = EBuiltin_Member::NONE;
  size_t          static_size     = 0;
  size_t          member_position = 0;
  bool            variadic_arg    = false;
  ast::ID         param_def;
};

struct Arena final {
  bool freeze = false;

  std::unordered_map<ast::ID, Metadata*, ast::ID::Hash> bindings;

  Metadata& add(ast::ID nodeid) noexcept
  {
    assert(!freeze && "Pool is immutable after semantic resolution");

    auto* ref = new Metadata();
    bindings.emplace(nodeid, ref);
    return *ref;
  }


  [[nodiscard]] Metadata* get_metadata(ast::ID nodeid) noexcept
  {
    auto it = bindings.find(nodeid);
    if (it != bindings.end()) return it->second;

    return nullptr;
  }
};


} // namespace semantic