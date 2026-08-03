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
  auto __id_##var_name = CU.metacodes->add<metacode::type_name>(current_parent->metaid);                               \
  auto var_name        = CU.metacodes->as<metacode::type_name>(__id_##var_name);

struct Preprocessor final {
  Preprocessor(cu::CU& CU) noexcept;

  cu::CU&        CU;
  token::Viewer* tok_v;

  [[nodiscard]] bool start_preprocessor() noexcept;

  [[nodiscard]]
  token::ID preprocess_token(token::Token& tok) noexcept;
  void      add_token_to_generate(token::Token& tok) noexcept;
  void      add_generated_token(token::ID id) noexcept;

  void preprocess_scope(metacode::Metacode& meta) noexcept;

  [[nodiscard]] metacode::ID preprocess_file() noexcept;
  [[nodiscard]] metacode::ID preprocess_any() noexcept;
  [[nodiscard]] metacode::ID preprocess_metablock() noexcept;

  [[nodiscard]] metacode::ID preprocess_if() noexcept;
  [[nodiscard]] bool         _if_end(metacode::If& end_wait) noexcept;
  [[nodiscard]] bool         _else(metacode::If& before_else) noexcept;
  [[nodiscard]] bool         _elif(metacode::If& before_elif) noexcept;

  [[nodiscard]] metacode::ID preprocess_expand() noexcept;
  void                       _expand_header() noexcept;
  void                       _expand_body() noexcept;
  [[nodiscard]] bool         _expand_placeholder() noexcept;
  [[nodiscard]] bool         _expand_if() noexcept;

  [[nodiscard]] metacode::ID preprocess_condition() noexcept;
  [[nodiscard]] metacode::ID _cond_atom() noexcept;

  // token viewver will jump after the metacode pattern (the invisible end metacode token) check first if have the start
  // metacode token : '#' don't start by '#' pattern !
  [[nodiscard]] bool match_metacode(std::initializer_list<std::string_view> pattern) noexcept;


  // token viewver will stay at his original position check first if have the start metacode token : '#' don't start by
  // '#' pattern !
  [[nodiscard]] bool check_metacode(std::initializer_list<std::string_view> pattern) noexcept;

  [[nodiscard]] bool is_tok_in_pattern(const token::Token& tok, std::string_view pattern) const noexcept;

  [[nodiscard]] std::string_view tok_to_str(token::ID id) const noexcept;

  metacode::Metacode* current_parent = nullptr;
  metacode::Expand*   current_expand = nullptr;

  size_t last_tok_pos = 0;
};

} // namespace metacode