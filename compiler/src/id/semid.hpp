#pragma once

#include "id/base.hpp"

namespace semantic
{
struct Metadata;
class ID final : public ::ID<ID, Metadata>
{

  ID_HEADER(Metadata)
};
} // namespace semantic