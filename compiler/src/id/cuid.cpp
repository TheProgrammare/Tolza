#include "id/cuid.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "pipeline/pipeline.hpp"

#include <cassert>
#include <cstdint>
#include <functional>


cu::CU& cu::ID::get() noexcept
{
  auto& scrs = PIPELINE.compilation_units;

  assert(*this && "Must be valid id");
  auto _offset = offset();

  assert(_offset < UINT32_MAX && "ID index will overflow on encoding");

  if (is_temp()) {
    auto& temps = PIPELINE.temp_compilation_units;
    assert(_offset < temps.size() && _offset >= 0 && "ID index is out of bound");
    return *static_cast<cu::CU*>(temps.at(_offset));
  }

  assert(_offset < scrs.size() && _offset >= 0 && "ID index is out of bound");

  return *scrs.at(_offset);
}
const cu::CU& cu::ID::get() const noexcept
{
  return const_cast<cu::ID*>(this)->get();
}

uint64_t cu::ID::Hash::operator()(const ID& x) const noexcept
{
  return std::hash<uint64_t>{}(x.raw());
}