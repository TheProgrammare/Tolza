#include "lexer.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "id/tokid.hpp"
#include "lexer/data.hpp"
#include "lexer/pool.hpp"
#include "misc/error_output.hpp"

#include <cassert>
#include <common/utils.hpp>
#include <cstddef>
#include <cstdio>
#include <initializer_list>
#include <queue>
#include <set>
#include <string_view>
#include <vector>


Lexer::DFANode::DFANode()
  : kind(token::ETokenKind::NONE)
{
}


constexpr Lexer::DFA Lexer::build_DFA()
{
  auto* root = new DFANode();

  // 1. Build trie
  for (const auto& [txt, kind] : token::k_DFA) {
    DFANode* n = root;
    for (char c : txt) {
      if (!n->next[c]) n->next[c] = new DFANode();
      n = n->next[c];
    }
    n->kind = kind;
  }

  // 2. Assign states (BFS)
  std::vector<DFANode*> nodes;
  std::queue<DFANode*>  q;

  root->id = 0;
  q.push(root);
  nodes.emplace_back(root);

  while (!q.empty()) {
    DFANode* n = q.front();
    q.pop();

    for (const auto& [c, nxt] : n->next) {
      if (nxt->id == DFA_INVALID) {
        nxt->id = nodes.size();
        nodes.emplace_back(nxt);
        q.push(nxt);
      }
    }
  }

  // 3. Build tables
  DFA dfa;
  dfa.transition.resize(nodes.size());
  dfa.accept.resize(nodes.size(), token::ETokenKind::NONE);

  for (auto* n : nodes) {
    auto& row = dfa.transition[n->id];
    row.fill(DFA_INVALID);

    for (const auto& [c, nxt] : n->next) {
      row[(unsigned char)c] = nxt->id;
    }

    dfa.accept[n->id] = n->kind;
  }

  return dfa;
}


Lexer::Lexer(cu::CU& _CU)
  : CU(_CU)
  , stream(CU.file_info.data)
{
}

void Lexer::start_buffer() noexcept
{
  buffer_start_pos = stream.position();
}

std::string_view Lexer::get_buffer_str() const noexcept
{
  assert(buffer_start_pos <= stream.position());
  size_t length = stream.position() - buffer_start_pos + 1;
  return stream.data().substr(buffer_start_pos, length);
}

bool Lexer::is_buffer_empty() const noexcept
{
  return buffer_start_pos > stream.position();
}


bool Lexer::tokenize(const std::set<char>& exit_char) noexcept
{
  auto stop_guard = [&]() -> bool {
    while ((common::utils::is_ctrl(stream.peek()) || common::utils::is_space(stream.peek()))
           && exit_char.find(stream.peek()) == exit_char.end()) {
      if (!stream.next()) return false;
    }
    if (stream.is_end()) return true;
    if (exit_char.find(stream.peek()) != exit_char.end()) return true;
    return false;
  };

  while (!stream.is_end()) {
    // check if must exit
    if (stop_guard()) break;

    // no char before keyword : clear all char control and spaces
    while ((common::utils::is_ctrl(stream.peek()) || common::utils::is_space(stream.peek())) && stream.next()) {
    }

    start_buffer();

    static size_t last_op = -1;
    // infinitive loop
    assert(last_op != stream.position());
    last_op = stream.position();

    // it's a literal string
    if (stream.check('"') || stream.check('`') || (stream.check('r') && (stream.peek(1) == '"'))
        || (stream.check('r') && stream.peek(1) == '`')) {
      start_buffer();
      tokenize_textual();
      continue;
    }

    // it's a comment
    if (stream.check('/') && (stream.peek(1) == '/' || stream.peek(1) == '*')) {
      tokenize_comment();
      continue;
    }

    // it's a metacode instruction
    if (stream.check('#')) {
      // handle multiple metacode in one line # static # const
      while (!stream.is_end()) {
        tokenize_metacode();
        if (!stream.check('#')) break;
      }

      continue;
    }

    // it's a placeholder: [[Identifier]]
    if (stream.check('[') && stream.peek(1) == '[') {
      start_buffer();

      (void)stream.next(); // consume [
      (void)stream.next(); // consume [
      read_identifier();
      auto a = 0;

      if (stream.peek(1) == ']') {
        (void)stream.next(); // consume ]
        if (stream.peek(1) == ']') {
          (void)stream.next(); // consume ]
          add_token(token::ETokenKind::S_METACODE_PLACEHOLDER);
          continue;
        }
      }

      add_error(0, "Expected end of placeholder end ']]' after placeholder start '[['",
                "define placeholders in code like: `[[_U]]`");
    } else if (tokenize_DFA()) {
      continue;
    }
    // can be a numeric value or a range token (.. or ..=) or a variadic (...)

    if (common::utils::is_digit(stream.peek())) {
      start_buffer();

      tokenize_numeric();
      continue;
    }

    (void)tokenize_keyword_identifier();
  }

  if (exit_char.empty()) add_token(token::ETokenKind::S_END_OF_FILE, true);

  return true;
}


