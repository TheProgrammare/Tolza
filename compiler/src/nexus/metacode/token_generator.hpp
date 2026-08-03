#pragma once

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include <string_view>
#include <vector>


namespace metacode
{

struct Generator final {
  Generator(cu::CU& p_CU, metacode::Preprocessor& p_prepro) noexcept;

  cu::CU&                 CU;
  metacode::Preprocessor& prepro;


  [[nodiscard]] bool start_generator() noexcept;

  void normalize_tokens_generated_ids() noexcept;


  [[nodiscard]] std::vector<ID>*    get_children(metacode::ID id) const noexcept;
  [[nodiscard]] metacode::Metacode& get_child(const std::vector<metacode::ID>* children,
                                              size_t                           gen_count) const noexcept;

  void add_token(token::Token& tok) noexcept;

  void gen_tokens(const std::vector<token::ID>& toks) noexcept;

  void gen_Metacode(metacode::Metacode& m) noexcept;

  void gen_Root(metacode::Root& m) noexcept;
  void gen_Metablock(metacode::Metablock& m) noexcept;
  void gen_If(metacode::If& m) noexcept;
  void gen_Expand(metacode::Expand& m) noexcept;

  [[nodiscard]] bool        eval_cond(metacode::ID id) noexcept;
  [[nodiscard]] bool        eval_binary(metacode::Binary_Cond& c) noexcept;
  [[nodiscard]] bool        eval_not_unary(metacode::Unary_Not_Cond& c) noexcept;
  [[nodiscard]] std::string eval_expr(metacode::ID id) noexcept;

  enum class EPostTokenKind : uint8_t { filesource, metacode, placeholder };

  [[nodiscard]] static EPostTokenKind scan_token(token::ID tok) noexcept;

  std::vector<token::Token> tokens_generated;

  metacode::Metacode* current_metacode = nullptr;
  metacode::Expand*   current_expand   = nullptr;
  metacode::Env       current_placeholder_env;
};

} // namespace metacode