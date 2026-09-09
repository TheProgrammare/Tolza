#pragma once


#include "id/base.hpp"

#include <cstddef>
#include <string_view>

namespace token
{

struct Token;

// token identifier
class ID final : public ::ID<ID, Token>
{
  ID_HEADER(Token)

public:
  // get string representation on file
  [[nodiscard]] std::string_view str() const noexcept;
  // get file position
  [[nodiscard]] size_t           pos() const noexcept;
  // get line position
  [[nodiscard]] size_t           line() const noexcept;
  // get column position
  [[nodiscard]] size_t           col() const noexcept;
  // get all line string respresentation on file
  [[nodiscard]] std::string_view line_str() const noexcept;
};

} // namespace token