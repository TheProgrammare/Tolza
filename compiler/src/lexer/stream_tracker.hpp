#pragma once

#include <cstddef>
#include <initializer_list>
#include <string>
#include <string_view>

class StreamTracker
{
  const std::string text;

  size_t cur = 0;

public:
  StreamTracker(const std::string_view& s)
    : text(std::string(s))
  {
  }

  [[nodiscard]] std::string_view data() const noexcept
  {
    return text;
  }

  [[nodiscard]] bool is_end() const noexcept
  {
    return cur >= text.size();
  }

  [[nodiscard]] char at(size_t pos) const noexcept
  {
    return text.at(pos);
  }


  [[nodiscard]] char peek(size_t offset = 0) const noexcept
  {
    if (cur + offset >= text.size()) return '\0';
    return text.at(cur + offset);
  }

  // Read char and update line and column
  [[nodiscard]] bool next() noexcept
  {
    if (cur >= text.size()) return false;

    cur++;
    return true;
  }

  [[nodiscard]] bool match(char c) noexcept
  {
    return peek() == c && next();
  }

  [[nodiscard]] size_t position() const noexcept
  {
    return cur;
  }

  [[nodiscard]] bool check(char c) const noexcept
  {
    return peek() == c;
  }

  [[nodiscard]] bool check_at(size_t offset, char c) const noexcept
  {
    return peek(offset) == c;
  }

  [[nodiscard]] bool check_chain(const std::initializer_list<char>& l) const noexcept
  {
    for (size_t i = 0; i < l.size(); i++) {
      const auto c = *(l.begin() + i);
      if (!check_at(i, c)) return false;
    }

    return true;
  }

  [[nodiscard]] bool match_chain(const std::initializer_list<char>& l) noexcept
  {
    const auto result = check_chain(l);
    if (result) (void)jump(position() + l.size());

    return result;
  }


  [[nodiscard]] bool jump(size_t pos) noexcept
  {
    if (pos > text.size()) return false;
    cur = pos;
    return true;
  }

  void go_back() noexcept
  {
    if (cur == 0) return;

    --cur;
  }
};
