#pragma once

#include "lexer/definition.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <map>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace token
{

constexpr uint32_t INVALID_POS = -1;
constexpr uint16_t INVALID_LEN = -1;


struct Token final {
  ID         tokid;
  uint32_t   begin  = INVALID_POS;
  uint16_t   length = INVALID_LEN;
  ETokenKind kind   = ETokenKind::NONE;
#ifdef DEBUG
  std::string debug_val;
#endif
};

struct Arena final {
  Arena(cu::ID _cuid)
    : cuid(_cuid)
  {
  }

  struct Audit final {
    Arena& arena;

    [[nodiscard]] std::string_view Token_to_str(ID tokid) const noexcept;
    [[nodiscard]] size_t           Token_to_line(ID tokid) const noexcept;
    [[nodiscard]] std::string_view Token_to_line_str(ID tokid) const noexcept;
  };

  const Audit audit{*this};

  const cu::ID cuid;

  // index = token id
  std::vector<Token> tokens;

  [[nodiscard]] ID add(Token& tok) noexcept;

  [[nodiscard]] const Token& get(ID id) const noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }

  [[nodiscard]] Token& get(ID id) noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }
};


[[nodiscard]] inline bool str_is_identifier(std::string_view s)
{
  if (s.empty()) return false;
  if (!std::isalpha(s[0]) && s[0] != '_') return false;
  for (size_t i = 1; i < s.size(); ++i)
    if (!std::isalnum(s[i]) && s[i] != '_') return false;
  return true;
}

[[nodiscard]] inline bool isKeywordChar(char ch)
{
  return !std::isspace(ch) && !std::iscntrl(ch);
}

[[nodiscard]] inline ETokenKind str_to_ETokenKind(std::string_view str)
{
  if (auto it = k_keywords.find(str); it != k_keywords.end()) return it->second;
  return ETokenKind::NONE;
}

} // namespace token
