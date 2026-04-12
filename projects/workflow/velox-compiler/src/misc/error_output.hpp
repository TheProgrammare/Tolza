#pragma once

#include <array>
#include <string>
#include <sys/types.h>
#include <vector>

#include "compiler/compiler.hpp"
#include "lexer/token.hpp"

namespace ast
{
struct Node;
}

struct ScriptInfo;

enum class EErrorSeverity { debug, warning, error, fatal };

[[nodiscard]] std::string ESeverity_to_str(EErrorSeverity severity);
[[nodiscard]] std::string ESeverity_to_color(EErrorSeverity severity);

[[nodiscard]] std::string escapeChar(unsigned char c);
[[nodiscard]] std::string trim(const std::string& str);


struct Error_Diagnostic {
  const ScriptInfo& pass_scr_info;
  const ScriptInfo* node_scr_info;

  Token                    token;
  std::vector<Token>       tokens_inpacted;
  compiler::EPhase         phase    = compiler::EPhase::parser;
  EErrorSeverity           severity = EErrorSeverity::error;
  // classic error 0000 - 0999 internal error 1000 - 1999
  ErrorCode                code     = 0;
  std::vector<std::string> context;
  std::string              msg;
  std::string              hint;

  Error_Diagnostic() = delete;

  Error_Diagnostic(const ScriptInfo& _pass_scr_info, ErrorCode _code, const ScriptInfo* _node_scr_info,
                   const Token& _token, compiler::EPhase _phase, const std::string& _msg, const std::string& _hint)
    : pass_scr_info(_pass_scr_info)
    , node_scr_info(_node_scr_info)
    , token(_token)
    , phase(_phase)
    , severity(EErrorSeverity::error)
    , code(_code)
    , msg(_msg)
    , hint(_hint)
  {
  }

  Error_Diagnostic(const ScriptInfo& _pass_scr_info, ErrorCode _code, const ScriptInfo* _node_scr_info,
                   const Token& _token, const std::vector<Token>& _tokens, compiler::EPhase _phase,
                   EErrorSeverity _severity, const std::vector<std::string>& _context, const std::string& _msg,
                   const std::string& _hint)
    : pass_scr_info(_pass_scr_info)
    , node_scr_info(_node_scr_info)
    , token(_token)
    , tokens_inpacted(_tokens)
    , phase(_phase)
    , severity(_severity)
    , code(_code)
    , context(_context)
    , msg(_msg)
    , hint(_hint)
  {
  }

  const ScriptInfo* get_scr_info() const
  {
    if (node_scr_info)
      return node_scr_info;
    else
      return &pass_scr_info;
  }

  // [file] file:LL:CC
  // [code] | code line
  //        |      ^^^^
  // [error] [AAwxyz] blabla
  // [hint] blabla
  // [context] global -> fn -> ...
  std::string print_error() const
  {
    return print_source() + print_line_cursor() + print_messages();
  }

  std::string print_code() const;

  std::string print_messages() const
  {
    std::string str_msg;
    str_msg += "[" + ESeverity_to_color(severity) + ESeverity_to_str(severity) + color_RESET "] ";
    str_msg += print_code() + " " + msg;
    if (!hint.empty()) str_msg += "\n[hint] " + hint;

    if (!context.empty()) {
      str_msg += "\n[context] ";

      size_t count = 0;
      for (auto& elem : context) {
        str_msg += elem;
        if (count != context.size() - 1) str_msg += " >> ";
      }
    }

    return str_msg;
  }

  std::string print_cursor() const;

  std::string print_line() const;
  std::string print_line_cursor() const;

  std::string print_source() const;

  std::string print_link_error() const
  {
    return std::string();
  }
};


struct Error_Diagnostic_Two {
  Error_Diagnostic_Two(const ScriptInfo& _pass_scr_info, ErrorCode code, const ast::Node& first,
                       const ast::Node& second, compiler::EPhase _phase, const std::string& msg,
                       const std::string& hint);

  Error_Diagnostic first;
  Error_Diagnostic second;

  std::string print_error() const;
};