#pragma once

#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"

#include <unordered_map>
#include <unordered_set>

namespace extension
{

using ExntensionSet = std::unordered_set<ast::ID, ast::ID::Hash>;

struct Arena {
  bool freeze = false;

  std::unordered_map<type::ID, ExntensionSet, type::ID::Hash> extensions;

  void add(type::ID tyid, ast::ID extension) noexcept
  {
    assert(!freeze && "Pool is immutable after parsing pass");

    auto [it, _] = extensions.try_emplace(tyid);
    it->second.emplace(extension);
  }

  [[nodiscard]] type::ID get(type::ID tyid) const noexcept
  {
    auto result = std::ranges::find_if(extensions, [&](const auto& pair) -> bool {
      auto& tyid = pair.first;
      return tyid == tyid;
    });

    if (result != extensions.end()) return result->first;

    return NO_ID;
  }

  [[nodiscard]] const ExntensionSet& get_extensions(type::ID tyid) const
  {
    static const ExntensionSet empty;

    auto it = extensions.find(tyid);
    if (it != extensions.end()) return it->second;

    return empty;
  }

  [[nodiscard]] bool is_extended(type::ID tyid) const
  {
    return extensions.find(tyid) != extensions.end();
  }
};


[[nodiscard]] ast::Global_Extend_Cast*         get_cast(type::ID extended_tyid, type::ID cast_to_tyid) noexcept;
[[nodiscard]] ast::Global_Extend_Op_Bin*       get_op_bin(type::ID tyid, ast::EOp_Bin opty) noexcept;
[[nodiscard]] ast::Global_Extend_Op_Un*        get_op_unary(type::ID tyid, ast::EOp_Unary opun) noexcept;
[[nodiscard]] ast::Global_Extend_Op_Subscript* get_op_subscript(type::ID tyid, ast::EOp_Subscript opsub) noexcept;
[[nodiscard]] ast::Global_Extend_Op_Transfert* get_op_transfert(type::ID tyid, ast::ETransfertType optr) noexcept;
[[nodiscard]] ast::Global_Extend_Op_Other*     get_op_other(type::ID tyid, ast::EOp_Other opot) noexcept;


} // namespace extension