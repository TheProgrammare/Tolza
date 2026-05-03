#include "token.hpp"

#include <iostream>
#include <string_view>
#include "nexus/script.hpp"

std::string_view token::Arena::Audit::Token_to_str(_id id)
{
  auto tok = arena.get(id);
  assert(arena.scr_info.file_info.data.data());
  assert(tok.begin + tok.length <= arena.scr_info.file_info.data.size());
  return std::string_view(arena.scr_info.file_info.data.data() + tok.begin, tok.length);
}
size_t token::Arena::Audit::Token_to_line(_id id)
{
  auto tok = arena.get(id);
  return arena.scr_info.file_info.get_line_from_pos(tok.begin);
}
std::string_view token::Arena::Audit::Token_to_line_str(_id id)
{
  auto   tok  = arena.get(id);
  size_t line = arena.scr_info.file_info.get_line_from_pos(tok.begin);
  return arena.scr_info.file_info.get_line(line);
}