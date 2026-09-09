#pragma once

#include "nexus/forward.hpp"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

namespace token
{

class Viewer
{
public:
  Viewer(cu::CU& p_CU);


  [[nodiscard]] token::Token& next() noexcept;
  void                        jump(size_t newPosition) noexcept;
  [[nodiscard]] token::Token& peek(size_t offset = 0) const noexcept;
  [[nodiscard]] token::Token& prev() noexcept;

  [[nodiscard]] bool is_end() const noexcept;

  [[nodiscard]] bool look_ahead(token::ETokenKind check, token::ETokenKind terminaison) const noexcept;


  [[nodiscard]] bool        match(token::ETokenKind kind) noexcept;
  [[nodiscard]] bool        match_val(std::string_view val) noexcept;
  [[nodiscard]] bool        match_id_val(std::string_view val) noexcept;
  [[nodiscard]] std::string match_any_val(const std::initializer_list<std::string>& val) noexcept;
  [[nodiscard]] bool        match_any(const std::initializer_list<token::ETokenKind>& kinds) noexcept;
  [[nodiscard]] bool        match_chain(const std::initializer_list<token::ETokenKind>& l) noexcept;

  [[nodiscard]] bool check(token::ETokenKind kind) const noexcept;
  [[nodiscard]] bool check_val(std::string_view val) const noexcept;
  [[nodiscard]] bool check_id_val(std::string_view val) const noexcept;
  [[nodiscard]] bool check_any(const std::initializer_list<token::ETokenKind>& kinds) const noexcept;
  [[nodiscard]] bool check_chain(const std::initializer_list<token::ETokenKind>& l) const noexcept;

  [[nodiscard]] token::Token& expect(ErrorCode code, token::ETokenKind kind, std::string_view msg,
                                     std::string_view hint) noexcept;
  [[nodiscard]] token::Token& expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& kinds,
                                         std::string_view msg, std::string_view hint) noexcept;

  [[nodiscard]] size_t size() const noexcept;
  [[nodiscard]] size_t position() const noexcept;
  [[nodiscard]] size_t line() const noexcept;
  void                 rewind(size_t pos) noexcept;

  void add_error(ErrorCode code, std::string_view msg, std::string_view hint) noexcept;
  void add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg, std::string_view hint) noexcept;

  [[nodiscard]] const token::Token& get(size_t pos) const noexcept;

  cu::CU&          CU;
  compiler::EPhase phase;

private:
  size_t            current = 0;
  std::string_view  current_str;
  token::ETokenKind current_token;
  // tabe sorted with logest keywords first
};

} // namespace token