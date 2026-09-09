#include "id/id_query.hpp"

#include "ast/pool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/file_info.hpp"
#include "id/cuid.hpp"
#include "id/defid.hpp"
#include "id/modid.hpp"
#include "id/scpid.hpp"
#include "id/tokid.hpp"
#include "lexer/pool.hpp"
#include "module/pool.hpp"
#include "pool/node_to_def.hpp"
#include "scope/pool.hpp"
#include "type/pool.hpp"

#include <cassert>
#include <cstdint>


ast::NodeHeader& id_query<ast::NodeHeader>::get(const ast::ID& id)
{
  assert(id && "Must be valid id");
  return id.cu().get().ast->get(id);
}

token::Token& id_query<token::Token>::get(const token::ID& id)
{
  assert(id && "Must be valid id");
  return id.cu().get().file_info.tokens->get(id);
}

type::TypeHeader& id_query<type::TypeHeader>::get(const type::ID& id)
{
  assert(id && "Must be valid id");
  return id.cu().get().types->get(id);
}

module::Module& id_query<module::Module>::get(const module::ID& id)
{
  assert(id && "Must be valid id");
  if (auto cu = id.cu()) return cu.get().modules->get(id);

  switch (id.index()) {
  case 0:  return module::get_root();
  case 1:  return module::get_src();
  case 2:  return module::get_std();
  case 3:  return module::get_pkg();
  case 4:  return module::get_bind();
  case 5:  return module::get_vendor();
  default: return module::get_src();
  }

  assert(false && "Invalid id");
}

scope::Scope& id_query<scope::Scope>::get(const scope::ID& id)
{
  assert(id && "Must be valid id");
  return id.cu().get().scopes->get(id);
}

definition::Definition& id_query<definition::Definition>::get(const definition::ID& id)
{
  assert(id && "Must be valid id");
  return id.cu().get().definitions->get(id);
}
