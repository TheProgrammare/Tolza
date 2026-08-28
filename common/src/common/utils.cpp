#include "utils.hpp"
#include "common/environment.hpp"


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


namespace
{
constexpr bool is_key_char(const char c) noexcept
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '.' || c == '-';
}

template <typename Lookup>
void fmt_template_impl(std::string& s, Lookup&& lookup) noexcept
{
  const size_t size = s.size();
  if (size < 2 || s.find('%') == std::string::npos) return;

  std::string out;
  out.reserve(size);

  const char* data = s.data();
  size_t      pos  = 0;
  size_t      copy = 0;

  while (pos < size) {
    const size_t percent = s.find('%', pos);
    if (percent == std::string::npos) break;

    const size_t begin = percent + 1;
    if (begin >= size) {
      out.append(data + copy, percent - copy + 1);
      copy = size;
      break;
    }

    size_t end = begin;
    while (end < size && is_key_char(data[end])) ++end;

    if (end == begin) {
      pos = begin;
      continue;
    }

    const std::string key(data + begin, end - begin);

    if (const auto* value = lookup(key)) {
      out.append(data + copy, percent - copy);
      out.append(*value);
      copy = end;
    }

    pos = end;
  }

  if (copy < size) out.append(data + copy, size - copy);

  s = std::move(out);
}
} // namespace

void common::utils::fmt_template(std::string& s, const std::initializer_list<std::string>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string_view key) -> const std::string* {
    if (key.empty()) return nullptr;

    size_t index = 0;
    for (const auto& arg : args) {
      if (std::to_string(index++) == key) return &arg;
    }

    return nullptr;
  });
}

void common::utils::fmt_template(std::string& s, const std::initializer_list<std::string_view>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string_view key) -> const std::string_view* {
    if (key.empty()) return nullptr;

    size_t index = 0;
    for (const auto& arg : args) {
      // Matching exact de l'index sans construire "%N".
      size_t n      = index++;
      size_t digits = 1;
      for (size_t x = n; x >= 10; x /= 10) ++digits;

      if (digits != key.size()) continue;

      bool match = true;
      for (size_t i = digits; i-- > 0; n /= 10) {
        if (key[i] != static_cast<char>('0' + (n % 10))) {
          match = false;
          break;
        }
      }

      if (match) return &arg;
    }

    return nullptr;
  });
}

void common::utils::fmt_template(std::string& s, const std::map<std::string_view, std::string_view>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string_view key) -> const std::string_view* {
    if (const auto it = args.find(key); it != args.end()) return &it->second;

    return nullptr;
  });
}

void common::utils::fmt_template(std::string& s, const std::map<std::string, std::string>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string& key) -> const std::string* {
    if (const auto it = args.find(key); it != args.end()) return &it->second;

    return nullptr;
  });
}
