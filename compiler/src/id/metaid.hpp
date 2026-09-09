#pragma once

#include "id/base.hpp"

namespace metacode
{
struct MetacodeHeader;

// metacode identifier
class ID final : public ::ID<ID, MetacodeHeader>
{
  ID_HEADER(MetacodeHeader)
};

} // namespace metacode