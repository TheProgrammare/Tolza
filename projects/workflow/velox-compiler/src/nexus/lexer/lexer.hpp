/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <set>

#include "stream_tracker.hpp"
#include "nexus/forward.hpp"


class Lexer
{
public:
  using DFA_State                        = uint16_t;
  static constexpr DFA_State DFA_INVALID = 0xFFFF;

  struct DFA {
    std::vector<std::array<DFA_State, 256>> transition;
    std::vector<token::ETokenKind>          accept;
  };
  struct DFANode {
    DFANode();

    std::unordered_map<char, DFANode*> next;
    token::ETokenKind                  kind;
    DFA_State                          id = DFA_INVALID;
  };
  DFA build_DFA();

  Lexer(script::ScriptInfo& _scr_info);

  enum class EPrefixFound { None, Prefix, All };

  bool tokenize(const std::set<char>& p_exit_char = {});

  bool tokenize_DFA();
  void tokenize_textual();
  bool tokenize_spec();
  void tokenize_comment();
  void tokenize_metacode();
  void tokenize_numeric();
  bool tokenize_keyword_identifier();

  void read_identifier();


  void process_escape();

  void add_token(token::ETokenKind type, bool do_not_move = false);
  void add_error(ErrorCode code, std::string_view msg, std::string_view hint);

  void             start_buffer();
  std::string_view get_buffer_str() const;
  bool             is_buffer_empty() const;

  script::ScriptInfo& scr_info;
  StreamTracker       stream;
  size_t              buffer_start_pos = -1;
  bool                on_escape        = false;
};

inline bool is_space(unsigned char c) noexcept
{
  return (c == ' ' || (c >= '\t' && c <= '\r'));
}

inline bool is_ctrl(unsigned char c) noexcept
{
  return (c < 32 || c == 127);
}

inline bool is_digit(unsigned char c) noexcept
{
  return c >= '0' && c <= '9';
}

inline bool is_hex(unsigned char c) noexcept
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

inline bool is_alnum(unsigned char c) noexcept
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

inline bool is_alpha(unsigned char c) noexcept
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}