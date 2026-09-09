#include "ast/pool.hpp"

#include "ast/node/base.hpp"
#include "id/nodeid.hpp"
#include "nexus/forward.hpp"

#include <cassert>


ast::Arena::Arena(cu::ID _cuid)
  : cuid(_cuid)
{
  (void)add_get<ast::Root>();
}


ast::Root* ast::Arena::get_file_root() noexcept
{
  return as<Root>(ID::make(cuid, 0));
}
