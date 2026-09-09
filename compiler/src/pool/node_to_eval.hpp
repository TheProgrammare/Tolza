#pragma once

#include "id/base.hpp"
#include "id/nodeid.hpp"

#include <unordered_map>

namespace evaluated
{

struct Arena final {

  std::unordered_map<ast::ID, ast::ID, ast::ID::Hash> constants;

  void add(ast::ID nodeid, ast::ID constant) noexcept
  {
    constants.emplace(nodeid, constant);
  }


  [[nodiscard]] ast::ID get(ast::ID nodeid) noexcept
  {
    auto it = constants.find(nodeid);
    if (it != constants.end()) return it->second;

    return NO_ID;
  }

  [[nodiscard]] bool is_evaluated(ast::ID nodeid) const noexcept
  {
    return constants.contains(nodeid);
  }
};


} // namespace evaluated