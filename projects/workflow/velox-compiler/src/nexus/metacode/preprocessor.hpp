#pragma once

#include <string_view>
#include <vector>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/metacode/metacode.hpp"


namespace metacode
{

#define METACODE_CREATE(var_name, type_name)                                                                           \
  auto __id_##var_name = scr_info.metacodes->new_metacode<metacode::type_name>(current_parent->id);                    \
  auto var_name        = scr_info.metacodes->get_as<metacode::type_name>(__id_##var_name);

struct Preprocessor final {
  Preprocessor(script::ScriptInfo& scr_info);

  script::ScriptInfo& scr_info;
  token::Viewer*      tok_v;

  bool start_preprocessor();

  [[nodiscard]]
  token::_id preprocess_token(token::Token& tok);
  void       add_token_to_generate(token::Token& tok);
  void       add_generated_token(token::_id id);

  void preprocess_scope(metacode::Metacode& meta);

  [[nodiscard]] metacode::_id preprocess_file();
  [[nodiscard]] metacode::_id preprocess_any();
  [[nodiscard]] metacode::_id preprocess_metablock();

  [[nodiscard]] metacode::_id preprocess_if();
  bool                        __if_end(metacode::If& end_wait);
  bool                        __else(metacode::If& before_else);
  bool                        __elif(metacode::If& before_elif);

  [[nodiscard]] metacode::_id preprocess_expand();
  void                        __expand_header();
  void                        __expand_body();
  bool                        __expand_placeholder();
  bool                        __expand_if();

  [[nodiscard]] metacode::_id preprocess_condition();
  [[nodiscard]] metacode::_id __cond_atom();

  // token viewver will jump after the metacode pattern (the invisible end metacode token) check first if have the start
  // metacode token : '#' don't start by '#' pattern !
  bool match_metacode(std::initializer_list<std::string_view> pattern);


  // token viewver will stay at his original position check first if have the start metacode token : '#' don't start by
  // '#' pattern !
  bool check_metacode(std::initializer_list<std::string_view> pattern);

  bool is_tok_in_pattern(const token::Token& tok, std::string_view pattern);

  std::string_view tok_to_str(token::_id id) const;

  metacode::Metacode* current_parent = nullptr;
  metacode::Expand*   current_expand = nullptr;

  size_t last_tok_pos = 0;
};

} // namespace metacode