#include "id/base.hpp"

#include <cstdint>
#include <functional>


uint64_t base_hash(uint64_t x) noexcept
{
  return std::hash<uint64_t>{}(x);
}
