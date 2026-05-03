#include "lexer.hpp"


#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <queue>

#include "nexus/ids.hpp"
#include "nexus/script.hpp"
#include "compiler/compiler.hpp"
#include "token.hpp"

#include "misc/error_output.hpp"

Lexer::DFANode::DFANode()
  : kind(token::ETokenKind::UNKNOWN)
{
}


Lexer::DFA Lexer::build_DFA()
{
  DFANode* root = new DFANode();

  // 1. Build trie
  for (auto& [txt, kind] : token::k_DFA) {
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
  nodes.push_back(root);

  while (!q.empty()) {
    DFANode* n = q.front();
    q.pop();

    for (auto& [c, nxt] : n->next) {
      if (nxt->id == DFA_INVALID) {
        nxt->id = nodes.size();
        nodes.push_back(nxt);
        q.push(nxt);
      }
    }
  }

  // 3. Build tables
  DFA dfa;
  dfa.transition.resize(nodes.size());
  dfa.accept.resize(nodes.size(), token::ETokenKind::UNKNOWN);

  for (auto* n : nodes) {
    auto& row = dfa.transition[n->id];
    row.fill(DFA_INVALID);

    for (auto& [c, nxt] : n->next) {
      row[(unsigned char)c] = nxt->id;
    }

    dfa.accept[n->id] = n->kind;
  }

  return dfa;
}


Lexer::Lexer(script::ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , stream(scr_info.file_info.data)
{
}

void Lexer::start_buffer()
{
  buffer_start_pos = stream.position();
}

std::string_view Lexer::get_buffer_str() const
{
  assert(buffer_start_pos <= stream.position());
  size_t length = stream.position() - buffer_start_pos + 1;
  return stream.data().substr(buffer_start_pos, length);
}

bool Lexer::is_buffer_empty() const
{
  return buffer_start_pos > stream.position();
}


bool Lexer::tokenize(const std::set<char>& exit_char)
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
    if (stop_guard()) break;

    // no char before keyword : clear all char control and spaces
    while ((is_ctrl(stream.peek()) || is_space(stream.peek())) && stream.next()) {
    }

    start_buffer();

    static size_t last_op = -1;
    // infinitive loop
    assert(last_op != stream.position());
    last_op = stream.position();

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
      read_identifier();
      auto a = 0;

      if (stream.peek(1) == ']') {
        stream.next(); // consume ]
        if (stream.peek(1) == ']') {
          stream.next(); // consume ]
          add_token(token::ETokenKind::S_METACODE_PLACEHOLDER);
          continue;
        }
      }

      add_error(0, "Expected end of placeholder end ']]' after placeholder start '[['",
                "define placeholders in code like: `[[_U]]`");
    } else if (tokenize_DFA()) {

    } else if (tokenize_keyword_identifier()) {

    }
    // can be a numeric value or a range token (.. or ..=) or a variadic (...)
    else if (is_digit(stream.peek())) {
      start_buffer();

      tokenize_numeric();
      continue;
    }
  }

  if (exit_char.empty()) add_token(token::ETokenKind::S_END_OF_FILE, true);

  return true;
}


bool Lexer::tokenize_DFA()
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

    if (dfa.accept[state] != token::ETokenKind::UNKNOWN) {
      last_accept_state = state;
      last_accept_pos   = stream.position();
    }

    if (!stream.next()) break;
  }

  if (last_accept_state == DFA_INVALID) {
    stream.jump(start);
    return false;
  }

  stream.jump(last_accept_pos);
  add_token(dfa.accept[last_accept_state]);
  return true;
}

