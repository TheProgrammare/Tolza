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

#include "metacode.hpp"

struct Token;
struct ScriptInfo;
struct TokenViewer;

struct Preprocessor {
public:
  Preprocessor(ScriptInfo& scr_info);

  ScriptInfo&             scr_info;
  META::MetablockManager* m_meta = nullptr;
  TokenViewer*            tok_v  = nullptr;

  [[nodiscard]] std::vector<Token> preprocess();

private:
  void debug_write_postprocess_code_files(const std::vector<Token>& toks);

  template <META::DerivedFromMeta MetaNode>
  [[nodiscard]] std::unique_ptr<MetaNode> Create_Meta(const Token& tok, size_t start_scope_pos);

  [[nodiscard]] bool process_any_meta(META::MetaBlock& parent);
  [[nodiscard]] bool process_metablock(META::MetaBlock& parent);
  void               process_scope(META::MetaBlock& meta);
  [[nodiscard]] bool process_if(META::MetaBlock& parent);

  bool _if_end_metacode(META::MetaBlock_If& end_wait);

  bool _else_metacode(META::MetaBlock_If& before_else);

  bool _elif_metacode(META::MetaBlock_If& before_elif);

  [[nodiscard]] bool                                    process_expand(META::MetaBlock& parent);
  [[nodiscard]] std::unique_ptr<META::MetaBlock_Expand> _expand_header();
  void                                                  _expand_body(META::MetaBlock_Expand& expansion_meta);
  [[nodiscard]] bool                                    _expand_placeholder(META::MetaBlock_Expand& expansion_meta);
  [[nodiscard]] bool                                    _expand_if(META::MetaBlock_Expand& expansion_meta);
  [[nodiscard]] std::unique_ptr<META::Cond_Base>        process_condition();
  [[nodiscard]] std::unique_ptr<META::Cond_Base>        _cond_atom();

  [[nodiscard]] bool check_metacode(std::initializer_list<std::string> pattern);
  [[nodiscard]] bool match_metacode(std::initializer_list<std::string> pattern);
  [[nodiscard]] bool is_tok_in_pattern(const Token& tok, const std::string& pattern);

  [[nodiscard]] std::vector<Token> get_scope_tokens(size_t start_pos, size_t end_pos);

  [[nodiscard]] size_t get_line_last_tok_pos(size_t line);
};
