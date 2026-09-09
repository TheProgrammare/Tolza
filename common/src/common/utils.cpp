#include "utils.hpp"

#include "common/compiler_options.hpp"
#include "common/forward.hpp"

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <set>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


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

void common::utils::fmt_template(
    std::string& s, const std::initializer_list<std::pair<std::string_view, std::string_view>>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string_view key) -> const std::string_view* {
    const auto* const it = std::ranges::find_if(args, [&key](const auto& pair) { return pair.first == key; });

    return it != args.end() ? &it->second : nullptr;
  });
}

void common::utils::fmt_template(std::string&                                                      s,
                                 const std::initializer_list<std::pair<std::string, std::string>>& args) noexcept
{
  fmt_template_impl(s, [&args](const std::string& key) -> const std::string* {
    const auto* const it = std::ranges::find_if(args, [&key](const auto& pair) { return pair.first == key; });

    return it != args.end() ? &it->second : nullptr;
  });
}


void common::utils::merge_list_cstr(std::vector<const char*>& _dest, const std::vector<const char*>& _val,
                                    compiler::EMergeMode mode) noexcept
{
  switch (mode) {
  case compiler::EMergeMode::NONE:
  case compiler::EMergeMode::_union: {
    _dest.insert(_dest.end(), _val.begin(), _val.end());
    break;
  }
  case compiler::EMergeMode::_override:     _dest = _val;
  case compiler::EMergeMode::_intersection: {
    std::vector<const char*> tmp;
    tmp.reserve(_dest.size());
    auto tmp_set = std::set<const char*>(_val.begin(), _val.end());
    for (const auto& elem : _dest) {
      if (tmp_set.find(elem) != tmp_set.end()) tmp.emplace_back(elem);
    }
    _dest = tmp;
    break;
  }
  case compiler::EMergeMode::_anti_intersection: {
    std::vector<const char*> tmp;
    tmp.reserve(_dest.size());
    auto tmp_set = std::set<const char*>(_val.begin(), _val.end());
    for (const auto& elem : _dest) {
      if (tmp_set.find(elem) == tmp_set.end()) tmp.emplace_back(elem);
    }
    _dest = tmp;
    break;
  }
  }
}
void common::utils::merge_list_str(std::vector<std::string>& _dest, const std::vector<std::string>& _val,
                                   compiler::EMergeMode mode) noexcept
{
  switch (mode) {
  case compiler::EMergeMode::NONE:
  case compiler::EMergeMode::_union: {
    _dest.insert(_dest.end(), _val.begin(), _val.end());
    break;
  }
  case compiler::EMergeMode::_override:     _dest = _val;
  case compiler::EMergeMode::_intersection: {
    std::vector<std::string> tmp;
    tmp.reserve(_dest.size());
    auto tmp_set = std::set<std::string_view>(_val.begin(), _val.end());
    for (const auto& elem : _dest) {
      if (tmp_set.find(elem) != tmp_set.end()) tmp.emplace_back(elem);
    }
    _dest = tmp;
    break;
  }
  case compiler::EMergeMode::_anti_intersection: {
    std::vector<std::string> tmp;
    tmp.reserve(_dest.size());
    auto tmp_set = std::set<std::string_view>(_val.begin(), _val.end());
    for (const auto& elem : _dest) {
      if (tmp_set.find(elem) == tmp_set.end()) tmp.emplace_back(elem);
    }
    _dest = tmp;
    break;
  }
  }
}

void common::utils::merge_map(std::vector<std::pair<std::string, std::string>>&       _dest,
                              const std::vector<std::pair<std::string, std::string>>& _val,
                              compiler::EMergeMode                                    mode) noexcept
{
  using Pair = std::pair<std::string, std::string>;

  // A value starting with '!' means that the corresponding key
  // must be removed from the destination.
  auto is_remove = [](const Pair& p) { return !p.second.empty() && p.second.front() == '!'; };

  auto key_exists = [](const std::vector<Pair>& vec, const std::string& key) {
    return std::ranges::find_if(vec, [&](const Pair& p) { return p.first == key; }) != vec.end();
  };

  auto find_key = [](std::vector<Pair>& vec, const std::string& key) {
    return std::ranges::find_if(vec, [&](const Pair& p) { return p.first == key; });
  };

  switch (mode) {
  case compiler::EMergeMode::NONE:
    // Do nothing.
    break;

  case compiler::EMergeMode::_union: {
    for (const auto& p : _val) {
      auto it = find_key(_dest, p.first);

      if (is_remove(p)) {
        // Explicitly remove the key from the destination.
        _dest.erase(std::remove_if(_dest.begin(), _dest.end(), [&](const Pair& d) { return d.first == p.first; }),
                    _dest.end());
        continue;
      }

      if (it == _dest.end()) {
        // Add the key if it does not already exist.
        _dest.push_back(p);
      } else {
        // Override the existing value.
        it->second = p.second;
      }
    }
    break;
  }

  case compiler::EMergeMode::_override: {
    // Remove keys explicitly marked with '!'.
    for (const auto& p : _val) {
      if (!is_remove(p)) continue;

      _dest.erase(std::remove_if(_dest.begin(), _dest.end(), [&](const Pair& d) { return d.first == p.first; }),
                  _dest.end());
    }

    // Replace the destination with the non-removal entries.
    std::vector<Pair> result;
    result.reserve(_val.size());

    for (const auto& p : _val) {
      if (!is_remove(p)) result.push_back(p);
    }

    _dest = std::move(result);
    break;
  }

  case compiler::EMergeMode::_intersection: {
    // Keep only keys that exist in both maps.
    _dest.erase(std::remove_if(_dest.begin(), _dest.end(),
                               [&](const Pair& d) {
                                 auto it = find_key(const_cast<std::vector<Pair>&>(_val), d.first);

                                 if (it == _val.end()) return true;

                                 // An explicit removal means that the key
                                 // is not part of the effective intersection.
                                 return is_remove(*it);
                               }),
                _dest.end());

    break;
  }

  case compiler::EMergeMode::_anti_intersection: {
    // Keep only keys that exist in the destination but not in _val.
    _dest.erase(std::remove_if(_dest.begin(), _dest.end(), [&](const Pair& d) { return key_exists(_val, d.first); }),
                _dest.end());

    break;
  }
  }
}
