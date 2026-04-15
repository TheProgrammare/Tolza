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

#include <memory>
#include <vector>

namespace meta
{
struct Manager;
struct Metablock;
struct Metablock_If;
struct Metablock_Expand;
struct Cond_Base;
} // namespace meta

struct Token;
struct ScriptInfo;
struct TokenViewer;

struct Preprocessor {
public:
  Preprocessor(ScriptInfo& scr_info);

  ScriptInfo&                    scr_info;
  std::shared_ptr<meta::Manager> meta_m;
  std::unique_ptr<TokenViewer>   tok_v;

  [[nodiscard]] std::vector<Token> preprocess();

private:
  void debug_write_postprocess_code_files(const std::vector<Token>& toks);

  template <typename MetaNode>
  [[nodiscard]] std::unique_ptr<MetaNode> Create_Meta(const Token& tok, size_t start_scope_pos);

  [[nodiscard]] bool process_any_meta(meta::Metablock& parent);
  [[nodiscard]] bool process_metablock(meta::Metablock& parent);
  void               process_scope(meta::Metablock& meta);
  [[nodiscard]] bool process_if(meta::Metablock& parent);

  bool _if_end_metacode(meta::Metablock_If& end_wait);

  bool _else_metacode(meta::Metablock_If& before_else);

  bool _elif_metacode(meta::Metablock_If& before_elif);

  [[nodiscard]] bool                                    process_expand(meta::Metablock& parent);
  [[nodiscard]] std::unique_ptr<meta::Metablock_Expand> _expand_header();
  void                                                  _expand_body(meta::Metablock_Expand& expansion_meta);
  [[nodiscard]] bool                                    _expand_placeholder(meta::Metablock_Expand& expansion_meta);
  [[nodiscard]] bool                                    _expand_if(meta::Metablock_Expand& expansion_meta);
  [[nodiscard]] std::unique_ptr<meta::Cond_Base>        process_condition();
  [[nodiscard]] std::unique_ptr<meta::Cond_Base>        _cond_atom();

  [[nodiscard]] bool check_metacode(std::initializer_list<std::string> pattern);
  [[nodiscard]] bool match_metacode(std::initializer_list<std::string> pattern);
  [[nodiscard]] bool is_tok_in_pattern(const Token& tok, const std::string& pattern);

  [[nodiscard]] std::vector<Token> get_scope_tokens(size_t start_pos, size_t end_pos);

  [[nodiscard]] size_t get_line_last_tok_pos(size_t line);
};
