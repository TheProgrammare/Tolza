#pragma once

#include "id/base.hpp"
#include "id/id_query.hpp"

namespace scope
{
struct Scope;

// scope identifier
class ID final : public ::ID<ID, Scope>
{
  ID_HEADER(Scope)
public:
  // get parent
  [[nodiscard]] ID         parent() const noexcept;
  // get node scope owner
  [[nodiscard]] ast::ID    node() const noexcept;
  // get scope parent module
  [[nodiscard]] module::ID module() const noexcept;
};

} // namespace scope