bool Lexer::tokenize_DFA() noexcept
{
  start_buffer();
  static DFA dfa = build_DFA();

  DFA_State state             = 0;
  DFA_State last_accept_state = DFA_INVALID;

  size_t start           = stream.position();
  size_t last_accept_pos = start;

  while (true) {
    unsigned char c = stream.peek();

    DFA_State next = dfa.transition[state][c];
    if (next == DFA_INVALID) break;

    state = next;

    if (dfa.accept[state] != token::ETokenKind::NONE) {
      last_accept_state = state;
      last_accept_pos   = stream.position();
    }

    if (!stream.next()) break;
  }

  if (last_accept_state == DFA_INVALID) {
    (void)stream.jump(start);
    return false;
  }

  // if is underscore but it's the first character of identifier
  if (dfa.accept[last_accept_state] == token::ETokenKind::UNDERSCORE) {
    const char next_c = stream.at(last_accept_pos);
    if (common::utils::is_alnum(next_c) || next_c == '_') {
      (void)stream.jump(start);
      return false;
    }
  }

  (void)stream.jump(last_accept_pos);
  add_token(dfa.accept[last_accept_state]);
  return true;
}


void Lexer::process_escape() noexcept
{
  (void)stream.next(); // consume '\'

  char c = stream.peek();
  if (stream.peek(1) == EOF) {
    add_error(6, "Incomplete escape sequence", "");
    return;
  }

  switch (c) {
  case 'n':
  case 't':
  case 'r':
  case '\\':
  case '"':
  case '\'':
  case '0':
  case 'a':
  case 'b':
  case 'f':
  case 'v':  break;
  case 'x':  {
    unsigned char value = 0;
    for (int i = 0; i < 2; ++i) {
      if (!stream.next() || !common::utils::is_hex(stream.peek())) {
        add_error(2, "Invalid hex escape", "");
        return;
      }
      int v = common::utils::hex_value(stream.peek());
      if (v < 0) {
        add_error(2, "Invalid hex digit", "");
        return;
      }
    }
    return;
  }

  default: add_error(6, "Unknown escape sequence", ""); return;
  }
}

void Lexer::tokenize_textual() noexcept
{
  auto tokenize_literal = [&](bool with_escape, bool with_interpolation, char end_token, bool multiline) {
    start_buffer();

    while (true) {
      // -----------------------------------------------------------------
      // End of literal
      // -----------------------------------------------------------------
      if (!multiline) {
        if (stream.check_at(1, end_token)) {
          add_token(token::ETokenKind::L_TEXTUAL);

          (void)stream.next();

          // Empty literal.
          if (is_buffer_empty()) CU.file_info.tokens->tokens.back().length = 0;

          return;
        }
      } else {
        if (stream.check_chain({'\n', '"', '"', '"'})) {
          add_token(token::ETokenKind::L_TEXTUAL, true);

          for (size_t i = 0; i < 4; ++i) (void)stream.next();

          return;
        }
        if (stream.check_chain({'"', '"', '"'})) {
          add_error(229, "Expected new line after the end of multiline literal", "");
          return;
        }
      }

      // -----------------------------------------------------------------
      // Interpolation: ${expression[:spec]}
      // -----------------------------------------------------------------
      if (with_interpolation && stream.check_chain({'$', '{'})) {
        if (!is_buffer_empty()) add_token(token::ETokenKind::L_TEXTUAL);

        start_buffer();
        add_token(token::ETokenKind::S_INTERPOLATION_START);

        start_buffer();
        (void)tokenize({'}', ':'});

        if (stream.check(':')) {
          start_buffer();
          add_token(token::ETokenKind::COLON);

          while (tokenize_spec()) (void)stream.next();
        }

        if (stream.check('}')) add_token(token::ETokenKind::S_INTERPOLATION_END);

        continue;
      }

      // -----------------------------------------------------------------
      // Escape sequence
      // -----------------------------------------------------------------
      if (with_escape && stream.check('\\')) {
        process_escape();
        continue;
      }

      // -----------------------------------------------------------------
      // Consume current character.
      // -----------------------------------------------------------------
      if (!stream.next()) {
        add_error(229, "Unterminated literal string", "");
        return;
      }
    }
  };


  // =====================================================================
  // Multiline literal
  // =====================================================================
  if (stream.match_chain({'"', '"', '"'})) {
    if (!stream.match('\n')) {
      add_error(229, "Expected new line after a literal", "");
      return;
    }

    tokenize_literal(true, true, '"', true);
    return;
  }


  // =====================================================================
  // Raw multiline literal
  // =====================================================================
  if (stream.match_chain({'r', '"', '"', '"'})) {
    if (!stream.match('\n')) {
      add_error(229, "Expected new line after a literal", "");
      return;
    }

    tokenize_literal(false, true, '"', true);
    return;
  }


  // =====================================================================
  // Raw string with double-quote delimiter
  //
  // r"C:\my\path"
  // =====================================================================
  if (stream.match_chain({'r', '"'})) {
    tokenize_literal(false, true, '"', false);
    return;
  }


  // =====================================================================
  // Raw string with backtick delimiter
  //
  // r`windows_command "C:\my\specific path"`
  // =====================================================================
  if (stream.match_chain({'r', '`'})) {
    tokenize_literal(false, true, '`', false);
    return;
  }


  // =====================================================================
  // String with backtick delimiter
  //
  // `hello "world"`
  // =====================================================================
  if (stream.match('`')) {
    tokenize_literal(true, true, '`', false);
    return;
  }


  // =====================================================================
  // Normal string
  //
  // "hello"
  // =====================================================================
  if (stream.match('"')) {
    tokenize_literal(true, true, '"', false);
    return;
  }
}

