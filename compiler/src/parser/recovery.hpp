#pragma once

#include "nexus/forward.hpp"

#include <exception>
#include <string_view>

namespace parser
{

struct Parser_Exception : std::exception {
  Parser_Exception() = delete;
  explicit Parser_Exception(Parser_Context& p, ErrorCode code, const token::Token& tok, std::string_view msg,
                            std::string_view hint) noexcept;
};

namespace error
{
bool recover(Parser_Context& p);

// returns true if the sync is not a the end of file
bool recover_sync(token::Viewer& v);
} // namespace error

} // namespace parser