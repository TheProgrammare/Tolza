#pragma once

#include <cstddef>
#include <string_view>
#include <string>

class StreamTracker
{
  const std::string text;

  size_t cur = 0;

#ifdef DEBUG
  char cur_str;
#endif

public:
  StreamTracker(const std::string_view& s)
    : text(std::string(s))
    , cur(0)
  {
  }

  std::string_view data() const noexcept
  {
    return text;
  }

  bool is_end() const noexcept
  {
    return cur >= text.size();
  }

  char peek(size_t offset = 0) const noexcept
  {
    if (cur + offset >= text.size()) return '\0';
    return text.at(cur + offset);
  }

  // Read char and update line and column
  bool next() noexcept
  {
    if (cur >= text.size()) return false;

    cur++;
#ifdef DEBUG
    update_debug_cur_str();
#endif

    return true;
  }

  bool match(char c) noexcept
  {
    return peek() == c && next();
  }

  size_t position() const noexcept
  {
    return cur;
  }

  bool check(char c) const noexcept
  {
    return peek() == c;
  }


  bool jump(size_t pos) noexcept
  {
    if (pos > text.size()) return false;
    cur = pos;
#ifdef DEBUG
    update_debug_cur_str();
#endif
    return true;
  }

  void go_back() noexcept
  {
    if (cur == 0) return;

    --cur;
#ifdef DEBUG
    update_debug_cur_str();
#endif
  }

private:
  static bool is_ctrl(unsigned char c) noexcept
  {
    return (c < 32 || c == 127);
  }

#ifdef DEBUG
  void update_debug_cur_str()
  {
    if (cur >= text.size())
      cur_str = text.back();
    else
      cur_str = text.at(cur);
  }
#endif
};
