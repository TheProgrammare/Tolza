#include "lexer/data.hpp"
#include "lexer/pool.hpp"
#include "lexer/token_viewer.hpp"
#include "nexus/forward.hpp"
#include "parser/context.hpp"
#include "parser/recovery.hpp"

#include <algorithm>
#include <string_view>

parser::Parser_Exception::Parser_Exception(Parser_Context& p, ErrorCode code, const token::Token& tok,
                                           std::string_view msg, std::string_view hint) noexcept
{
  p.add_error_tok(code, tok, msg, hint);
}


bool parser::error::recover(Parser_Context& p)
{
  // start parsing if end of file not reached
  if (recover_sync(*p.tok_v)) return p.start_parsing();

  return false;
}

bool parser::error::recover_sync(token::Viewer& v)
{
  while (v.is_end() && v.next().tokid) {
    const auto& current = v.peek().kind;

    // return if end of file not reached
    if (std::ranges::contains(token::k_synchronize_point, current)) return !v.is_end();
  }

  // end of file reached
  return false;
}