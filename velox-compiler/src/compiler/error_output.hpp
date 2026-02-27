#pragma once

#include <string>
#include <vector>

#include "globals.hpp"
#include "compiler/lexer/token.hpp"
#include "compiler/script_info.hpp"

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


template <size_t Code>
struct Error_Diagnostic {
  const ScriptInfo& scr_info;

  Token                    token;
  std::vector<Token>       tokens_inpacted;
  Config::EPhase           phase    = Config::EPhase::parser;
  EErrorSeverity           severity = EErrorSeverity::error;
  // classic error 0000 - 0999 internal error 1000 - 1999
  size_t                   code     = 0000;
  std::vector<std::string> context;
  std::string              msg;
  std::string              hint;

  Error_Diagnostic<Code>() = delete;

  Error_Diagnostic<Code>(const ScriptInfo& _scr_info, const Token& _token, const std::vector<Token>& _tokens,
                         Config::EPhase _phase, EErrorSeverity _severity, const std::vector<std::string>& _context,
                         const std::string& _msg, const std::string& _hint)
    : scr_info(_scr_info)
    , token(_token)
    , tokens_inpacted(_tokens)
    , phase(_phase)
    , severity(_severity)
    , code(Code)
    , context(_context)
    , msg(_msg)
    , hint(_hint)
  {
    static_assert(Code < 9999, "The Error code is higher than 9999 maximum permitted.");
  }

  // [file] file:LL:CC
  // [code] | code line
  //        |      ^^^^
  // [error] [AAwxyz] blabla
  // [hint] blabla
  // [context] global -> fn -> ...
  std::string print_error() const
  {
    return print_source() + print_line() + print_messages();
  }

  std::string print_code() const
  {
    return "[" + ESeverity_to_color(severity) + Config::Phase_to_code(phase) + std::format("{:04}", code)
           + color_RESET "] ";
  }

  std::string print_messages() const
  {
    std::string str_msg;
    str_msg += "[" + ESeverity_to_color(severity) + ESeverity_to_str(severity) + color_RESET "] ";
    str_msg += print_code() + " " + msg;
    if (!hint.empty()) str_msg += "\n[hint] " color_CYAN + hint + color_RESET;

    if (!context.empty()) {
      str_msg += "\n[context] " color_CYAN;

      size_t count = 0;
      for (auto& elem : context) {
        str_msg += elem;
        if (count != context.size() - 1) str_msg += " >> ";
      }
      str_msg += color_RESET;
    }

    return str_msg;
  }

  std::string print_line() const
  {
    size_t finalCursorSize = token.span.size == 0 ? 1 : token.span.size;
    // Trim
    // code
    if (token.span.line - 1 < 0 || token.span.line - 1 > scr_info.get_line_size()) return "NO VALID LINE INDEX";

    std::string line_source = scr_info.get_line(token.span.line);
    std::string trimmedLine = line_source.empty() ? "NO LINE FOUND" : trim(line_source);
    size_t      trimSize    = abs(int(trimmedLine.size() - line_source.size()));

    // cursor
    std::string line_offset_str = std::string(6 - std::to_string(token.span.line).size(), ' ');
    std::string cursor          = std::string(finalCursorSize, '^');
    size_t      cursor_offset =
        token.span.col < token.span.size + trimSize ? 0 : token.span.col - (token.span.size + trimSize);
    std::string cursor_offset_str = std::string(cursor_offset, ' ');

    // final
    return line_offset_str + std::to_string(token.span.line) + " | " + trimmedLine +
           "\n"
           "       | " color_RED
           + cursor_offset_str + cursor + color_RESET "\n";
  }

  std::string print_source() const
  {
    return "[file] " + scr_info.file_path.string() + ":" + std::to_string(token.span.line) + ":"
           + std::to_string(token.span.col) + "\n" color_RESET;
  }

  std::string print_link_error() const
  {
    return std::string();
  }
};
