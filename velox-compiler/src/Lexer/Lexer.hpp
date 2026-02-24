/*
 *	The
 *Velox
 *programming
 *language
 *- Apache
 *License,
 *Version 2.0
 *  Copyright
 *2024-2026
 *Foz
 *Florian
 *
 *  Licensed
 *under
 *the
 *Apache
 *License,
 *Version 2.0
 *(the
 *"License");
 *  you
 *may not
 *use this
 *file
 *except
 *in
 *compliance
 *with the
 *License.
 *  You
 *may
 *obtain a
 *copy of
 *the
 *License
 *at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless
 *required
 *by
 *applicable
 *law or
 *agreed
 *to in
 *writing,
 *software
 *  distributed
 *under
 *the
 *License
 *is
 *distributed
 *on an
 *"AS IS"
 *BASIS,
 *  WITHOUT
 *WARRANTIES
 *OR
 *CONDITIONS
 *OF ANY
 *KIND,
 *either
 *express
 *or
 *implied.
 *  See
 *the
 *License
 *for the
 *specific
 *language
 *governing
 *permissions
 *and
 *  limitations
 *under
 *the
 *License.
 */

#pragma once

#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "ScriptInfo.hpp"
#include "StreamTracker.hpp"
#include "Token.hpp"

enum class ETokenType;
using TokTy = ETokenType;

const std::unordered_set<std::string> kScriptMeta = {
    "aut"
    "ho"
    "r",
    "tit"
    "le",
    "ver"
    "sio"
    "n",
    "des"
    "cri"
    "pti"
    "on",
    "cre"
    "ate"
    "d",
    "upd"
    "ate"
    "d",
    "lan"
    "gua"
    "ge_"
    "ver"
    "sio"
    "n",
    "enc"
    "odi"
    "ng",
    "lic"
    "ens"
    "e",
    "con"
    "tri"
    "but"
    "or",
    "con"
    "tac"
    "t",
    "cop"
    "yri"
    "gh"
    "t",
    "wik"
    "i",
    "do"
    "c",
    "o"
    "s"};

class Lexer
{
public:
  Lexer(ScriptInfo& _scr_info)
    : scr_info(_scr_info)
    , stream(scr_info.file_str)
  {
  }

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
  template <size_t Code>
  void  add_error(const std::string& msg, const std::string& hint);
  TokTy classifyNumerals(std::string& outValue);
  TokTy classifyKeyword(std::string& outWord);
  TokTy classifyFormatSpec(std::string& outFormat);

  ScriptInfo&              scr_info;
  StreamTracker            stream;
  std::vector<std::string> errors;
  std::string              buffer;
  char                     ch = '\0';
};
