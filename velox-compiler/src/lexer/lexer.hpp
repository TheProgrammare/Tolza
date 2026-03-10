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
#include "token.hpp"

enum class ETokenType;
using TokTy = ETokenType;

struct ScriptInfo;

using ErrorCode = short;


class Lexer
{
public:
  Lexer(ScriptInfo& _scr_info);

  enum class EPrefixFound { None, Prefix, All };

  EPrefixFound                  get_prefix_keyword(TokTy _type, const std::string& _key, const std::string& _search);
  bool                          is_valid_prefix(char prefix, const std::string& _current);
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
  void                          addToken(TokTy type);
  bool                          eat();
  void                          add_error(ErrorCode code, const std::string& msg, const std::string& hint);
  TokTy                         classifyNumerals(std::string& outValue);
  TokTy                         classifyKeyword(std::string& outWord);
  TokTy                         classifyFormatSpec(std::string& outFormat);

  ScriptInfo&              scr_info;
  StreamTracker            stream;
  std::vector<std::string> errors;
  std::string              buffer;
  char                     ch = '\0';
};
