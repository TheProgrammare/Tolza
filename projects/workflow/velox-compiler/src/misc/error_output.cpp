#include "error_output.hpp"

#include "nexus/ast/ast.hpp"

#include "nexus/forward.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/script.hpp"
#include "compiler/compiler.hpp"

#include <sstream>
#include <iomanip>
#include <format>
#include <string>
#include <string_view>

std::string_view ESeverity_to_str(EErrorSeverity severity)
{
  switch (severity) {
  case EErrorSeverity::debug:   return "debug";
  case EErrorSeverity::warning: return "warning";
  case EErrorSeverity::error:   return "error";
  case EErrorSeverity::fatal:   return "fatal";
  }
}

std::string_view ESeverity_to_color(EErrorSeverity severity)
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

std::string_view trim(std::string_view str)
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


std::string Error_Elem::print_code() const
{
  return "[" + std::string(ESeverity_to_color(severity)) + compiler::Phase_to_code(phase) + std::format("{:04}", code)
         + color_RESET "] ";
}

std::string Error_Elem::print_error() const
{
  return print_source() + print_line_cursor() + print_messages();
}

std::string Error_Elem::print_messages() const
{
  std::string str_msg;
  str_msg +=
      "[" + std::string(ESeverity_to_color(severity)) + std::string(ESeverity_to_str(severity)) + color_RESET "] ";
  str_msg += print_code() + " " + msg;
  if (!hint.empty()) str_msg += "\n[hint] " + hint;

  return str_msg;
}

std::string Error_Elem::print_cursor() const
{
  auto& script = compiler::COMPILER.pipeline.get_script(scr_id);

  size_t line = script.file_info.get_line_from_pos(start_pos);

  const std::string cursor            = std::string(end_pos - start_pos, '^');
  const int         dist              = end_pos - script.file_info.get_line_start(line) - cursor.size();
  const size_t      cursor_offset     = dist < 0 ? 0 : dist;
  const std::string cursor_offset_str = std::string(cursor_offset, ' ');

  return cursor_offset_str + cursor;
}

std::string Error_Elem::print_line() const
{
  auto& script = compiler::COMPILER.pipeline.get_script(scr_id);

  auto line_pos = script.file_info.get_line_from_pos(start_pos);
  auto line_str = script.file_info.get_line(line_pos);

  const std::string line_offset_str = std::string(6 - std::to_string(line_pos + 1).size(), ' ');

  return line_offset_str + std::to_string(line_pos + 1) + " | " color_RESET + std::string(line_str) + "\n" color_RESET;
}


std::string Error_Elem::print_line_cursor() const
{
  const std::string line   = print_line();
  const std::string cursor = print_cursor();

  return line + "       | " color_RED + cursor + color_RESET "\n";
}

std::string Error_Elem::print_source() const
{
  auto&  script = compiler::COMPILER.pipeline.get_script(scr_id);
  size_t column = script.file_info.get_column_from_pos(start_pos);
  size_t line   = 0;

  return "[file] " color_MAGENTA + script.file_info.path + ":" + std::to_string(line + 1) + ":"
         + std::to_string(column + 1) + "\n" color_RESET;
}


std::string Error_Diagnostic::print_error() const
{
  auto& first_script = compiler::COMPILER.pipeline.get_script(elem_first.scr_id);

  // one file
  if (!elem_second.scr_id) {
    return elem_first.print_error();
  }

  auto& second_script = compiler::COMPILER.pipeline.get_script(elem_second.scr_id);

  size_t first_line  = first_script.file_info.get_line_from_pos(elem_first.start_pos);
  size_t second_line = second_script.file_info.get_line_from_pos(elem_second.start_pos);


  // same file, same line
  if (elem_first.scr_id == elem_second.scr_id && first_line == second_line) {
    auto line       = elem_first.print_line();
    auto top_cursor = elem_first.print_cursor();
    std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
    auto down_cursor = elem_second.print_cursor();

    std::string out;
    out += "       | " color_RED + top_cursor + color_RESET "\n";
    out += line;
    out += "       | " color_RED + down_cursor + color_RESET "\n";

    out += elem_first.print_messages() + "\n";
    return out;
  }
  // same file
  else if (elem_first.scr_id == elem_second.scr_id) {
    auto top_line   = elem_first.print_line();
    auto top_cursor = elem_first.print_cursor();
    std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
    auto down_line   = elem_second.print_line();
    auto down_cursor = elem_second.print_cursor();

    std::string out;
    out += elem_first.print_source();
    out += "       | " color_RED + top_cursor + color_RESET "\n";
    out += top_line;
    out += "       | ...\n";
    out += down_line;
    out += "       | " color_RED + down_cursor + color_RESET "\n";

    out += elem_first.print_messages() + "\n";
    return out;
  }

  // different files
  auto top_line   = elem_first.print_line();
  auto top_cursor = elem_first.print_cursor();
  std::replace(top_cursor.begin(), top_cursor.end(), '^', 'v');
  auto down_line   = elem_second.print_line();
  auto down_cursor = elem_second.print_cursor();

  std::string out;
  out += elem_first.print_source();
  out += "       | " color_RED + top_cursor + color_RESET "\n";
  out += top_line;
  out += "       | ...\n";
  out += down_line;
  out += "       | " color_RED + down_cursor + color_RESET "\n";
  out += elem_second.print_source();

  out += elem_first.print_messages() + "\n";
  return out;
}
