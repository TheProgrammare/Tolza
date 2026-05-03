#pragma once

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include <string_view>
#include <vector>


namespace metacode
{

struct Generator final {
  Generator(script::ScriptInfo& p_scr_info, metacode::Preprocessor& p_prepro);

  script::ScriptInfo&     scr_info;
  metacode::Preprocessor& prepro;

  std::vector<_id>*   get_children(metacode::_id id);
  metacode::Metacode& get_child(const std::vector<metacode::_id>* children, size_t gen_count);

  void add_token(token::Token& tok);


  bool start_generator();

  void gen_tokens(const std::vector<token::_id>& toks);

  void gen_Metacode(metacode::Metacode& m);

  void gen_Root(metacode::Root& m);
  void gen_Metablock(metacode::Metablock& m);
  void gen_If(metacode::If& m);
  void gen_Expand(metacode::Expand& m);

  bool        eval_cond(metacode::_id id);
  bool        eval_binary(metacode::Binary_Cond& c);
  bool        eval_not_unary(metacode::Unary_Not_Cond& c);
  std::string eval_expr(metacode::_id id);

  enum class EPostTokenKind { filesource, metacode, placeholder };

  EPostTokenKind scan_token(token::_id tok);

  std::vector<token::Token> tokens_generated;

  metacode::Metacode* current_metacode = nullptr;
  metacode::Expand*   current_expand   = nullptr;
  metacode::Env       current_placeholder_env;
};

} // namespace metacode