#include "token_viewer.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"
#include "nexus/ids.hpp"
#include "pool/token.hpp"

#include <cstddef>


token::Viewer::Viewer(cu::CU& p_CU)
  : CU(p_CU)
{
  phase         = compiler::EPhase::parser;
  current_token = token::ETokenKind::S_END_OF_FILE;
}


token::Token& token::Viewer::expect(ErrorCode code, token::ETokenKind kind, std::string_view msg,
                                    std::string_view hint) noexcept
{
  if (!check(kind)) {
    add_error(code, msg, hint);
  }
  return next();
}

token::Token& token::Viewer::expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& kinds,
                                        std::string_view msg, std::string_view hint) noexcept
{
  for (auto kind : kinds) {
    if (check(kind)) {
      return next();
    }
  }
  add_error(code, msg, hint);
  return *CU.file_info.tokens->tokens.end();
}

void token::Viewer::add_error(ErrorCode code, std::string_view msg, std::string_view hint) noexcept
{
  auto&  tok        = peek();
  size_t start      = tok.begin;
  size_t end        = start + tok.length;
  auto   error_diag = Error_Diagnostic(CU.cuid, code, start, end, phase, msg, hint);

  COMPILER.add_error(error_diag);
}

void token::Viewer::add_error_tok(ErrorCode code, const Token& tok, std::string_view msg,
                                  std::string_view hint) noexcept
{
  size_t start      = tok.begin;
  size_t end        = start + tok.length;
  auto   error_diag = Error_Diagnostic(CU.cuid, code, start, end, phase, msg, hint);

  COMPILER.add_error(error_diag);
}

void token::Viewer::jump(size_t newPosition) noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  if (newPosition < tokens.size()) {
    current = newPosition;
  } else {
    current = tokens.size() - 1;
  }

#ifdef DEBUG
  current_str   = tokens[current].debug_val;
  current_token = tokens[current].kind;
#endif
}

token::Token& token::Viewer::next() noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  if (!is_end()) {
    auto& pre_tok = tokens[current];
    current++;
    auto& post_tok = tokens[current];
#ifdef DEBUG
    current_str   = post_tok.debug_val;
    current_token = post_tok.kind;
#endif
    return pre_tok;
  }
  return tokens.back();
}

token::Token& token::Viewer::peek(size_t offset) const noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  size_t index = current + offset;
  if (index < tokens.size()) return tokens[index];
  return tokens.back();
}

token::Token& token::Viewer::prev() noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  if (current == 0) {
    return tokens[0];
  }
  current--;
  return tokens[current];
}

bool token::Viewer::is_end() const noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  return current == tokens.size() - 1
         || CU.file_info.tokens->get(token::ID::make(CU.cuid, current)).kind == token::ETokenKind::S_END_OF_FILE;
}

bool token::Viewer::look_ahead(token::ETokenKind check, token::ETokenKind terminaison) const noexcept
{
  for (size_t offset = 0;; offset++) {
    token::ETokenKind type = peek(offset).kind;
    if (type == check) return true;
    if (type == terminaison) return false;
    if (type == token::ETokenKind::S_END_OF_FILE) return false;
    offset++;
  }
}

// will ignore new line if not specified
bool token::Viewer::check(token::ETokenKind kind) const noexcept
{
  if (!is_end()) {
    return peek().kind == kind;
  }
  return false;
}

bool token::Viewer::check_any(const std::initializer_list<token::ETokenKind>& kinds) const noexcept
{
  for (auto type : kinds) {
    if (check(type)) return true;
  }
  return false;
}

bool token::Viewer::match(token::ETokenKind kind) noexcept
{
  if (check(kind)) {
    (void)next();
    return true;
  }
  return false;
}

bool token::Viewer::check_id_val(std::string_view val) const noexcept
{
  auto& tok = peek();
  auto  str = CU.file_info.tokens->audit.Token_to_str(tok.tokid);

  if (check(token::ETokenKind::IDENTIFIER)) {
    return str == val;
  }
  return false;
}

bool token::Viewer::check_val(std::string_view val) const noexcept
{
  auto& tok = peek();
  auto  str = CU.file_info.tokens->audit.Token_to_str(tok.tokid);

  return str == val;
}

bool token::Viewer::check_chain(const std::initializer_list<token::ETokenKind>& l) const noexcept
{
  for (size_t i = 0; i < l.size(); i++) {
    const auto& t  = *(l.begin() + i);
    const auto& tk = peek(i);
    if (tk.kind != t) return false;
  }

  return true;
}

bool token::Viewer::match_chain(const std::initializer_list<token::ETokenKind>& l) noexcept
{
  const bool result = check_chain(l);

  if (result) jump(position() + l.size());

  return result;
}


std::string token::Viewer::match_any_val(const std::initializer_list<std::string>& val) noexcept
{
  auto& tok = peek();
  auto  str = CU.file_info.tokens->audit.Token_to_str(tok.tokid);

  for (const auto& elem : val) {
    if (str == elem) {
      (void)next();
      return elem;
    }
  }

  return {};
}

bool token::Viewer::match_val(std::string_view val) noexcept
{
  auto& tok = peek();
  auto  str = tok.tokid.str();

  if (str == val) {
    (void)next();
    return true;
  }
  return false;
}

bool token::Viewer::match_id_val(std::string_view val) noexcept
{
  if (check_id_val(val)) {
    (void)next();
    return true;
  }
  return false;
}

bool token::Viewer::match_any(const std::initializer_list<token::ETokenKind>& kinds) noexcept
{
  for (auto kind : kinds) {
    if (check(kind)) {
      (void)next();
      return true;
    }
  }
  return false;
}

size_t token::Viewer::size() const noexcept
{
  return CU.file_info.tokens->tokens.size();
}

size_t token::Viewer::position() const noexcept
{
  return current;
}

size_t token::Viewer::line() const noexcept
{
  auto line = CU.file_info.get_line_from_pos(peek().begin);

  return line;
}

// Go back to a know position
void token::Viewer::rewind(size_t pos) noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  if (pos >= tokens.size()) pos = tokens.size() - 1;
  current = pos;
#ifdef DEBUG
  current_str   = tokens[pos].debug_val;
  current_token = tokens[pos].kind;
#endif
}

const token::Token& token::Viewer::get(size_t pos) const noexcept
{
  auto& tokens = CU.file_info.tokens->tokens;

  if (tokens.size() < pos) return tokens.back();
  return tokens[pos];
}