bool Lexer::tokenize_spec() noexcept
{
  // Unsigned integrals
  if (common::utils::is_digit(stream.peek())) {
    start_buffer();

    while (common::utils::is_digit(stream.peek(1))) {
      (void)stream.next(); // consume current
    }

    add_token(token::ETokenKind::L_I);
    return true;
  }

  // Letters
  if (common::utils::is_alpha(stream.peek())) {
    start_buffer();

    add_token(token::ETokenKind::L_CUNE);
    return true;
  }

  // Symbols and operators
  start_buffer();
  switch (stream.peek()) {
  case '.':  add_token(token::ETokenKind::DOT); return true;
  case ',':  add_token(token::ETokenKind::COMMA); return true;
  case '_':  add_token(token::ETokenKind::UNDERSCORE); return true;
  case '\'': add_token(token::ETokenKind::TICK); return true;
  case '+':  add_token(token::ETokenKind::OP_PLUS); return true;
  case '-':  add_token(token::ETokenKind::OP_MINUS); return true;
  case '<':  add_token(token::ETokenKind::L_ANGLE); return true;
  case '>':  add_token(token::ETokenKind::R_ANGLE); return true;
  case '^':  add_token(token::ETokenKind::OP_CIRCUMFLEX); return true;
  case '~':  add_token(token::ETokenKind::TILDE); return true;
  case '=':  add_token(token::ETokenKind::ASSIGN); return true;
  case '%':  add_token(token::ETokenKind::OP_MODULO); return true;
  case ' ':  add_token(token::ETokenKind::SPACE); return true;
  case '}':  return false;
  default:   {
    auto error =
        Error_Diagnostic(CU.cuid, 151, buffer_start_pos, stream.position(), compiler::EPhase::lexer,
                         "Unexpected format specifier character",
                         "define format specifier like:"
                         "\n  - right-aligned: `{val:>10}`\n  - 2 decimals `{val:.2f}`\n  - hexadecimal `{val:#x}`");

    COMPILER.add_error(error);
  }
  }
  return false;
}

void Lexer::tokenize_comment() noexcept
{
  if (stream.match_chain({'/', '/'})) {
    while (stream.next() && stream.peek() != '\n') {
    }
    return;
  }

  // Ignore block comments /* */
  if (stream.match_chain({'/', '*'})) {
    while (stream.next()) {
      if (stream.match_chain({'*', '/'})) return;
    }
  }
}

void Lexer::tokenize_metacode() noexcept
{
  add_token(token::ETokenKind::METACODE);
  (void)tokenize({'\n', '#'});
  start_buffer();
  add_token(token::ETokenKind::S_METACODE_END, true);
}

