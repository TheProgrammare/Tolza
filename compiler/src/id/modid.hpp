#pragma once

#include "id/base.hpp"
#include "id/id_query.hpp"

namespace module
{
struct Module;

// module identifier
class ID final : public ::ID<ID, Module>
{
  ID_HEADER(Module)
public:
  // get parent
  [[nodiscard]] ID        parent() const noexcept;
  // get module node owner
  [[nodiscard]] ast::ID   node() const noexcept;
  // get module base scope
  [[nodiscard]] scope::ID scope() const noexcept;
};

} // namespace module
