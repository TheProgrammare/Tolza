#pragma once

#include <string>

#include "nexus/forward.hpp"
// #include "token.hpp"

namespace token
{

class Viewer
{
public:
  Viewer(script::ScriptInfo& p_scr_info);

  token::Token& next() noexcept;
  void          jump(size_t newPosition) noexcept;
  token::Token& peek(int offset = 0) const noexcept;
  token::Token& prev() noexcept;

  bool is_end() const noexcept;

  bool look_ahead(token::ETokenKind check, token::ETokenKind terminaison) const noexcept;


  bool        match(token::ETokenKind expected) noexcept;
  bool        match_val(std::string_view val) noexcept;
  bool        match_id_val(std::string_view val) noexcept;
  std::string match_any_val(const std::initializer_list<std::string>& val) noexcept;
  bool        match_any(const std::initializer_list<token::ETokenKind>& types) noexcept;

  bool check(token::ETokenKind expected) const noexcept;
  bool check_val(std::string_view val) const noexcept;
  bool check_id_val(std::string_view val) const noexcept;
  bool check_any(const std::initializer_list<token::ETokenKind>& types) const noexcept;

  token::Token& expect(ErrorCode code, token::ETokenKind type, std::string_view msg, std::string_view hint) noexcept;
  token::Token& expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types, std::string_view msg,
                           std::string_view hint) noexcept;

  size_t size() const noexcept;
  size_t position() const noexcept;
  size_t line() const noexcept;
  void   rewind(size_t pos) noexcept;

  void add_error(ErrorCode code, std::string_view msg, std::string_view hint);

  void add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg, std::string_view hint);

  void synchronize() noexcept;

  const token::Token& get(size_t position);

  script::ScriptInfo& scr_info;
  compiler::EPhase    phase;

private:
  size_t            current = 0;
  std::string_view  current_str;
  token::ETokenKind current_token;
  // tabe sorted with logest keywords first
};

} // namespace token