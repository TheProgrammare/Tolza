#pragma once

#include "id/base.hpp"
#include "id/id_query.hpp"

namespace definition
{
struct Definition;

// definition identifier
class ID final : public ::ID<ID, Definition>
{
  ID_HEADER(Definition)
public:
  // get definition type
  [[nodiscard]] type::ID   type() const noexcept;
  // get definition node reference
  [[nodiscard]] ast::ID    node() const noexcept;
  // scope definition owner
  [[nodiscard]] scope::ID  scope() const noexcept;
  // module definition owner
  [[nodiscard]] module::ID module() const noexcept;
};

} // namespace definition