void Lexer::process_escape()
{
  // read next chracter after backslash
  stream.match('\\');

  on_escape = true;

  switch (stream.peek()) {
  case 'n':  break;
  case 't':  break;
  case 'r':  break;
  case '\\': break;
  case '\'': break;
  case '"':  break;
  case '0':  break;
  case 'a':  break;
  case 'b':  break;
  case 'f':  break;
  case 'v':  break;

  // hex sequence \xHH
  case 'x':  {
    for (int i = 0; i < 2; ++i) { // read 1 or 2 hex
      if (!stream.next() || !is_hex(stream.peek())) {
        add_error(2, "Invalid hex escape sequence", "define hex escape like: `\\xHH`");
        return;
      }
    }
    break;
  }
  // Unicode sequence \uXXXX or \UXXXXXXXX
  case 'u':
  case 'U': {
    int         num_digits = (stream.check('u')) ? 4 : 8;
    std::string hex;
    for (int i = 0; i < num_digits; ++i) {
      if (!stream.next() || !is_hex(stream.peek())) {
        add_error(3, "Invalid Unicode escape sequence", "define unicode escape like: `\\uXXXX`");
        return;
      }
    }
    break;
  }

  default:
    // escape char unknown : keep literally or ring the error
    add_error(6, "Unknown escape sequence: \\" + std::to_string(stream.peek()), "");
    break;
  }
}

