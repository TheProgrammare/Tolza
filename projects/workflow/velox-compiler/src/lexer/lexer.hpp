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

#include <string>
#include <vector>
#include <set>

#include "stream_tracker.hpp"

enum class ETokenType;
using TokTy = ETokenType;

struct ScriptInfo;

using ErrorCode = short;


class Lexer
{
public:
  Lexer(ScriptInfo& _scr_info);

  enum class EPrefixFound { None, Prefix, All };

  EPrefixFound                  get_prefix_keyword(TokTy _type, std::string_view _key, std::string_view _search);
  bool                          is_valid_prefix(char prefix, std::string_view _current);
  void                          tokenize(const std::set<char>& exit_char);
  void                          process_escape();
  void                          tokenize_textual();
  bool                          tokenize_spec();
  void                          tokenize_comment();
  void                          tokenize_metacode();
  void                          tokenize_numeric();
  // not
  // idependent
  void                          tokenize_identifier();
  void                          tokenize_keyword();
  std::pair<TokTy, std::string> getToken();
  void                          add_token(TokTy type, bool do_not_move = false);
  void                          add_error(ErrorCode code, const std::string& msg, const std::string& hint);
  TokTy                         classifyNumerals(std::string& outValue);
  TokTy                         classifyKeyword(std::string& outWord);
  TokTy                         classifyFormatSpec(std::string& outFormat);

  void start_buffer();

  ScriptInfo&              scr_info;
  StreamTracker            stream;
  std::vector<std::string> errors;
  std::string              buffer;
  size_t                   buffer_start_line = 1;
  size_t                   buffer_start_col  = 1;
};

const std::vector<std::pair<std::string_view, TokTy>>& get_sorted_keywords();


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