void Lexer::tokenize_numeric() noexcept
{
  bool is_bin     = false;
  bool is_oct     = false;
  bool is_hex     = false;
  bool id_decimal = false;

  auto check_range_case = [&]() -> bool {
    if (!is_buffer_empty()) add_token(token::ETokenKind::L_I, true); // create literal integral

    if (stream.peek(1) == '.' && stream.peek(2) == '.') {
      (void)stream.next(); // consume last
      start_buffer();
      (void)stream.next(); // consume .

      // case range included
      if (stream.peek(1) == '=') {
        (void)stream.next(); // consume =
        add_token(token::ETokenKind::RANGE_INCLUSIVE);
      }
      // case variadic
      else if (stream.peek(1) == '.') {
        (void)stream.next(); // consume .
        add_token(token::ETokenKind::VARIADIC);
      }
      // case range excluded
      else {
        add_token(token::ETokenKind::RANGE);
      }
      return true;
    }
    return false;
  };

  if (stream.check('0') && stream.peek(1) != EOF) {
    char nt = stream.peek(1);
    if (nt == 'b' || nt == 'B') {
      is_bin = true;
      (void)stream.next(); // consume 0
      (void)stream.next(); // consume b B
    } else if (nt == 'o' || nt == 'O') {
      is_oct = true;
      (void)stream.next(); // consume 0
      (void)stream.next(); // consume o O
    } else if (nt == 'x' || nt == 'X') {
      is_hex = true;
      (void)stream.next(); // consume 0
      (void)stream.next(); // consume x X
    }
  }


  do {
    // Only allow 0 1 ' _
    if (is_bin) {
      if (stream.peek(1) == '0' || stream.peek(1) == '1') {
        continue;
      }

      if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      }

      break;
    }

    // Only allow 0 1 2 3 4 5 6 7 ' _
    if (is_oct) {
      if (stream.peek(1) >= '0' && stream.peek(1) <= '7') {
        continue;
      }

      if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      }

      break;
    }

    // Only allow 0 1 2 3 4 5 6 7 8 9 A B C D E F ' _
    if (is_hex) {
      if (common::utils::is_digit(stream.peek(1)) || (stream.peek(1) >= 'a' && stream.peek(1) <= 'f')
          || (stream.peek(1) >= 'A' && stream.peek(1) <= 'F')) {
        continue;
      }
      if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      }

      break;
    }

    // numeric vvv

    // classic numeric
    if (common::utils::is_digit(stream.peek(1))) {
      continue;
    }
    // prevent range creation : save buffer vals

    if (stream.peek(1) == '.' && stream.peek(2) == '.') {
      check_range_case(); // create range
      return;             // must stop after range creation
    }

    if (stream.peek(1) == '.') {
      id_decimal = true;
    }
    // floating numeric (scientific notation) 10000e+10 100e-15
    else if (stream.peek(1) == 'e' || stream.peek(1) == 'E') {
      id_decimal = true;
      // exponent sign
      if (stream.peek(2) == '+' || stream.peek(2) == '-') {
        (void)stream.next();
      }
    } else if (stream.peek(1) == '.') {
      // member access : a.b

      if (!CU.file_info.tokens->tokens.empty()
          && CU.file_info.tokens->tokens.back().kind == token::ETokenKind::IDENTIFIER
          && common::utils::is_alpha(stream.peek(1))) {
        add_token(token::ETokenKind::DOT);
        return;
      }
      // floating value : 8. or 10.f or 3.14

      id_decimal = true;
      if (stream.peek(2) == 'f' || stream.peek(2) == 'F') (void)stream.next(); // consume .
    }
    // end
    else {
      break;
    }

  } while (stream.next());

  if (is_bin)
    add_token(token::ETokenKind::L_BIN);
  else if (is_oct)
    add_token(token::ETokenKind::L_OCT);
  else if (is_hex)
    add_token(token::ETokenKind::L_HEX);
  else if (id_decimal)
    add_token(token::ETokenKind::L_D);
  else
    add_token(token::ETokenKind::L_I);
}

bool Lexer::tokenize_keyword_identifier() noexcept
{
  start_buffer();

  read_identifier();

  auto it = token::k_keywords.find(get_buffer_str());
  if (it == token::k_keywords.end()) {
    if (is_buffer_empty()) return false;
    add_token(token::ETokenKind::IDENTIFIER);
    return true;
  }


  add_token(it->second);
  return true;
}

void Lexer::read_identifier() noexcept
{
  if (!common::utils::is_alpha(stream.peek()) && stream.peek() != '_') return;

  while (!stream.is_end()) {
    if (common::utils::is_alnum(stream.peek(1)) || stream.peek(1) == '_') {
      (void)stream.next();
      continue;
    }

    break;
  }
}

void Lexer::add_token(token::ETokenKind kind, bool do_not_move) noexcept
{
  token::Token tok;

  tok.begin  = buffer_start_pos;
  tok.length = stream.position() - buffer_start_pos + 1;
  tok.kind   = kind;

  assert(stream.position() >= buffer_start_pos && "Position calculation error");

  if (!do_not_move) (void)stream.next();

  (void)CU.file_info.tokens->add(tok);
}


void Lexer::add_error(ErrorCode code, std::string_view msg, std::string_view hint) noexcept
{
  start_buffer();
  auto out = Error_Diagnostic(CU.cuid, code, buffer_start_pos, stream.position(), compiler::EPhase::lexer, msg, hint);
  COMPILER.add_error(out);
}
