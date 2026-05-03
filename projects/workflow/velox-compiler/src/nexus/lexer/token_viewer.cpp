#include "token_viewer.hpp"

#include <cstddef>
#include <iostream>

#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/script.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"


token::Viewer::Viewer(script::ScriptInfo& p_scr_info)
  : scr_info(p_scr_info)
{
  phase         = compiler::EPhase::parser;
  current_token = token::ETokenKind::S_END_OF_FILE;
}


token::Token& token::Viewer::expect(ErrorCode code, token::ETokenKind type, std::string_view msg,
                                    std::string_view hint) noexcept
{
  if (!check(type)) {
    add_error(code, msg, hint);
  }
  return next();
}

token::Token& token::Viewer::expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types,
                                        std::string_view msg, std::string_view hint) noexcept
{
  for (auto type : types) {
    if (check(type)) {
      return next();
    }
  }
  add_error(code, msg, hint);
  return *scr_info.file_info.tokens->tokens.end();
}

void token::Viewer::add_error(ErrorCode code, std::string_view msg, std::string_view hint)
{
  auto   tok        = peek();
  size_t start      = tok.begin;
  size_t end        = start + tok.length;
  auto   error_diag = Error_Diagnostic(scr_info.id, code, start, end, phase, msg, hint);

  compiler::COMPILER.add_error(std::move(error_diag));
}

void token::Viewer::add_error_tok(ErrorCode code, const Token& tok, std::string_view msg, std::string_view hint)
{
  size_t start      = tok.begin;
  size_t end        = start + tok.length;
  auto   error_diag = Error_Diagnostic(scr_info.id, code, start, end, phase, msg, hint);

  compiler::COMPILER.add_error(std::move(error_diag));
}

void token::Viewer::jump(size_t newPosition) noexcept
{
  auto& tokens = scr_info.file_info.tokens->tokens;

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
  auto& tokens = scr_info.file_info.tokens->tokens;

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

token::Token& token::Viewer::peek(int offset) const noexcept
{
  auto& tokens = scr_info.file_info.tokens->tokens;

  size_t index = current + offset;
  if (index < tokens.size()) return tokens[index];
  return tokens.back();
}

token::Token& token::Viewer::prev() noexcept
{
  auto& tokens = scr_info.file_info.tokens->tokens;

  if (current == 0) {
    return tokens[0];
  }
  current--;
  return tokens[current];
}

bool token::Viewer::is_end() const noexcept
{
  auto& tokens = scr_info.file_info.tokens->tokens;

  return current == tokens.size() - 1
         || scr_info.file_info.tokens->get(token::_id(current)).kind == token::ETokenKind::S_END_OF_FILE;
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
    next();
    return true;
  }
  return false;
}

bool token::Viewer::check_id_val(std::string_view val) const noexcept
{
  auto tok = peek();
  auto str = scr_info.file_info.tokens->audit.Token_to_str(tok.id);

  if (check(token::ETokenKind::IDENTIFIER)) {
    return str == val;
  }
  return false;
}

bool token::Viewer::check_val(std::string_view val) const noexcept
{
  auto tok = peek();
  auto str = scr_info.file_info.tokens->audit.Token_to_str(tok.id);

  return str == val;
}

std::string token::Viewer::match_any_val(const std::initializer_list<std::string>& val) noexcept
{
  auto tok = peek();
  auto str = scr_info.file_info.tokens->audit.Token_to_str(tok.id);

  for (auto& elem : val) {
    if (str == elem) {
      next();
      return elem;
    }
  }

  return "";
}

bool token::Viewer::match_val(std::string_view val) noexcept
{
  auto tok = peek();
  auto str = scr_info.file_info.tokens->audit.Token_to_str(tok.id);

  if (str == val) {
    next();
    return true;
  }
  return false;
}

bool token::Viewer::match_id_val(std::string_view val) noexcept
{
  if (check_id_val(val)) {
    next();
    return true;
  }
  return false;
}

bool token::Viewer::match_any(const std::initializer_list<token::ETokenKind>& types) noexcept
{
  for (ETokenKind type : types) {
    if (check(type)) {
      next();
      return true;
    }
  }
  return false;
}

size_t token::Viewer::size() const noexcept
{
  return scr_info.file_info.tokens->tokens.size();
}

size_t token::Viewer::position() const noexcept
{
  return current;
}

size_t token::Viewer::line() const noexcept
{
  auto line = scr_info.file_info.get_line_from_pos(peek().begin);

  return line;
}

// Go back to a know position
void token::Viewer::rewind(size_t pos) noexcept
{
  auto& tokens = scr_info.file_info.tokens->tokens;

  if (pos >= tokens.size()) pos = tokens.size() - 1;
  current = pos;
#ifdef DEBUG
  current_str   = tokens[pos].debug_val;
  current_token = tokens[pos].kind;
#endif
}

void token::Viewer::synchronize() noexcept
{
  while (!is_end()) {
    if (peek(-1).kind == token::ETokenKind::SEMICOLON) return;

    switch (peek().kind) {
    case token::ETokenKind::LET:
    case token::ETokenKind::VAR:
    case token::ETokenKind::ENTITY:
    case token::ETokenKind::METACODE:
    case token::ETokenKind::ENUM:
    case token::ETokenKind::IF:
    case token::ETokenKind::ELSE:
    case token::ETokenKind::WHILE:
    case token::ETokenKind::INJECT:
    case token::ETokenKind::FOR:
    case token::ETokenKind::RETURN:
    case token::ETokenKind::BREAK:
    case token::ETokenKind::CONTINUE:
    case token::ETokenKind::MATCH:
    case token::ETokenKind::CAST:
    case token::ETokenKind::FUNCTION:
    case token::ETokenKind::SYSTEM:
    case token::ETokenKind::OP:
    case token::ETokenKind::COMPONENT:
    case token::ETokenKind::IDENTIFIER: return; // end the function
    default:                            break;                             // continue
    }

    next(); // consume token and continue
  }
}

const token::Token& token::Viewer::get(size_t position)
{
  auto& tokens = scr_info.file_info.tokens->tokens;

  if (tokens.size() < position) auto val = tokens.back();
  return tokens[position];
}