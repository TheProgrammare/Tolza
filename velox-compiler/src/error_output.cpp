#include "error_output.hpp"

#include "script_info.hpp"

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
  case EErrorSeverity::debug:   return color_CYAN;
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

std::string Error_Diagnostic::print_line() const
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

std::string Error_Diagnostic::print_source() const
{
  return "[file] " + scr_info.file_path + ":" + std::to_string(token.span.line) + ":" + std::to_string(token.span.col)
         + "\n" color_RESET;
}