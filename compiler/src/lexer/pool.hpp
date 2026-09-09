#pragma once

#include "id/cuid.hpp"
#include "id/tokid.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

// INFO
// no more storage optimization
// Token struct size is only 16 bytes

namespace token
{

constexpr uint32_t INVALID_POS = -1;
constexpr uint16_t INVALID_LEN = -1;


struct Token final {
  ID         tokid;
  uint32_t   begin  = INVALID_POS;
  uint16_t   length = INVALID_LEN;
  ETokenKind kind;
};

struct Arena final {
  Arena(cu::ID& _cuid)
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

  [[nodiscard]] const Token& get(const ID& id) const noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }

  [[nodiscard]] Token& get(const ID& id) noexcept
  {
    const auto offset = id.index();
    assert(id && offset < tokens.size());
    return tokens[offset];
  }
};


[[nodiscard]] bool str_is_identifier(std::string_view s);

[[nodiscard]] bool isKeywordChar(char ch);

ETokenKind str_to_ETokenKind(std::string_view str);

} // namespace token
