#include "pool/node_resolved.hpp"

#include "id/base.hpp"
#include "id/defid.hpp"
#include "id/nodeid.hpp"

#include <cstdlib>


definition::ID resolved::Arena::get_definition(ast::ID nodeid) noexcept
{
  // assume finalize() called before
  auto it = bindings.find(nodeid);
  if (it != bindings.end()) return it->second;

  return NO_ID;
}