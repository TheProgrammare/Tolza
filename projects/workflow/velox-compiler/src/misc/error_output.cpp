#include "misc/error_output.hpp"

#include "ast/ast_base.hpp"
#include "misc/script_info.hpp"

#include <sstream>
#include <iomanip>
#include <format>

std::string ESeverity_to_str(EErrorSeverity severity)
{
  switch (severity) {
  case EErrorSeverity::debug:   return "debug";
  case EErrorSeverity::warning: return "warning";
  case EErrorSeverity::error:   return "error";
  case EErrorSeverity::fatal:   return "fatal";
  }
}

std::string ESeverity_to_color(EErrorSeverity severity)
{
  switch (severity) {
  case EErrorSeverity::debug:   return "";
  case EErrorSeverity::warning: return color_YELLOW;
  case EErrorSeverity::error:   return color_RED;
  case EErrorSeverity::fatal:   return color_RED;
  }
}


std::string escapeChar(unsigned char c)
{
  switch (c) {
  case '\a': return "\\a";
  case '\b': return "\\b";
  case '\f': return "\\f";
  case '\n': return "\\n";
  case '\r': return "\\r";
  case '\t': return "\\t";
  case '\v': return "\\v";
  case '\\': return "\\\\";
  case '\'': return "\\\'";
  case '\"': return "\\\"";
  default:   break;
  }

  if (isprint(c)) return std::string(1, c); // visible char

  std::ostringstream oss;
  oss << "\\x" << std::hex << std::setw(2) << std::setfill('0') << (int)c;
  return oss.str();
}

std::string trim(const std::string& str)
{
  const char* whitespace = " \t\n\r\f\v";

  // Find first position who is not white space
  size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos) {
    // string is null or contains only spaces
    return "";
  }

  // Find the last position who is not white space
  size_t end = str.find_last_not_of(whitespace);

  // Extract the sub string without spaces around
  return str.substr(start, end - start + 1);
}


std::string Error_Diagnostic::print_code() const
{
  return "[" + ESeverity_to_color(severity) + compiler::Phase_to_code(phase) + std::format("{:04}", code)
         + color_RESET "] ";
}

std::string Error_Diagnostic::print_cursor() const
{
  const size_t finalCursorSize = token.span.size == 0 ? 1 : token.span.size;

  const std::string cursor            = std::string(finalCursorSize, '^');
  const int         dist              = token.span.col - token.span.size;
  const size_t      cursor_offset     = dist < 0 ? 0 : dist;
  const std::string cursor_offset_str = std::string(cursor_offset, ' ');

  return cursor_offset_str + cursor;
}

std::string Error_Diagnostic::print_line() const
{
  if (token.span.line - 1 < 0 || token.span.line - 1 > get_scr_info()->get_line_size()) return "NO VALID LINE INDEX";

  const std::string line_offset_str = std::string(6 - std::to_string(token.span.line).size(), ' ');

  return line_offset_str + std::to_string(token.span.line) + " | " color_RESET
         + get_scr_info()->get_line(token.span.line) + "\n" color_RESET;
}


std::string Error_Diagnostic::print_line_cursor() const
{
  const std::string line   = print_line();
  const std::string cursor = print_cursor();

  return line + "       | " color_RED + cursor + color_RESET "\n";
}

std::string Error_Diagnostic::print_source() const
{
  return "[file] " color_MAGENTA + get_scr_info()->file_path + ":" + std::to_string(token.span.line) + ":"
         + std::to_string(token.span.col) + "\n" color_RESET;
}

Error_Diagnostic_Two::Error_Diagnostic_Two(const ScriptInfo& _pass_scr_info, ErrorCode code, const ast::Node& first,
                                           const ast::Node& second, compiler::EPhase _phase, const std::string& msg,
                                           const std::string& hint)
  : first(Error_Diagnostic(_pass_scr_info, code, first._scr_info, first._token, _phase, msg, hint))
  , second(Error_Diagnostic(_pass_scr_info, code, second._scr_info, second._token, _phase, msg, hint))
{
}

std::string Error_Diagnostic_Two::print_error() const
{
  // same file, same line
  if (first.get_scr_info() == second.get_scr_info() && first.token.span.line == second.token.span.line) {
    auto line       = first.print_line();
    auto top_cursor = first.print_cursor();
    std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
    auto down_cursor = second.print_cursor();

    std::string out;
    out += "       | " color_RED + top_cursor + color_RESET "\n";
    out += line;
    out += "       | " color_RED + down_cursor + color_RESET "\n";

    out += first.print_messages() + "\n";
    return out;
  }
  // same file
  else if (first.get_scr_info() == second.get_scr_info()) {
    auto top_line   = first.print_line();
    auto top_cursor = first.print_cursor();
    std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
    auto down_line   = second.print_line();
    auto down_cursor = second.print_cursor();

    std::string out;
    out += first.print_source();
    out += "       | " color_RED + top_cursor + color_RESET "\n";
    out += top_line;
    out += "       | ...\n";
    out += down_line;
    out += "       | " color_RED + down_cursor + color_RESET "\n";

    out += first.print_messages() + "\n";
    return out;
  }

  // different files
  auto top_line   = first.print_line();
  auto top_cursor = first.print_cursor();
  std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
  auto down_line   = second.print_line();
  auto down_cursor = second.print_cursor();

  std::string out;
  out += first.print_source();
  out += "       | " color_RED + top_cursor + color_RESET "\n";
  out += top_line;
  out += "       | ...\n";
  out += down_line;
  out += "       | " color_RED + down_cursor + color_RESET "\n";
  out += second.print_source();

  out += first.print_messages() + "\n";
  return out;
}
