#pragma once

#include <algorithm>
#include <unordered_map>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace inference
{


struct Arena {
  bool freeze = false;

  std::unordered_map<ast::ID, type::ID, ast::ID::Hash> inference;

  void add(ast::ID n, type::ID type)
  {
    assert(!freeze && "Pool is immutable after type resolution");

    inference.try_emplace(n, type);
  }

  [[nodiscard]] ast::ID get_declaration(type::ID ty) const
  {
    auto result = std::ranges::find_if(inference, [&](std::pair<ast::ID, type::ID> pair) -> bool {
      auto& nodeid = pair.first;
      auto& tyid   = pair.second;
      return tyid == ty;
    });

    if (result != inference.end()) return result->first;

    return NO_ID;
  }

  [[nodiscard]] type::ID get_inference(ast::ID n) const
  {
    auto it = inference.find(n);
    if (it != inference.end()) return it->second;

    return NO_ID;
  }

  [[nodiscard]] bool is_inferred(ast::ID n) const
  {
    return inference.find(n) != inference.end();
  }
};


} // namespace inference