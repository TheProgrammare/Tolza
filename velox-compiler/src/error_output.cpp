#include "error_output.hpp"

#include <sstream>
#include <iomanip>

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
