#include "token.hpp"

#include <string_view>
#include "compiler/compilation_unit.hpp"

token::ID token::Arena::add(Token& tok) noexcept
{
  auto id   = token::ID::make(cuid, tokens.size());
  tok.tokid = id;
  tokens.emplace_back(std::move(tok));
  return id;
}

std::string_view token::Arena::Audit::Token_to_str(ID tokid) const noexcept
{
  auto&       tok = arena.get(tokid);
  const auto& cu  = arena.cuid.get();
  assert(cu.file_info.data.data());
  if (tok.begin == cu.file_info.data.size()) {
    return {cu.file_info.data.data() + tok.begin - 1, 1};
  }
  assert(tok.begin + tok.length <= cu.file_info.data.size());
  return {cu.file_info.data.data() + tok.begin, tok.length};
}
size_t token::Arena::Audit::Token_to_line(ID tokid) const noexcept
{
  auto& tok = arena.get(tokid);
  return arena.cuid.get().file_info.get_line_from_pos(tok.begin);
}
std::string_view token::Arena::Audit::Token_to_line_str(ID tokid) const noexcept
{
  auto&       tok  = arena.get(tokid);
  const auto& cu   = arena.cuid.get();
  size_t      line = cu.file_info.get_line_from_pos(tok.begin);
  return cu.file_info.get_line(line);
}