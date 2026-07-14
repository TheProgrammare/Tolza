#include "error_output.hpp"

#include "common/utils.hpp"
#include "nexus/forward.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/lexer/token.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>
#include <format>
#include <string>
#include <string_view>

#include <common/compiler_options.hpp>


std::string_view ESeverity_to_str(EErrorSeverity severity) noexcept
{
  switch (severity) {
  case EErrorSeverity::debug:   return "debug";
  case EErrorSeverity::warning: return "warning";
  case EErrorSeverity::error:   return "error";
  case EErrorSeverity::fatal:   return "fatal";
  }
}

std::string_view ESeverity_to_color(EErrorSeverity severity) noexcept
{
  switch (severity) {
  case EErrorSeverity::debug:   return "";
  case EErrorSeverity::warning: return color_YELLOW;
  case EErrorSeverity::error:
  case EErrorSeverity::fatal:   return color_RED;
  }
}


std::string escapeChar(char c) noexcept
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

std::string_view trim(std::string_view str) noexcept
{
  constexpr std::string_view whitespace = " \t\n\r\f\v";

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

Error_Elem::Error_Elem(cu::ID _cuid, ErrorCode _code, ast::ID nodeid, compiler::EPhase _phase, std::string_view _msg,
                       std::string_view _hint)
  : cuid(_cuid)
  , start_pos(nodeid.token().get().begin)
  , end_pos(nodeid.token().get().begin + nodeid.token().get().length)
  , error_info{
        .code  = _code,
        .msg   = std::string(_msg),
        .hint  = std::string(_hint),
        .phase = _phase,
    }
{
  assert(nodeid && "Invalid node id");
  assert((!_cuid || start_pos < end_pos) && "Illegal error bounds");
  assert(nodeid.token() && "Invalid token id");
  assert(nodeid.token().pos() < _cuid.get().file_info.data.size() && "Token out of bounds");
}

std::string Error_Elem::print_code() const noexcept
{
  return "[" + std::string(ESeverity_to_color(error_info.severity)) + compiler::Phase_to_code(error_info.phase)
         + std::format("{:04}", error_info.code) + color_RESET "] ";
}

std::string Error_Elem::print_error() const noexcept
{
  return print_source() + print_line_cursor() + print_messages();
}

std::string Error_Elem::print_messages() const noexcept
{
  std::string str_msg;
  str_msg += "[" + std::string(ESeverity_to_color(error_info.severity))
             + std::string(ESeverity_to_str(error_info.severity)) + color_RESET "] ";
  str_msg += print_code() + " " + std::string(error_info.msg);
  if (!error_info.hint.empty()) str_msg += "\n[hint] " + std::string(error_info.hint);

  return str_msg;
}

std::string Error_Elem::print_cursor() const noexcept
{
  const auto& cu = cuid.get();

  size_t line = cu.file_info.get_line_from_pos(start_pos);

  const size_t      size              = end_pos - start_pos == 0 ? 1 : end_pos - start_pos;
  const std::string cursor            = std::string(size, '^');
  const int         dist              = int(end_pos) - cu.file_info.get_line_start(line) - cursor.size();
  const size_t      cursor_offset     = dist < 0 ? 0 : dist;
  const std::string cursor_offset_str = std::string(cursor_offset, ' ');

  return cursor_offset_str + cursor;
}

std::string_view Error_Elem::get_raw_line() const noexcept
{
  const auto& cu = cuid.get();

  auto line_pos = cu.file_info.get_line_from_pos(start_pos);
  return cu.file_info.get_line(line_pos);
}

std::string Error_Elem::print_line() const noexcept
{
  const auto& cu = cuid.get();

  auto line_pos = cu.file_info.get_line_from_pos(start_pos);
  auto line_str = cu.file_info.get_line(line_pos);

  const std::string line_offset_str = std::string(6 - std::to_string(line_pos + 1).size(), ' ');
  const int         start_col       = int(end_pos) - cu.file_info.get_line_start(line_pos) - (end_pos - start_pos);
  int               end_col         = start_col + (end_pos - start_pos);
  if (end_col >= line_str.size()) end_col = line_str.size() - 1;
  int         next_cursor    = end_col + 1;
  std::string final_line_str = std::string(line_str.substr(0, start_col)) + color_RED
                               + std::string(line_str.substr(start_col, end_pos - start_pos)) + color_RESET
                               + std::string(line_str.substr(end_col + 1));

  return line_offset_str + std::to_string(line_pos + 1) + " | " color_RESET + final_line_str + "\n" color_RESET;
}


std::string Error_Elem::print_line_cursor() const noexcept
{
  const std::string line   = print_line();
  const std::string cursor = print_cursor();

  return line + "       | " color_RED + cursor + color_RESET "\n";
}

std::string Error_Elem::print_source() const noexcept
{
  const auto& cu     = cuid.get();
  size_t      column = cu.file_info.get_column_from_pos(start_pos);
  size_t      line   = cu.file_info.get_line_from_pos(start_pos);

  return "[file] " color_MAGENTA + cu.file_info.path + ":" + std::to_string(line + 1) + ":" + std::to_string(column + 1)
         + "\n" color_RESET;
}

Error_Diagnostic::Error_Diagnostic(cu::ID _cuid, ErrorCode _code, ast::ID nodeid, compiler::EPhase _phase,
                                   std::string_view _msg, std::string_view _hint)
  : elem_first(_cuid, _code, nodeid, _phase, _msg, _hint)
  , elem_second({}, 0, 0, 0, _phase, "", "")
{
  assert(nodeid && "Invalid node id");
  assert(_cuid && "Illegal error on unknown script");
}

Error_Diagnostic::Error_Diagnostic(cu::ID _cuid, ErrorCode _code, ast::ID nodeid1, ast::ID nodeid2,
                                   compiler::EPhase _phase, std::string_view _msg, std::string_view _hint)
  : elem_first(_cuid, _code, nodeid1, _phase, _msg, _hint)
  , elem_second(nodeid2.cu(), _code, nodeid2, _phase, "", "")
{
  assert(nodeid1 && "Invalid node id");
  assert(nodeid2 && "Invalid node id");
  assert(_cuid && "Illegal error on unknown script");
}

std::string Error_Diagnostic::print_error() const noexcept
{
  static const auto mode = compiler::OPTIONS.diagnostic.out_format;
  switch (mode) {
  case common::compiler::EDiagnosticFormat::DEFAULT:
  case common::compiler::EDiagnosticFormat::userfriendly: return print_userfriendly_error();
  case common::compiler::EDiagnosticFormat::json:         return print_json_error();
  case common::compiler::EDiagnosticFormat::github:       return print_github_error();
  }
}

std::string Error_Diagnostic::print_userfriendly_error() const noexcept
{
  const auto& first_script = elem_first.cuid.get();

  // one file
  if (!elem_second.cuid
      || (elem_first.cuid == elem_second.cuid && elem_first.start_pos == elem_second.start_pos
          && elem_first.end_pos == elem_second.end_pos)) {
    return elem_first.print_error();
  }

  const auto& second_script = elem_second.cuid.get();

  size_t first_line  = first_script.file_info.get_line_from_pos(elem_first.start_pos);
  size_t second_line = second_script.file_info.get_line_from_pos(elem_second.start_pos);


  // same file, same line
  if (elem_first.cuid == elem_second.cuid && first_line == second_line) {
    auto line       = elem_first.print_line();
    auto top_cursor = elem_first.print_cursor();
    std::ranges::replace(top_cursor, '^', 'v');
    auto down_cursor = elem_second.print_cursor();

    std::string out;
    out += "       | " color_RED + top_cursor + color_RESET "\n";
    out += line;
    out += "       | " color_RED + down_cursor + color_RESET "\n";

    out += elem_first.print_messages() + "\n";
    return out;
  }

  // same file
  if (elem_first.cuid == elem_second.cuid) {
    auto top_line   = elem_first.print_line();
    auto top_cursor = elem_first.print_cursor();
    std::ranges::replace(top_cursor, '^', 'v');
    auto down_line   = elem_second.print_line();
    auto down_cursor = elem_second.print_cursor();

    // lower line count first
    if (top_line > down_line) {
      auto tmp_line = top_line;
      top_line      = down_line;
      down_line     = tmp_line;
      auto tmp_cur  = top_cursor;
      top_cursor    = down_cursor;
      down_cursor   = tmp_cur;
      std::ranges::replace(top_cursor, '^', 'v');
      std::ranges::replace(down_cursor, 'v', '^');
    }

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
  std::ranges::replace(top_cursor, '^', 'v');
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

constexpr std::string_view JSON_ERROR_TEMPLATE =
    R"({"type":"%type","code":"%code","message":"%message","hint":"%hint","file":"%file","start_pos":%start_pos,"end_pos":%end_pos})";
constexpr std::string_view JSON_MULTIPLEFILES_ERROR_TEMPLATE =
    R"({"type":"%type","code":"%code","message":"%message","hint":"%hint","file":"%file","start_pos":%start_pos,"end_pos":%end_pos,"related":[%related]})";
constexpr std::string_view JSON_RELATED_ERROR_TEMPLATE =
    R"({"file":"%file","start_pos":%start_pos,"end_pos":%end_pos,"message":"%message"})";

std::string Error_Diagnostic::print_json_error() const noexcept
{
  const auto& first_script = elem_first.cuid.get();

  const std::string code =
      compiler::Phase_to_code(elem_first.error_info.phase) + std::format("{:04}", elem_first.error_info.code);
  const auto first_start_pos = std::to_string(elem_first.start_pos);
  const auto first_end_pos   = std::to_string(elem_first.end_pos);


  const auto msg  = escape_json(elem_first.error_info.msg);
  const auto hint = escape_json(elem_first.error_info.hint);


  // one error domain
  if (!elem_second.cuid
      || (elem_first.cuid == elem_second.cuid && elem_first.start_pos == elem_second.start_pos
          && elem_first.end_pos == elem_second.end_pos)) {
    auto str = std::string(JSON_ERROR_TEMPLATE);

    std::map<std::string_view, std::string_view> data = {
        {"type",      "error"                             },
        {"code",      code                                },
        {"message",   msg                                 },
        {"hint",      hint                                },
        {"file",      elem_first.cuid.get().file_info.path},
        {"start_pos", first_start_pos                     },
        {"end_pos",   first_end_pos                       },
    };
    common::utils::fmt_template(str, data);

    return str;
  }

  // two errors domains
  const auto& second_script = elem_second.cuid.get();

  size_t first_line  = first_script.file_info.get_line_from_pos(elem_first.start_pos);
  size_t second_line = second_script.file_info.get_line_from_pos(elem_second.start_pos);

  const auto second_start_pos = std::to_string(elem_second.start_pos);
  const auto second_end_pos   = std::to_string(elem_second.end_pos);


  auto str         = std::string(JSON_MULTIPLEFILES_ERROR_TEMPLATE);
  auto str_related = std::string(JSON_RELATED_ERROR_TEMPLATE);

  std::map<std::string_view, std::string_view> err_related = {
      {"file",      elem_second.cuid.get().file_info.path},
      {"start_pos", second_start_pos                     },
      {"end_pos",   second_end_pos                       },
      {"message",   ""                                   },
  };
  common::utils::fmt_template(str_related, err_related);

  std::map<std::string_view, std::string_view> err_data = {
      {"type",      "error"                             },
      {"code",      code                                },
      {"message",   msg                                 },
      {"hint",      hint                                },
      {"file",      elem_first.cuid.get().file_info.path},
      {"start_pos", first_start_pos                     },
      {"end_pos",   first_end_pos                       },
      {"related",   str_related                         },
  };
  common::utils::fmt_template(str, err_data);

  return str;
}

constexpr std::string_view GITHUB_ERROR_TEMPLATE =
    R"({::%type file=%file,line=%line,col=%column,endLine=%endLine,endColumn=%endColumn,title=%title::%msg})";

std::string Error_Diagnostic::print_github_error() const noexcept
{
  auto        str = std::string(GITHUB_ERROR_TEMPLATE);
  const auto& cu  = elem_first.cuid.get();

  auto line_pos = cu.file_info.get_line_from_pos(elem_first.start_pos);

  const int start_col =
      int(elem_first.end_pos) - cu.file_info.get_line_start(line_pos) - (elem_first.end_pos - elem_first.start_pos);
  int end_col = start_col + (elem_first.end_pos - elem_first.start_pos);

  const auto        line          = std::to_string(line_pos);
  const auto        str_start_col = std::to_string(start_col);
  const auto        str_end_col   = std::to_string(end_col);
  const std::string code =
      compiler::Phase_to_code(elem_first.error_info.phase) + std::format("{:04}", elem_first.error_info.code);


  const auto msg = escape_json(elem_first.error_info.msg);

  std::map<std::string_view, std::string_view> data = {
      {"type",      "error"                              },
      {"file",      elem_second.cuid.get().file_info.path},
      {"line",      line                                 },
      {"col",       str_start_col                        },
      {"endLine",   line                                 },
      {"endColumn", str_end_col                          },
      {"title",     code                                 },
      {"msg",       msg                                  },
  };
  common::utils::fmt_template(str, data);

  return str;
}