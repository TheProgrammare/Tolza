#pragma once

#include "id/base.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "nexus/forward.hpp"

#include <algorithm>
#include <cassert>
#include <unordered_map>


namespace inference
{


inline std::unordered_map<ast::ID, type::ID, ast::ID::Hash> builtin_inference;

void initialization() noexcept;

struct Arena {
  bool freeze = false;

  std::unordered_map<ast::ID, type::ID, ast::ID::Hash> inference;

  void add(ast::ID nodeid, type::ID tyid) noexcept
  {
    assert(tyid && "Invalid type");

    assert(!freeze && "Pool is immutable after type resolution");

    inference.insert_or_assign(nodeid, tyid);
  }

  [[nodiscard]] ast::ID get_declaration(type::ID tyid) const noexcept
  {
    auto result = std::ranges::find_if(inference, [&](const auto& pair) -> bool {
      auto& nodeid = pair.first;
      auto& tyid   = pair.second;
      return tyid == tyid;
    });

    if (result != inference.end()) return result->first;

    return NO_ID;
  }

  [[nodiscard]] type::ID get_inference(ast::ID nodeid) const noexcept;

  [[nodiscard]] bool is_inferred(ast::ID nodeid) const noexcept;
};


} // namespace inference