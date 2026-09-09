#include "id/tokid.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/file_info.hpp"
#include "lexer/pool.hpp"

#include <cassert>
#include <cstddef>
#include <string_view>

std::string_view token::ID::str() const noexcept
{
  assert(*this && "Must be valid id");
  const auto* arena = cu().get().file_info.tokens;
  const auto& data  = cu().get().file_info.data;

  const auto& tok = arena->get(*this);
  assert(data.data() && "File must have a textual representation");
  if (tok.begin == data.size()) {
    return {data.data() + tok.begin - 1, 1};
  }
  assert(tok.begin + tok.length <= data.size());
  return {data.data() + tok.begin, tok.length};
}
size_t token::ID::pos() const noexcept
{
  assert(*this && "Must be valid id");
  return get().begin;
}
size_t token::ID::line() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_line_from_pos(pos());
}
size_t token::ID::col() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_column_from_pos(pos());
}
std::string_view token::ID::line_str() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_line(line());
}