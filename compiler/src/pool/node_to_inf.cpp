#include "pool/node_to_inf.hpp"

#include "ast/pool.hpp"
#include "id/base.hpp"
#include "nexus/forward.hpp"
#include "type/pool.hpp"


void inference::initialization() noexcept
{
  builtin_inference.emplace(ast::NODEID_BUILTIN_Member_Access_Len, type::TYPEID_usize);
  builtin_inference.emplace(ast::NODEID_BUILTIN_Member_Access_Capa, type::TYPEID_usize);
}


[[nodiscard]] type::ID inference::Arena::get_inference(ast::ID nodeid) const noexcept
{
  if (nodeid.raw() < ast::NODEID_USER_START) {
    auto it = builtin_inference.find(nodeid);
    if (it != builtin_inference.end()) return it->second;
  }

  auto it = inference.find(nodeid);
  if (it != inference.end()) return it->second;

  return NO_ID;
}

[[nodiscard]] bool inference::Arena::is_inferred(ast::ID nodeid) const noexcept
{
  return inference.find(nodeid) != inference.end();
}