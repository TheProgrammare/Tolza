/*
 *	The Tolza programming language - Apache License, Version 2.0
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

#include "nexus/forward.hpp"
#include "stream_tracker.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <set>
#include <string_view>
#include <unordered_map>
#include <vector>


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
  [[nodiscard]] static constexpr DFA build_DFA();

  Lexer(cu::CU& _CU);

  enum class EPrefixFound : uint8_t { None, Prefix, All };

  [[nodiscard]] bool tokenize(const std::set<char>& p_exit_char = {}) noexcept;

  [[nodiscard]] bool tokenize_DFA() noexcept;
  void               tokenize_textual() noexcept;
  [[nodiscard]] bool tokenize_spec() noexcept;
  void               tokenize_comment() noexcept;
  void               tokenize_metacode() noexcept;
  void               tokenize_numeric() noexcept;
  [[nodiscard]] bool tokenize_keyword_identifier() noexcept;

  void read_identifier() noexcept;


  void process_escape() noexcept;

  void add_token(token::ETokenKind kind, bool do_not_move = false) noexcept;
  void add_error(ErrorCode code, std::string_view msg, std::string_view hint) noexcept;

  void                           start_buffer() noexcept;
  [[nodiscard]] std::string_view get_buffer_str() const noexcept;
  [[nodiscard]] bool             is_buffer_empty() const noexcept;

  cu::CU&       CU;
  StreamTracker stream;
  size_t        buffer_start_pos = -1;
  bool          on_escape        = false;
};
