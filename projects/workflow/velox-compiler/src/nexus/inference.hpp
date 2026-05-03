#pragma once

#include <unordered_map>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"


namespace inference
{


struct Arena {
  std::unordered_map<ast::_gnid, type::_id, ast::_gnid_hash> inference;

  void add(ast::_gnid n, type::_id type)
  {
    inference[n] = type;
  }

  type::_id get_inference(ast::_gnid n) const
  {
    auto it = inference.find(n);
    if (it == inference.end()) return NO_ID;

    return it->second;
  }

  bool is_inferred(ast::_gnid n) const
  {
    return inference.find(n) != inference.end();
  }
};


} // namespace inference