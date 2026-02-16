#include "TokenViewer.hpp"

#include <system_error>

#include "ErrorOutput.hpp"
#include "ScriptInfo.hpp"

void TokenViewer::jump(size_t newPosition)
{
  if (newPosition < scr_info.tokens.size()) {
    current = newPosition;
  } else {
    current = scr_info.tokens.size() - 1;
  }

  currentTokStr = scr_info.tokens[current].val;
  currentTokTy  = scr_info.tokens[current].type;
  currentLine   = scr_info.tokens[current].span.line;
  if (currentLine - 1 >= scr_info.get_line_size()) currentLine = scr_info.get_line_size();
  currentLineStr = scr_info.get_line(currentLine);
}

Token TokenViewer::next()
{
  if (!is_end()) {
    auto pre_tok = scr_info.tokens[current];
    current++;
    auto post_tok = scr_info.tokens[current];
    currentTokStr = post_tok.val;
    currentTokTy  = post_tok.type;
    currentLine   = post_tok.span.line;
    if (currentLine - 1 >= scr_info.get_line_size()) currentLine = scr_info.get_line_size();
    currentLineStr = scr_info.get_line(currentLine);
    return pre_tok;
  }
  return scr_info.tokens.back();
}

Token TokenViewer::peek(int offset) const
{
  size_t index = current + offset;
  if (index < scr_info.tokens.size()) return scr_info.tokens[index];
  return scr_info.tokens.back();
}

Token TokenViewer::prev()
{
  if (current == 0) {
    return scr_info.tokens[0];
  }
  current--;
  return scr_info.tokens[current];
}

bool TokenViewer::is_end() const
{
  return current == scr_info.tokens.size() - 1 || scr_info.tokens[current].type == ETokenType::S_END_OF_FILE;
}

bool TokenViewer::look_ahead(TokTy check, TokTy terminaison)
{
  for (size_t offset = 0;; offset++) {
    TokTy type = peek(offset).type;
    if (type == check) return true;
    if (type == terminaison) return false;
    if (type == TokTy::S_END_OF_FILE) return false;
    offset++;
  }
}

// will ignore new line if not specified
bool TokenViewer::check(TokTy type)
{
  if (!is_end()) {
    return peek().type == type;
  }
  return false;
}

bool TokenViewer::check_any(const std::initializer_list<ETokenType> &types)
{
  for (ETokenType type : types) {
    if (check(type)) return true;
  }
  return false;
}

bool TokenViewer::match(TokTy type)
{
  if (check(type)) {
    next();
    return true;
  }
  return false;
}

bool TokenViewer::check_id_val(const std::string &val)
{
  if (check(TokTy::IDENTIFIER)) {
    return peek().val == val;
  }
  return false;
}

bool TokenViewer::check_val(const std::string &val) { return peek().val == val; }

std::string TokenViewer::match_any_val(const std::initializer_list<std::string> &val)
{
  const std::string tok = peek().val;
  for (auto &elem : val) {
    if (tok == elem) {
      next();
      return elem;
    }
  }

  return "";
}

bool TokenViewer::match_val(const std::string &val)
{
  if (peek().val == val) {
    next();
    return true;
  }
  return false;
}

bool TokenViewer::match_id_val(const std::string &val)
{
  if (check_id_val(val)) {
    next();
    return true;
  }
  return false;
}

bool TokenViewer::match_any(const std::initializer_list<ETokenType> &types)
{
  for (ETokenType type : types) {
    if (check(type)) {
      next();
      return true;
    }
  }
  return false;
}

size_t TokenViewer::position() const { return current; }

size_t TokenViewer::line() const { return peek().span.line; }

// Go back to a know position
void TokenViewer::rewind(size_t pos)
{
  if (pos >= scr_info.tokens.size()) pos = scr_info.tokens.size() - 1;
  current       = pos;
  currentTokStr = scr_info.tokens[pos].val;
  currentTokTy  = scr_info.tokens[pos].type;
  currentLine   = scr_info.tokens[pos].span.line;
  if (currentLine >= scr_info.get_line_size()) currentLine = scr_info.get_line_size() - 1;
  currentLineStr = scr_info.get_line(currentLine);
}

void TokenViewer::synchronize()
{
  while (!is_end()) {
    if (peek(-1).type == TokTy::SEMICOLON) return;

    switch (peek().type) {
      case TokTy::LET:
      case TokTy::VAR:
      case TokTy::ENTITY:
      case TokTy::METACODE:
      case TokTy::ENUM:
      case TokTy::IF:
      case TokTy::ELSE:
      case TokTy::WHILE:
      case TokTy::INJECT:
      case TokTy::FOR:
      case TokTy::RETURN:
      case TokTy::BREAK:
      case TokTy::CONTINUE:
      case TokTy::MATCH:
      case TokTy::CAST:
      case TokTy::FUNCTION:
      case TokTy::SYSTEM:
      case TokTy::OP:
      case TokTy::COMPONENT:
      case TokTy::IDENTIFIER:
        return; // end the function
      default:
        break; // continue
    }

    next(); // consume token and continue
  }
}

std::string TokenViewer::get_line_str_at(size_t line) const
{
  int offset = line - 1;
  if (scr_info.get_line_size() < offset) return "EOF";
  return scr_info.get_line(offset);
}

const Token &TokenViewer::get(size_t position)
{
  if (scr_info.tokens.size() < position) auto val = scr_info.tokens.back();
  return scr_info.tokens[position];
}