void Lexer::tokenize_textual()
{
  auto tok_text = [&](bool with_escape, bool with_interpolation, std::vector<char> end_tokens) {
    do {
      if (with_interpolation) {

        if (stream.check('{')) {
          if (!is_buffer_empty()) add_token(token::ETokenKind::L_TEXTUAL);
          start_buffer();
          add_token(token::ETokenKind::S_INTERPOLATION_START);

          start_buffer();
          tokenize({'}', ':'});

          if (stream.check(':')) {
            start_buffer();
            add_token(token::ETokenKind::COLON);
            while (tokenize_spec()) {
              stream.next();
            }
          }

          if (stream.check('}')) {
            add_token(token::ETokenKind::S_INTERPOLATION_END);
            if (stream.check('"')) {
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
          add_token(token::ETokenKind::L_TEXTUAL);
          return;
        }
      } else if (end_tokens.size() == 1) {
        if (stream.check(end_tokens[0])) {
          add_token(token::ETokenKind::L_TEXTUAL);
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
          add_token(token::ETokenKind::L_TEXTUAL, true);
          return;
        }
      }

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

    while (is_digit(stream.peek(1))) {
      stream.next(); // consume current
    }

    add_token(token::ETokenKind::L_I);
    return true;
  }
  // Letters
  else if (is_alpha(stream.peek())) {
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
  case '<':  add_token(token::ETokenKind::OPEN_BRACKETS); return true;
  case '>':  add_token(token::ETokenKind::CLOSE_BRACKETS); return true;
  case '^':  add_token(token::ETokenKind::OP_CIRCUMFLEX); return true;
  case '~':  add_token(token::ETokenKind::TILDE); return true;
  case '=':  add_token(token::ETokenKind::ASSIGN); return true;
  case '%':  add_token(token::ETokenKind::OP_MODULO); return true;
  case ' ':  add_token(token::ETokenKind::SPACE); return true;
  case '}':  return false;
  default:   {
    auto error =
        Error_Diagnostic(scr_info.id, 151, buffer_start_pos, stream.position(), compiler::EPhase::lexer,
                         "Unexpected format specifier character",
                         "define format specifier like:"
                         "\n  - right-aligned: `{val:>10}`\n  - 2 decimals `{val:.2f}`\n  - hexadecimal `{val:#x}`");

    compiler::COMPILER.add_error(std::move(error));
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
  add_token(token::ETokenKind::METACODE);
  tokenize({'\n', '#'});
  add_token(token::ETokenKind::S_METACODE_END, true);
}

void Lexer::tokenize_numeric()
{
  bool is_bin = false, is_oct = false, is_hex = false;
  bool id_decimal = false;

  auto check_range_case = [&]() -> bool {
    if (!is_buffer_empty()) add_token(token::ETokenKind::L_I, true); // create literal integral

    if (stream.peek(1) == '.' && stream.peek(2) == '.') {
      stream.next(); // consume last
      start_buffer();
      stream.next(); // consume .

      // case range included
      if (stream.peek(1) == '=') {
        stream.next(); // consume =
        add_token(token::ETokenKind::RANGE_INCLUSIVE);
      }
      // case variadic
      else if (stream.peek(1) == '.') {
        stream.next(); // consume .
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
      stream.next(); // consume 0
      stream.next(); // consume b B
    } else if (nt == 'o' || nt == 'O') {
      is_oct = true;
      stream.next(); // consume 0
      stream.next(); // consume o O
    } else if (nt == 'x' || nt == 'X') {
      is_hex = true;
      stream.next(); // consume 0
      stream.next(); // consume x X
    }
  }


  do {
    // Only allow 0 1 ' _
    if (is_bin) {
      if (stream.peek(1) == '0' || stream.peek(1) == '1') {
        continue;
      } else if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      } else {
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 ' _
    else if (is_oct) {
      if (stream.peek(1) >= '0' && stream.peek(1) <= '7') {
        continue;
      } else if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      } else {
        break;
      }
    }
    // Only allow 0 1 2 3 4 5 6 7 8 9 A B C D E F ' _
    else if (is_hex) {
      if (is_digit(stream.peek(1)) || (stream.peek(1) >= 'a' && stream.peek(1) <= 'f')
          || (stream.peek(1) >= 'A' && stream.peek(1) <= 'F')) {
        continue;
      } else if (stream.peek(1) == '\'' || stream.peek(1) == '_') {
        continue;
      } else {
        break;
      }
    }
    // numeric
    else {
      // classic numeric
      if (is_digit(stream.peek(1))) {
        continue;
      }
      // prevent range creation : save buffer vals
      else if (stream.peek(1) == '.' && stream.peek(2) == '.') {
        check_range_case(); // create range
        return;             // must stop after range creation
      } else if (stream.peek(1) == '.') {
        id_decimal = true;
      }
      // floating numeric (scientific notation) 10000e+10 100e-15
      else if (stream.peek(1) == 'e' || stream.peek(1) == 'E') {
        id_decimal = true;
        // exponent sign
        if (stream.peek(2) == '+' || stream.peek(2) == '-') {
          stream.next();
        }
      } else if (stream.peek(1) == '.') {
        // member access : a.b

        if (!scr_info.file_info.tokens->tokens.empty()
            && scr_info.file_info.tokens->tokens.back().kind == token::ETokenKind::IDENTIFIER
            && is_alpha(stream.peek(1))) {
          add_token(token::ETokenKind::DOT);
          return;
        }
        // floating value : 8. or 10.f or 3.14
        else {
          id_decimal = true;
          if (stream.peek(2) == 'f' || stream.peek(2) == 'F') stream.next(); // consume .
        }
      }
      // end
      else {
        break;
      }
    }
  } while (stream.next());

  if (is_bin) return add_token(token::ETokenKind::L_BIN);
  if (is_oct) return add_token(token::ETokenKind::L_OCT);
  if (is_hex) return add_token(token::ETokenKind::L_HEX);
  if (id_decimal) return add_token(token::ETokenKind::L_D);

  return add_token(token::ETokenKind::L_I);
}

bool Lexer::tokenize_keyword_identifier()
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

void Lexer::read_identifier()
{
  while (!stream.is_end()) {
    if (is_alnum(stream.peek(1)) || stream.peek(1) == '_') {
      stream.next();
      continue;
    }

    break;
  }
}

void Lexer::add_token(token::ETokenKind kind, bool do_not_move)
{
  token::Token tok;

  tok.begin  = buffer_start_pos;
  tok.length = stream.position() - buffer_start_pos + 1;
  tok.kind   = kind;

#ifdef DEBUG
  tok.debug_val = get_buffer_str();
#endif

  if (!do_not_move) stream.next();

  scr_info.file_info.tokens->add(std::move(tok));
}

void Lexer::add_error(ErrorCode code, std::string_view msg, std::string_view hint)
{
  auto out =
      Error_Diagnostic(scr_info.id, code, buffer_start_pos, stream.position(), compiler::EPhase::lexer, msg, hint);
  compiler::COMPILER.add_error(std::move(out));
}
