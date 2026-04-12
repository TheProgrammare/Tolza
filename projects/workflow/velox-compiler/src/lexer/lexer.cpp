#include "lexer.hpp"

#include <llvm/ADT/StringMap.h>

#include <initializer_list>

#include "misc/script_info.hpp"
#include "misc/error_output.hpp"
#include "compiler/compiler.hpp"
#include "token.hpp"


Lexer::Lexer(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , stream(scr_info.file_str)
{
}

void Lexer::start_buffer()
{
  buffer_start_col  = stream.get_column();
  buffer_start_line = stream.get_line();
}

void Lexer::tokenize(const std::set<char>& exit_char)
{
  auto stop_guard = [&]() -> bool {
    while ((is_ctrl(stream.peek()) || is_space(stream.peek())) && exit_char.find(stream.peek()) == exit_char.end()) {
      if (!stream.next()) return false;
    }
    if (stream.is_end()) return true;
    if (exit_char.find(stream.peek()) != exit_char.end()) return true;
    return false;
  };

  while (!stream.is_end()) {
    // check if must exit
    if (stop_guard()) return;

    // no char before keyword : clear all char control and spaces
    while ((is_ctrl(stream.peek()) || is_space(stream.peek())) && stream.next()) {
    }

    // it's a literal string
    if (stream.check('"') || (stream.check('r') && (stream.peek(1) == '"'))
        || (stream.check('r') && stream.peek(1) == '#' && stream.peek(2) == '"')) {
      start_buffer();
      tokenize_textual();
      continue;
    }
    // it's a comment
    else if (stream.check('/') && (stream.peek(1) == '/' || stream.peek(1) == '*')) {
      tokenize_comment();
      continue;
    }
    // it's a metacode instruction
    else if (stream.check('#')) {
      // handle multiple metacode in one line # static # const
      while (!stream.is_end()) {
        tokenize_metacode();
        if (!stream.check('#')) break;
      }

      continue;
    }
    // it's a placeholder: [[Identifier]]
    else if (stream.check('[') && stream.peek(1) == '[') {
      start_buffer();

      stream.next(); // consume [
      stream.next(); // consume [
      tokenize_identifier();

      if (stream.peek(1) == ']') {
        stream.next(); // consume ]
        if (stream.peek(1) == ']') {
          stream.next(); // consume ]
          add_token(TokTy::S_METACODE_PLACEHOLDER);
          continue;
        }
      }

      add_error(0, "Expected end of placeholder end ']]' after placeholder start '[['",
                "define placeholders in code like: `[[_U]]`");
    }
    // can be a numeric value or a range token (.. or ..=) or a variadic (...)
    else if (is_digit(stream.peek()) || stream.check('.')) {
      start_buffer();

      tokenize_numeric();
      continue;
    }
    // can be a identifier or keyword
    else if (is_alpha(stream.peek()) || stream.check('_')) {
      start_buffer();

      tokenize_identifier();

      // try to avoid tokenize_keyword who is expensive :(
      // no prefix possible: can be a keyword or identifier
      if (is_ctrl(stream.peek(1)) || is_space(stream.peek(1))) {
        // it's a keyword
        if (TokTy type = Str_to_ETokenType(buffer); type != TokTy::UNKNOWN) {
          add_token(type);
        }
        // it's a identifier
        else
          add_token(TokTy::IDENTIFIER);
      }
      // can be a keyword
      else if (is_valid_prefix(stream.peek(1), buffer)) {
        tokenize_keyword();
      }
      // it's a keyword
      else if (TokTy type = Str_to_ETokenType(buffer); type != TokTy::UNKNOWN) {
        add_token(type);
      }
      // it's a identifier
      else {
        add_token(TokTy::IDENTIFIER);
      }
      continue;
    }
    // it's can be a keyword or identifier or an error
    else {
      tokenize_keyword();
    }
  }

  buffer.clear();
  if (exit_char.empty()) add_token(TokTy::S_END_OF_FILE, true);
}

void Lexer::process_escape()
{
  // read next chracter after backslash
  if (!stream.next()) {
    add_error(1, "Unexpected end of input after escape sequence.", "");
    return;
  }

  switch (stream.peek()) {
  case 'n':  buffer += '\n'; break;
  case 't':  buffer += '\t'; break;
  case 'r':  buffer += '\r'; break;
  case '\\': buffer += '\\'; break;
  case '\'': buffer += '\''; break;
  case '"':  buffer += '"'; break;
  case '0':  buffer += '\0'; break;
  case 'a':  buffer += '\a'; break;
  case 'b':  buffer += '\b'; break;
  case 'f':  buffer += '\f'; break;
  case 'v':  buffer += '\v'; break;

  // hex sequence \xHH
  case 'x':  {
    std::string hex;
    for (int i = 0; i < 2; ++i) { // read 1 or 2 hex
      if (!stream.next() || !is_hex(stream.peek())) {
        if (hex.empty()) {
          add_error(2, "Invalid hex escape sequence", "define hex escape like: `\\xHH`");
          return;
        } else {
          // return partial char if only one digit is read
          break;
        }
      }
      hex.push_back(stream.peek());
    }
    try {
      char value = static_cast<char>(std::stoul(hex, nullptr, 16));
      buffer += value;
    } catch (...) {
      add_error(3, "Invalid hex escape value", "define hex escape like: `\\xHH`");
    }
    break;
  }

  // Unicode sequence \uXXXX or \UXXXXXXXX
  case 'u':
  case 'U': {
    int         numDigits = (stream.check('u')) ? 4 : 8;
    std::string hex;
    for (int i = 0; i < numDigits; ++i) {
      if (!stream.next() || !is_hex(stream.peek())) {
        add_error(3, "Invalid Unicode escape sequence", "define unicode escape like: `\\uXXXX`");
        return;
      }
      hex.push_back(stream.peek());
    }
    try {
      char32_t codepoint = std::stoul(hex, nullptr, 16);
      // basic convertion UTF-32 -> UTF-8
      if (codepoint <= 0x7F)
        buffer += static_cast<char>(codepoint);
      else if (codepoint <= 0x7FF) {
        buffer += static_cast<char>(0xC0 | (codepoint >> 6));
        buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
      } else if (codepoint <= 0xFFFF) {
        buffer += static_cast<char>(0xE0 | (codepoint >> 12));
        buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
      } else if (codepoint <= 0x10FFFF) {
        buffer += static_cast<char>(0xF0 | (codepoint >> 18));
        buffer += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        buffer += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        buffer += static_cast<char>(0x80 | (codepoint & 0x3F));
      } else {
        add_error(4, "Unicode codepoint out of range (" + std::to_string(codepoint) + ")", "");
      }
    } catch (...) {
      add_error(5, "Invalid Unicode escape value", "");
    }
    break;
  }

  default:
    // escape char unknown : keep literally or ring the error
    add_error(6, "Unknown escape sequence: \\" + std::to_string(stream.peek()), "");
    buffer += stream.peek();
    break;
  }
}

void Lexer::tokenize_textual()
{
  auto tok_text = [&](bool with_escape, bool with_interpolation, std::vector<char> end_tokens) {
    do {
      if (with_interpolation) {

        if (stream.check('{')) {
          if (!buffer.empty()) add_token(TokTy::L_TEXTUAL);
          start_buffer();
          add_token(TokTy::S_INTERPOLATION_START);

          start_buffer();
          tokenize({'}', ':'});

          if (stream.check(':')) {
            start_buffer();
            buffer += stream.peek();
            add_token(TokTy::COLON);
            while (tokenize_spec()) {
              stream.next();
            }
          }

          if (stream.check('}')) {
            add_token(TokTy::S_INTERPOLATION_END);
            if (stream.check('"')) {
              buffer.clear();
              return;
            }
          }
        }
      }

      if (with_escape) {
        if (stream.check('\\')) {
          process_escape();
          continue;
        }
      }

      if (end_tokens.empty()) {
        if (stream.check('"')) {
          add_token(TokTy::L_TEXTUAL);
          return;
        }
      } else if (end_tokens.size() == 1) {
        if (stream.check(end_tokens[0])) {
          add_token(TokTy::L_TEXTUAL);
          return;
        }
      } else {
        bool is_ended = true;
        for (size_t i = 0; i < end_tokens.size(); i++) {
          auto elem = end_tokens[i];
          if (stream.peek(i) != elem) {
            is_ended = false;
            break;
          }
        }
        if (is_ended) {
          for (size_t i = 0; i < end_tokens.size(); i++) {
            stream.next(); // consume end token
          }
          add_token(TokTy::L_TEXTUAL, true);
          return;
        }
      }

      buffer += stream.peek();
    } while (stream.next());
  };

  if (stream.peek(0) == '"' && stream.peek(1) == '"' && stream.peek(2) == '"') {
    stream.next(); // consume "
    stream.next(); // consume "
    stream.next(); // consume "
    if (!stream.match('\n')) add_error(229, "Expected new line after a literal", "");


    tok_text(true, true, {'\n', '"', '"', '"'});
  } else if (stream.peek(0) == 'r' && stream.peek(1) == '"' && stream.peek(2) == '"' && stream.peek(3) == '"') {
    stream.next(); // consume r
    stream.next(); // consume "
    stream.next(); // consume "
    stream.next(); // consume "
    if (!stream.match('\n')) add_error(229, "Expected new line after a literal", "");

    tok_text(false, true, {'\n', '"', '"', '"'});
  } else if (stream.peek(0) == 'r' && stream.peek(1) == '#' && stream.peek(2) == '"') {
    stream.next(); // consume r
    stream.next(); // consume #
    stream.next(); // consume "

    tok_text(false, true, {'#', '"'});
  } else {
    stream.next(); // consume "
    tok_text(true, true, {});
  }
}

bool Lexer::tokenize_spec()
{
  // Unsigned integrals
  if (is_digit(stream.peek())) {
    start_buffer();

    buffer += stream.peek();
    while (is_digit(stream.peek(1))) {
      stream.next();           // consume current
      buffer += stream.peek(); // save peeked digit
    }

    add_token(TokTy::L_I);
    return true;
  }
  // Letters
  else if (is_alpha(stream.peek())) {
    start_buffer();

    buffer = stream.peek();
    add_token(TokTy::L_CUNE);
    return true;
  }

  // Symbols and operators
  start_buffer();
  buffer = stream.peek();
  switch (stream.peek()) {
  case '.':  add_token(TokTy::DOT); return true;
  case ',':  add_token(TokTy::COMMA); return true;
  case '_':  add_token(TokTy::UNDERSCORE); return true;
  case '\'': add_token(TokTy::TICK); return true;
  case '+':  add_token(TokTy::OP_PLUS); return true;
  case '-':  add_token(TokTy::OP_MINUS); return true;
  case '<':  add_token(TokTy::OPEN_BRACKETS); return true;
  case '>':  add_token(TokTy::CLOSE_BRACKETS); return true;
  case '^':  add_token(TokTy::OP_CIRCUMFLEX); return true;
  case '~':  add_token(TokTy::TILDE); return true;
  case '=':  add_token(TokTy::ASSIGN); return true;
  case '%':  add_token(TokTy::OP_MODULO); return true;
  case ' ':  add_token(TokTy::SPACE); return true;
  case '}':  return false;
  default:   {
    auto error = Error_Diagnostic(
        scr_info, 151, &scr_info, Token("", ETokenType::NONE, Span(0, stream.get_line(), stream.get_column(), 1)),
        compiler::EPhase::lexer, "Unexpected format specifier character",
        "define format specifier like:"
        "\n  - right-aligned: `{val:>10}`\n  - 2 decimals `{val:.2f}`\n  - hexadecimal `{val:#x}`");

    errors.push_back(error.print_error());
  }
  }
  return false;
}

void Lexer::tokenize_comment()
{
  if (stream.check('/') && stream.peek(1) == '/') {
    while (stream.next() && stream.peek() != '\n') {
    }
    return;
  }
  // Ignore block comments /* */
  else if (stream.check('/') && stream.peek(1) == '*') {
    stream.next(); // consume '*'
    while (stream.next()) {
      if (stream.check('*') && stream.peek(1) == '/') {
        stream.next(); /* consume '/' */
        return;
      }
    }
  }
}

void Lexer::tokenize_metacode()
{
  add_token(TokTy::METACODE);
  tokenize({'\n', '#'});
  add_token(TokTy::S_METACODE_END, true);
}

void Lexer::tokenize_numeric()
{
  bool is_bin = false, is_oct = false, is_hex = false;
  bool id_decimal = false;

  auto check_range_case = [&]() -> bool {
    if (stream.check('.') && stream.peek(1) == '.') {
      // case range included
      if (stream.peek(2) == '=') {
        stream.jump(stream.position() + 3);
        buffer = "..=";
        add_token(TokTy::RANGE_INCLUSIVE, true);
      }
      // case variadic
      else if (stream.peek(2) == '.') {
        stream.jump(stream.position() + 3);
        buffer = "...";
        add_token(TokTy::VARIADIC, true);
      }
      // case range excluded
      else {
        stream.jump(stream.position() + 2);
        buffer = "..";
        add_token(TokTy::RANGE, true);
      }
      return true;
    }
    return false;
  };

  if (stream.check('0') && stream.peek(1) != EOF) {
    char nt = stream.peek(1);
    if (nt == 'b' || nt == 'B') {
      is_bin = true;
      stream.next();
      buffer += stream.peek();
    } else if (nt == 'o' || nt == 'O') {
      is_oct = true;
      stream.next();
      buffer += stream.peek();
    } else if (nt == 'x' || nt == 'X') {
      is_hex = true;
      stream.next();
      buffer += stream.peek();
    }
  }

  if (!is_bin && !is_oct && !is_hex) {
    if (is_digit(stream.peek()))
      buffer += stream.peek();
    else if (check_range_case())
      return;
    // start by dot . : can be float or member access
    else if (stream.check('.')) {
      // member access : a.b

      if (!scr_info.tokens.empty() && scr_info.tokens.back().type == TokTy::IDENTIFIER && is_alpha(stream.peek(1))) {
        buffer = ".";
        add_token(TokTy::DOT);
        return;
      }
      // floating value : 8. or 10.f or 3.14
      else {
        buffer     = '.';
        id_decimal = true;
        if (stream.peek(1) == 'f' || stream.peek(1) == 'F') stream.next(); // consume .
      }
    }
  }

  while (stream.next()) {
    // Only allow 0 1 ' _
    if (is_bin) {
      if (stream.check('0') || stream.check('1'))
        buffer += stream.peek();
      else if (stream.check('\'') || stream.check('_'))
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 ' _
    else if (is_oct) {
      if (stream.peek() >= '0' && stream.peek() <= '7')
        buffer += stream.peek();
      else if (stream.check('\'') || stream.check('_'))
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 8 9 A B C D E F ' _
    else if (is_hex) {
      if (is_digit(stream.peek()) || (stream.peek() >= 'a' && stream.peek() <= 'f')
          || (stream.peek() >= 'A' && stream.peek() <= 'F'))
        buffer += stream.peek();
      else if (stream.check('\'') || stream.check('_'))
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // numeric
    else {
      // classic numeric
      if (is_digit(stream.peek())) {
        buffer += stream.peek();
      }
      // prevent range creation : save buffer vals
      else if (stream.check('.') && stream.peek(1) == '.') {
        if (!buffer.empty()) add_token(TokTy::L_I, true); // create literal integral
        check_range_case();                               // create range
        return;                                           // must stop after range creation
      } else if (stream.check('.')) {
        buffer += stream.peek();
        id_decimal = true;
      }
      // floating numeric (scientific notation) 10000e+10 100e-15
      else if (stream.check('e') || stream.check('E')) {
        buffer += stream.peek();
        id_decimal = true;
        // exponent sign
        if (stream.peek(1) == '+' || stream.peek(1) == '-') {
          stream.next();
          buffer += stream.peek();
        }
      }
      // end
      else {
        stream.go_back();
        break;
      }
    }
  }

  if (is_bin) return add_token(TokTy::L_BIN);
  if (is_oct) return add_token(TokTy::L_OCT);
  if (is_hex) return add_token(TokTy::L_HEX);
  if (id_decimal) return add_token(TokTy::L_D);

  return add_token(TokTy::L_I);
}

Lexer::EPrefixFound Lexer::get_prefix_keyword(TokTy p_type, std::string_view p_key, std::string_view p_search)
{
  if (p_key == p_search) return EPrefixFound::All;
  if (p_search.size() < p_key.size() && p_key.rfind(p_search, 0) == 0) return EPrefixFound::Prefix;
  return EPrefixFound::None;
}

const std::vector<std::pair<std::string_view, ETokenType>>& get_sorted_keywords()
{
  static std::vector<std::pair<std::string_view, ETokenType>> sorted;
  if (sorted.empty()) {
    sorted.reserve(k_keywords.size());
    for (auto& [text, type] : k_keywords) sorted.emplace_back(text, type);

    // Trie décroissant par taille pour matcher le mot clé le plus long en premier
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });
  }
  return sorted;
}

bool Lexer::is_valid_prefix(char p_prefix, std::string_view p_current)
{
  for (auto& [val, type] : get_sorted_keywords()) {
    if (val.size() <= p_current.size()) continue;
    // Manual prefix comparison
    if (val.substr(0, p_current.size()) != p_current) continue;
    if (val[p_current.size()] == p_prefix) return true;
  }
  return false;
}

void Lexer::tokenize_keyword()
{
  struct Keyword {
    std::string text;
    ETokenType  type;
  };

  if (buffer.empty()) buffer = stream.peek();

  bool keep_searching = true;

  while (keep_searching) {
    keep_searching = false;

    for (auto& [val, type] : get_sorted_keywords()) {
      if (val.empty() || val[0] != buffer[0]) continue;
      if (buffer.size() > val.size()) continue;

      switch (get_prefix_keyword(type, val, buffer)) {
      case EPrefixFound::None: continue;

      case EPrefixFound::All:
        buffer = std::string(val);
        add_token(type);
        return;

      case EPrefixFound::Prefix: {
        char next = stream.peek(1);
        if (next == EOF || is_ctrl(next) || is_space(next)) {
          keep_searching = false;
          break;
        }

        if (is_valid_prefix(next, buffer)) {
          stream.next();
          buffer += next;
          if (buffer.size() > k_max_keyword_size) break;
          keep_searching = true;
        } else {
          // Backtracking optimisé : vider le buffer tout en remettant les chars
          while (!buffer.empty()) {
            if (str_is_identifier(buffer)) return;
            if (auto type2 = Str_to_ETokenType(buffer); type2 != TokTy::UNKNOWN) {
              add_token(type2);
              return;
            }
            buffer.pop_back();
            stream.go_back();
          }
          keep_searching = false;
        }
        break;
      }
      }

      if (keep_searching) break;
    }
  }

  // Nettoyage du buffer : remove_if + erase pour éviter copie
  buffer.erase(std::remove_if(buffer.begin(), buffer.end(), [](char c) { return is_ctrl(c) || is_space(c); }),
               buffer.end());

  if (buffer.empty()) return;

  if (str_is_identifier(buffer)) {
    add_token(TokTy::IDENTIFIER);
  } else {
    add_error(7, "Unexpected token symbol",
              "define keywords like:"
              "\n  - Identifier: alpha or '_' first and after alphanumeric: [a-zA-Z_][a-zA-Z0-9_]"
              "\n  - Reserved keyword: please, refer to the language documentation.");
  }
}

void Lexer::tokenize_identifier()
{
  buffer += stream.peek();
  while (!stream.is_end()) {
    if (is_alnum(stream.peek(1)) || stream.peek(1) == '_') {
      stream.next();
      buffer += stream.peek();
      continue;
    }

    break;
  }
}

void Lexer::add_token(TokTy type, bool do_not_move)
{
  Span span(0, stream.get_line(), stream.get_column(), buffer.size());
  span.anteprocess_pos = scr_info.tokens.size();
  scr_info.tokens.emplace_back(Token(buffer, type, span));
  buffer.clear();
  if (!do_not_move) stream.next();
}

void Lexer::add_error(ErrorCode code, const std::string& msg, const std::string& hint)
{
  Span span(0, stream.get_line(), stream.get_column(), buffer.size());
  span.anteprocess_pos = scr_info.tokens.size();
  auto tok             = Token(buffer, ETokenType::NONE, span);

  std::string out = Error_Diagnostic(scr_info, code, &scr_info, tok, compiler::EPhase::lexer, msg, hint).print_error();
  errors.push_back(out);
}
