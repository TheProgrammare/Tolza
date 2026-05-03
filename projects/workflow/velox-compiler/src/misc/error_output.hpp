#pragma once

#include <cassert>
#include <cstddef>
#include <string>
#include <string_view>
#include <sys/types.h>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

enum class EErrorSeverity { debug, warning, error, fatal };

[[nodiscard]] std::string_view ESeverity_to_str(EErrorSeverity severity);
[[nodiscard]] std::string_view ESeverity_to_color(EErrorSeverity severity);

[[nodiscard]] std::string      escapeChar(unsigned char c);
[[nodiscard]] std::string_view trim(std::string_view str);

struct Error_Raw {
  script::_id scr_info;

  size_t           start_pos = 0;
  size_t           end_pos   = 0;
  compiler::EPhase phase;
  EErrorSeverity   severity = EErrorSeverity::error;
  ErrorCode        code     = 0;
  std::string      msg;
  std::string      hint;
};


struct Error_Elem {
  script::_id scr_id;

  size_t           start_pos = 0;
  size_t           end_pos   = 0;
  compiler::EPhase phase;
  EErrorSeverity   severity = EErrorSeverity::error;
  // classic error 0000 - 0999 internal error 1000 - 1999
  ErrorCode        code     = 0;
  std::string      msg;
  std::string      hint;

  Error_Elem(script::_id _scr_id, ErrorCode _code, size_t _start_pos, size_t _end_pos, compiler::EPhase _phase,
             std::string_view _msg, std::string_view _hint)
    : scr_id(_scr_id)
    , start_pos(_start_pos)
    , end_pos(_end_pos)
    , phase(_phase)
    , severity(EErrorSeverity::error)
    , code(_code)
    , msg(_msg)
    , hint(_hint)
  {
    assert((!_scr_id || start_pos < end_pos) && "Illegal error bounds");
  }

  Error_Elem(script::_id _scr_id, ErrorCode _code, size_t _start_pos, size_t _end_pos, compiler::EPhase _phase,
             EErrorSeverity _severity, std::string_view _msg, std::string_view _hint)
    : scr_id(_scr_id)
    , start_pos(_start_pos)
    , end_pos(_end_pos)
    , phase(_phase)
    , severity(_severity)
    , code(_code)
    , msg(_msg)
    , hint(_hint)
  {
    assert((!_scr_id || start_pos < end_pos) && "Illegal error bounds");
  }

  // [file] file:LL:CC
  // [code] | code line
  //        |      ^^^^
  // [error] [AAwxyz] blabla
  // [hint] blabla
  // [context] global -> fn -> ...
  std::string print_error() const;

  std::string print_code() const;

  std::string print_messages() const;

  std::string print_cursor() const;

  std::string print_line() const;
  std::string print_line_cursor() const;

  std::string print_source() const;

  std::string print_link_error() const
  {
    return std::string();
  }
};


struct Error_Diagnostic {
  Error_Diagnostic(const Error_Elem& first, const Error_Elem& second)
    : elem_first(first)
    , elem_second(second)
  {
  }
  Error_Diagnostic(script::_id _scr_id, ErrorCode _code, size_t _start_pos, size_t _end_pos, compiler::EPhase _phase,
                   std::string_view _msg, std::string_view _hint)
    : elem_first(_scr_id, _code, _start_pos, _end_pos, _phase, _msg, _hint)
    , elem_second({}, 0, 0, 0, _phase, "", "")
  {
    assert(_scr_id && "Illegal error on unknown script");
  }

  Error_Elem elem_first;
  Error_Elem elem_second;

  std::string print_error() const;
};