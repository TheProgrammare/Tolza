
#include "lexer.hpp"

#include "script_info.hpp"
#include "error_output.hpp"
#include "compiler_data.hpp"
#include "token.hpp"

Lexer::Lexer(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , stream(scr_info.file_str)
{
}

void Lexer::tokenize(const std::set<char>& exit_char)
{
  auto stop_guard = [&]() -> bool {
    while ((is_ctrl(ch) || is_space(ch)) && exit_char.find(ch) == exit_char.end()) {
      if (!eat()) return false;
    }
    if (ch == EOF) return true;
    if (exit_char.find(ch) != exit_char.end()) return true;
    return false;
  };

  while (eat()) {
    // check if must exit
    if (stop_guard()) return;

    // no char before keyword : clear all char control and spaces
    while ((is_ctrl(ch) || is_space(ch)) && eat()) {
    }

    // it's a literal string
    if (ch == '"' || ch == 'c' && stream.peek() == '"') {
      tokenize_textual();
      continue;
    }
    // it's a comment
    else if (ch == '/' && (stream.peek() == '/' || stream.peek() == '*')) {
      tokenize_comment();
      continue;
    }
    // it's a metacode instruction
    else if (ch == '#') {
      // handle multiple metacode in one line # static # const
      for (;;) {
        tokenize_metacode();
        if (ch != '#') break;
      }

      continue;
    }
    // it's a placeholder: [[Identifier]]
    else if (ch == '[' && stream.peek() == '[') {
      eat(); // consume [
      eat(); // consume [
      tokenize_identifier();

      if (stream.peek() == ']') {
        eat(); // consume ]
        if (stream.peek() == ']') {
          eat(); // consume ]
          addToken(TokTy::S_METACODE_PLACEHOLDER);
          continue;
        }
      }

      add_error(0, "Expected end of placeholder end ']]' after placeholder start '[['",
                "define placeholders in code like: `[[_U]]`");
    }
    // can be a numeric value or a range token (.. or ..=) or a variadic (...)
    else if (is_digit(ch) || ch == '.') {
      tokenize_numeric();
      continue;
    }
    // can be a identifier or keyword
    else if (is_alpha(ch) || ch == '_') {
      tokenize_identifier();

      // try to avoid tokenize_keyword who is expensive :(
      // no prefix possible: can be a keyword or identifier
      if (is_ctrl(stream.peek()) || is_space(stream.peek())) {
        // it's a keyword
        if (TokTy type = Str_to_ETokenType(buffer); type != TokTy::UNKNOWN) {
          addToken(type);
        }
        // it's a identifier
        else
          addToken(TokTy::IDENTIFIER);
      }
      // can be a keyword
      else if (is_valid_prefix(stream.peek(), buffer)) {
        tokenize_keyword();
      }
      // it's a keyword
      else if (TokTy type = Str_to_ETokenType(buffer); type != TokTy::UNKNOWN) {
        addToken(type);
      }
      // it's a identifier
      else {
        addToken(TokTy::IDENTIFIER);
      }
      continue;
    }
    // it's can be a keyword or identifier or an error
    else {
      tokenize_keyword();
    }
  }
  buffer.clear();
  addToken(TokTy::S_END_OF_FILE);
}

void Lexer::process_escape()
{
  // read next chracter after backslash
  if (!stream.get(ch)) {
    add_error(1, "Unexpected end of input after escape sequence.", "");
    return;
  }

  switch (ch) {
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
    for (int i = 0; i < 2; ++i) { // On lit 1 ou 2 chiffres hexadécimaux
      if (!stream.get(ch) || !is_hex(ch)) {
        if (hex.empty()) {
          add_error(2, "Invalid hex escape sequence", "define hex escape like: `\\xHH`");
          return;
        } else {
          // return partial char if only one digit is read
          break;
        }
      }
      hex.push_back(ch);
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
    int         numDigits = (ch == 'u') ? 4 : 8;
    std::string hex;
    for (int i = 0; i < numDigits; ++i) {
      if (!stream.get(ch) || !is_hex(ch)) {
        add_error(3, "Invalid Unicode escape sequence", "define unicode escape like: `\\uXXXX`");
        return;
      }
      hex.push_back(ch);
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
    add_error(6, "Unknown escape sequence: \\" + std::to_string(ch), "");
    buffer += ch;
    break;
  }
}

void Lexer::tokenize_textual()
{
  bool is_c_string = ch == 'c';
  if (ch == 'c') eat(); // consume c
  while (eat()) {
    if (ch == '{') {
      if (!buffer.empty()) addToken(TokTy::L_TEXTUAL);
      addToken(TokTy::S_TEXTUAL_EXPR_START);

      tokenize({'}', ':'});

      if (ch == ':') {
        buffer += ch;
        addToken(TokTy::COLON);
        eat(); // consume :
        while (tokenize_spec()) {
          eat();
        }
      }

      if (ch == '}') {
        addToken(TokTy::S_TEXTUAL_EXPR_END);
        eat(); // consume }
        if (ch == '"') {
          buffer.clear();
          return;
        }
      }
    }

    if (ch == '"') {
      addToken(is_c_string ? TokTy::L_C_STRING : TokTy::L_TEXTUAL);
      return;
    } else if (ch == '\\') {
      process_escape();
    }

    buffer += ch;
  }
}

bool Lexer::tokenize_spec()
{
  // Unsigned integrals
  if (is_digit(ch)) {
    buffer += ch;
    while (is_digit(stream.peek())) {
      eat();        // consume current
      buffer += ch; // save peeked digit
    }

    addToken(TokTy::L_U);
    return true;
  }
  // Letters
  else if (is_alpha(ch)) {
    buffer = ch;
    addToken(TokTy::L_ASCII);
    return true;
  }

  // Symbols and operators
  buffer = ch;
  switch (ch) {
  case '.':  addToken(TokTy::DOT); return true;
  case ',':  addToken(TokTy::COMMA); return true;
  case '_':  addToken(TokTy::UNDERSCORE); return true;
  case '\'': addToken(TokTy::TICK); return true;
  case '+':  addToken(TokTy::OP_PLUS); return true;
  case '-':  addToken(TokTy::OP_MINUS); return true;
  case '<':  addToken(TokTy::OPEN_BRACKETS); return true;
  case '>':  addToken(TokTy::CLOSE_BRACKETS); return true;
  case '^':  addToken(TokTy::OP_CIRCUMFLEX); return true;
  case '~':  addToken(TokTy::TILDE); return true;
  case '=':  addToken(TokTy::ASSIGN); return true;
  case '%':  addToken(TokTy::OP_MODULO); return true;
  case ' ':  addToken(TokTy::SPACE); return true;
  case '}':  return false;
  default:   {
    auto error = Error_Diagnostic(
        151, scr_info, Token("", ETokenType::NONE, Span(0, stream.get_line(), stream.get_column(), 1)), {},
        compiler::EPhase::lexer, EErrorSeverity::error, {}, "Unexpected format specifier character",
        "define format specifier like:"
        "\n  - right-aligned: `{val:>10}`\n  - 2 decimals `{val:.2f}`\n  - hexadecimal `{val:#x}`");

    errors.push_back(error.print_error());
  }
  }
  return false;
}

void Lexer::tokenize_comment()
{
  if (ch == '/' && stream.peek() == '/') {
    while (stream.peek() != EOF && stream.peek() != '\n') eat();
    return;
  }
  // Ignore block comments /* */
  else if (ch == '/' && stream.peek() == '*') {
    eat(); // consume '*'
    while (eat()) {
      if (ch == '*' && stream.peek() == '/') {
        eat(); /* consume '/' */
        return;
      }
    }
  }
}

void Lexer::tokenize_metacode()
{
  addToken(TokTy::METACODE);
  tokenize({'\n', '#'});
  addToken(TokTy::S_METACODE_END);
}

void Lexer::tokenize_numeric()
{
  bool isBin = false, isOct = false, isHex = false;
  bool isFloat = false, isDecimal = false, isuDecimal = false;

  auto check_range_case = [&]() -> bool {
    if (ch == '.' && stream.peek() == '.') {
      eat();
      // case range included
      if (stream.peek() == '=') {
        eat();
        buffer = "..=";
        addToken(TokTy::RANGE_INCLUSIVE);
      }
      // case variadic
      else if (stream.peek() == '.') {
        eat();
        buffer = "...";
        addToken(TokTy::VARIADIC);
      }
      // case range excluded
      else {
        buffer = "..";
        addToken(TokTy::RANGE);
      }
      return true;
    }
    return false;
  };

  if (ch == '0' && stream.peek() != EOF) {
    char nt = stream.peek();
    if (nt == 'b' || nt == 'B') {
      isBin = true;
      eat();
      buffer += ch;
    } else if (nt == 'o' || nt == 'O') {
      isOct = true;
      eat();
      buffer += ch;
    } else if (nt == 'x' || nt == 'X') {
      isHex = true;
      eat();
      buffer += ch;
    }
  }

  if (!isBin && !isOct && !isHex) {
    if (is_digit(ch))
      buffer += ch;
    else if (check_range_case())
      return;
    // start by dot . : can be float or member access
    else if (ch == '.') {
      // member access : a.b

      if (!scr_info.tokens.empty() && scr_info.tokens.back().type == TokTy::IDENTIFIER && is_alpha(stream.peek())) {
        buffer = ".";
        addToken(TokTy::DOT);
        return;
      }
      // floating value : 8. or 10.f or 3.14
      else {
        buffer  = '.';
        isFloat = true;
        if (stream.peek() == 'f' || stream.peek() == 'F') eat(); // consume .
      }
    }
  }

  while (eat()) {
    // Only allow 0 1 ' _
    if (isBin) {
      if (ch == '0' || ch == '1')
        buffer += ch;
      else if (ch == '\'' || ch == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 ' _
    else if (isOct) {
      if (ch >= '0' && ch <= '7')
        buffer += ch;
      else if (ch == '\'' || ch == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 8 9 A B C D E F ' _
    else if (isHex) {
      if (is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
        buffer += ch;
      else if (ch == '\'' || ch == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // numeric
    else {
      // classic numeric
      if (is_digit(ch)) {
        buffer += ch;
      }
      // prevent range creation : save buffer vals
      else if (ch == '.' && stream.peek() == '.') {
        if (!buffer.empty()) addToken(TokTy::L_I); // create literal integral
        check_range_case();                        // create range
        return;                                    // must stop after range creation
      } else if (ch == '.') {
        buffer += ch;
        isDecimal = true;
      }
      // floating numeric 0.0f
      else if (ch == 'f') {
        isFloat = true;
        break;
      }
      // floating numeric (scientific notation) 10000e+10 100e-15
      else if (ch == 'e' || ch == 'E') {
        buffer += ch;
        isFloat = true;
        // exponent sign
        if (stream.peek() == '+' || stream.peek() == '-') {
          eat();
          buffer += ch;
        }
      }
      // decimal numeric
      else if (ch == 'd') {
        isDecimal = true;
        break;
      }
      // udecimal numeric
      else if (ch == 'u' && stream.peek() == 'd') {
        isuDecimal = true;
        break;
      }
      // end
      else {
        stream.go_back();
        break;
      }
    }
  }

  if (isBin) return addToken(TokTy::L_BIN);
  if (isOct) return addToken(TokTy::L_OCT);
  if (isHex) return addToken(TokTy::L_HEX);
  if (isFloat) return addToken(TokTy::L_F);
  if (isDecimal) return addToken(TokTy::L_DECIMAL);
  if (isuDecimal) return addToken(TokTy::L_UDECIMAL);

  return addToken(TokTy::L_I);
}

Lexer::EPrefixFound Lexer::get_prefix_keyword(TokTy _type, std::string_view _key, std::string_view _search)
{
  if (_key == _search) return EPrefixFound::All;
  if (_search.size() < _key.size() && _key.rfind(_search, 0) == 0) return EPrefixFound::Prefix;
  return EPrefixFound::None;
}

const std::vector<std::pair<std::string_view, ETokenType>>& get_sorted_keywords()
{
  static std::vector<std::pair<std::string_view, ETokenType>> sorted;
  if (sorted.empty()) {
    sorted.reserve(kKeywords.size());
    for (auto& [text, type] : kKeywords) sorted.emplace_back(text, type);

    // Trie décroissant par taille pour matcher le mot clé le plus long en premier
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const auto& a, const auto& b) { return a.first.size() > b.first.size(); });
  }
  return sorted;
}

bool Lexer::is_valid_prefix(char prefix, std::string_view _current)
{
  for (auto& [val, type] : get_sorted_keywords()) {
    if (val.size() <= _current.size()) continue;
    // Manual prefix comparison
    if (val.substr(0, _current.size()) != _current) continue;
    if (val[_current.size()] == prefix) return true;
  }
  return false;
}

void Lexer::tokenize_keyword()
{
  struct Keyword {
    std::string text;
    ETokenType  type;
  };

  if (buffer.empty()) buffer = ch;

  bool keep_searching = true;

  while (keep_searching) {
    keep_searching = false;

    for (auto& [val, type] : get_sorted_keywords()) {
      if (val.empty() || val[0] != buffer[0]) continue;
      if (buffer.size() > val.size()) continue;

      switch (get_prefix_keyword(type, val, buffer)) {
      case EPrefixFound::None: continue;

      case EPrefixFound::All:
        buffer = std::string(val); // convertir string_view en string
        addToken(type);
        return;

      case EPrefixFound::Prefix: {
        char next = stream.peek();
        if (next == EOF || is_ctrl(next) || is_space(next)) {
          keep_searching = false;
          break;
        }

        if (is_valid_prefix(next, buffer)) {
          eat();
          buffer += next;
          if (buffer.size() > k_max_keyword_size) break;
          keep_searching = true;
        } else {
          // Backtracking optimisé : vider le buffer tout en remettant les chars
          while (!buffer.empty()) {
            if (str_is_identifier(buffer)) return;
            if (auto type2 = Str_to_ETokenType(buffer); type2 != TokTy::UNKNOWN) {
              addToken(type2);
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
    addToken(TokTy::IDENTIFIER);
  } else {
    add_error(7, "Unexpected token symbol",
              "define keywords like:"
              "\n  - Identifier: alpha or '_' first and after alphanumeric: [a-zA-Z_][a-zA-Z0-9_]"
              "\n  - Reserved keyword: please, refer to the language documentation.");
  }

  /*

  // if no start by id : buffer must have the current character
  if (buffer.empty()) buffer = ch;

  bool keep_searching = true;
  while (keep_searching) {
    keep_searching = false;

    for (auto& [val, type] : get_sorted_keywords()) {
      // Ignore keywords that can't possibly match the first character
      if (val.empty() || val[0] != buffer[0]) continue;
      if (buffer.size() > val.size()) continue;
      switch (get_prefix_keyword(type, val, buffer)) {
      case EPrefixFound::None: {
        continue;
      }
      case EPrefixFound::All: {
        buffer = val;
        addToken(type);
        return;
      }
      case EPrefixFound::Prefix: {
        char next = stream.peek();
        if (next == EOF || is_ctrl(next) || is_space(next)) {
          keep_searching = false;
          break;
        }

        // optimization
        if (is_valid_prefix(next, buffer)) {
          eat();
          buffer += next;
          if (buffer.size() > k_max_keyword_size) break;
          keep_searching = true;
        } else {
          while (buffer.size() > 0) {
            if (str_is_identifier(buffer)) return;
            if (TokTy type = Str_to_ETokenType(buffer); type != TokTy::UNKNOWN) {
              addToken(type);
              return;
            }
            char last = buffer.back();
            buffer.pop_back();
            stream.putback(last);
          }

          keep_searching = false;
        }
        break;
      }
      }
      if (keep_searching) break;
    }
  }

  // purge the buffer of control char
  std::string buffer_temp;
  for (char elem : buffer) {
    if (!is_ctrl(elem) && !is_space(elem)) buffer_temp += elem;
  }
  buffer = buffer_temp;

  if (buffer.empty()) return;

  // no corresponding keyword found : it's an identifier
  if (str_is_identifier(buffer)) {
    addToken(TokTy::IDENTIFIER);
  }
  // not identifier standard : error
  else {
    add_error(7, "Unexpected token symbol",
              "define keywords like:"
              "\n  - Identifier: alpha or '_' first and after alphanumeric: [a-zA-Z_][a-zA-Z0-9_]"
              "\n  - Reserved keyword: please, refer to the language documentation.");
  }
              */
}

void Lexer::tokenize_identifier()
{
  buffer += ch;
  bool keep_tokenize = true;
  while (keep_tokenize) {
    keep_tokenize = false;
    if (is_alnum(stream.peek()) || stream.peek() == '_') {
      eat();
      buffer += ch;
      keep_tokenize = true;
    }

    if (!keep_tokenize) break;
  }
}

void Lexer::addToken(TokTy type)
{
  Span span(0, stream.get_line(), stream.get_column(), buffer.size());
  span.anteprocess_pos = scr_info.tokens.size();
  scr_info.tokens.emplace_back(Token(buffer, type, span));
  buffer.clear();
}

bool Lexer::eat()
{
  if (stream.peek() == '\n' && !scr_info.tokens.empty()) scr_info.tokens.back().debug_end_of_line = true;
  if (stream.get(ch)) return true;
  return false;
}

void Lexer::add_error(ErrorCode code, const std::string& msg, const std::string& hint)
{
  Span span(0, stream.get_line(), stream.get_column(), buffer.size());
  span.anteprocess_pos = scr_info.tokens.size();
  auto tok             = Token(buffer, ETokenType::NONE, span);

  std::string out =
      Error_Diagnostic(code, scr_info, tok, {}, compiler::EPhase::lexer, EErrorSeverity::error, {}, msg, hint)
          .print_error();
  errors.push_back(out);
}

TokTy Lexer::classifyNumerals(std::string& outValue)
{
  outValue.clear();
  char c;

  while (stream.get(c) && is_space(c)) {
  }

  // Check if start of number
  if (!is_digit(c) && c != '.') {
    stream.go_back();
    return TokTy::UNKNOWN;
  }

  std::string buffer;
  bool        isBin = false, isOct = false, isHex = false;
  bool        isFloat = false, isDecimal = false, isuDecimal = false;

  buffer.push_back(c);

  // Check prefix bin/oct/hex
  if (c == '0' & stream.peek() != EOF) {
    char next = stream.peek();
    if (next == 'b' || next == 'B') {
      isBin = true;
      stream.get(c);
      buffer += c;
    } else if (next == 'o' || next == 'O') {
      isOct = true;
      stream.get(c);
      buffer += c;
    } else if (next == 'x' || next == 'X') {
      isHex = true;
      stream.get(c);
      buffer += c;
    }
  }

  while (stream.get(c)) {
    // Binary
    if (isBin) {
      if (c == '0' || c == '1')
        buffer += c;
      else if (c == '\'' || c == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Octal
    else if (isOct) {
      if (c >= '0' && c <= '7')
        buffer += c;
      else if (c == '\'' || c == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Hexadecimal
    else if (isHex) {
      if (is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
        buffer += c;
      else if (c == '\'' || c == '_')
        continue;
      else {
        stream.go_back();
        break;
      }
    }
    // Numeric / float / decimal
    else {
      if (is_digit(c))
        buffer += c;
      else if (c == '.' & stream.peek() != '.' && !isDecimal) {
        buffer += c;
        isDecimal = true;
      } else if (c == 'f') {
        isFloat = true;
        break;
      } else if (c == 'e' || c == 'E') {
        buffer += c;
        isFloat = true;
        if (stream.peek() == '+' || stream.peek() == '-') {
          stream.get(c);
          buffer += c;
        }
      } else if (c == 'd') {
        isDecimal = true;
        break;
      } else if (c == 'u' && stream.peek() == 'd') {
        isuDecimal = true;
        break;
      } else {
        stream.go_back();
        break;
      }
    }
  }

  outValue = buffer;

  if (isBin) return TokTy::L_BIN;
  if (isOct) return TokTy::L_OCT;
  if (isHex) return TokTy::L_HEX;
  if (isFloat) return TokTy::L_F;
  if (isDecimal) return TokTy::L_DECIMAL;
  if (isuDecimal) return TokTy::L_UDECIMAL;

  return TokTy::L_I; // standard integral
}

TokTy Lexer::classifyKeyword(std::string& outWord)
{
  char c = outWord[0];

  std::string buffer(1, c);
  while (stream.peek() != EOF && !is_space(stream.peek()) && !is_ctrl(stream.peek())) {
    stream.get(c);
    buffer += c;
  }

  for (size_t len = buffer.size(); len > 0; --len) {
    std::string candidate = buffer.substr(0, len);
    auto        it        = kKeywords.find(candidate);
    if (it != kKeywords.end()) {
      outWord = candidate;
      // Remettre les caractères restants
      for (int i = (int)buffer.size() - 1; i >= (int)len; --i) stream.go_back();
      return it->second;
    }
  }


  if (is_alpha(buffer[0]) || buffer[0] == '_') {
    size_t i = 1;
    while (i < buffer.size() && (is_alnum(buffer[i]) || buffer[i] == '_')) i++;

    outWord = buffer.substr(0, i);

    for (int j = (int)buffer.size() - 1; j >= (int)i; --j) stream.go_back();

    return TokTy::IDENTIFIER;
  }

  outWord = buffer;
  return TokTy::UNKNOWN;
}

TokTy Lexer::classifyFormatSpec(std::string& outFormat)
{
  outFormat.clear();
  char c = outFormat[0];

  // Unsigned integrals
  if (is_digit(c)) {
    outFormat += c;

    while (stream.get(c)) {
      // concat numbers
      if (is_digit(c))
        outFormat += c;
      else {
        stream.go_back();
        break;
      }
    }
    return TokTy::L_U;
  }
  // Letters
  else if (is_alpha(c)) {
    outFormat += c;
    return TokTy::L_ASCII;
  }

  // Symbols and operators;
  outFormat += c;
  switch (c) {
  case '.':  return TokTy::DOT;
  case ',':  return TokTy::COMMA;
  case '_':  return TokTy::UNDERSCORE;
  case '\'': return TokTy::TICK;
  case '+':  return TokTy::OP_PLUS;
  case '-':  return TokTy::OP_MINUS;
  case '<':  return TokTy::OPEN_BRACKETS;
  case '>':  return TokTy::CLOSE_BRACKETS;
  case '^':  return TokTy::OP_CIRCUMFLEX;
  case '~':  return TokTy::TILDE;
  case '=':  return TokTy::ASSIGN;
  case '%':  return TokTy::OP_MODULO;
  case ' ':  return TokTy::SPACE;
  default:   return TokTy::UNKNOWN;
  }
}
