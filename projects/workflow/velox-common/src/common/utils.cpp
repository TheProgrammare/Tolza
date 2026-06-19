#include "utils.hpp"


std::vector<std::string> common::utils::split_flags(std::string_view s, char separator) noexcept
{
  std::vector<std::string> out;

  size_t start = 0;

  while (start < s.size()) {
    size_t end = s.find(separator, start);
    if (end == std::string_view::npos) end = s.size();

    out.emplace_back(s.substr(start, end - start));

    start = end + 1;
  }

  return out;
}

bool common::utils::is_valid_identifier(std::string_view s, bool path_possible) noexcept
{
  if (s.empty()) return false;
  if (s.contains(' ')) return false;

  auto is_valid_single = [](std::string_view id) -> bool {
    if (id.empty()) return false;

    if (!is_alpha(id[0]) && id[0] != '_') return false;

    for (char c : id) {
      if (!is_alnum(c) && c != '_') return false;
    }

    return true;
  };

  if (!path_possible) return is_valid_single(s);

  // path mode : split on "::"
  std::size_t start = 0;

  while (start < s.size()) {
    std::size_t pos = s.find("::", start);

    std::string_view part;

    if (pos == std::string_view::npos) {
      part  = s.substr(start);
      start = s.size();
    } else {
      part  = s.substr(start, pos - start);
      start = pos + 2;
    }

    // reject empty segments (e.g. "::a", "a::", "a::::b")
    if (!is_valid_single(part)) {
      return false;
    }
  }

  return true;
}


void common::utils::fmt_template(std::string& template_str, const std::initializer_list<std::string>& args) noexcept
{
  size_t count = 0;
  for (const auto& arg : args) {
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}

void common::utils::fmt_template(std::string&                                   template_str,
                                 const std::initializer_list<std::string_view>& args) noexcept
{
  size_t count = 0;
  for (const auto& arg : args) {
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}

void common::utils::fmt_template(std::string&                                        template_str,
                                 const std::map<std::string_view, std::string_view>& args) noexcept
{
  for (const auto& [key, val] : args) {
    std::string placeholder = "%" + std::string(key);
    size_t      pos         = 0;
    while ((pos = template_str.find(placeholder, pos)) != std::string::npos) {
      template_str.replace(pos, placeholder.length(), val);
      pos += key.length();
    }
  }
}
