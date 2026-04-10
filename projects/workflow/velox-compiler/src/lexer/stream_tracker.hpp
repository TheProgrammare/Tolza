#pragma once

#include <cstddef>
#include <string>

class StreamTracker
{
  std::string text;

  const char* const start;
  const char* const end;
  const char*       cur;


  size_t line   = 1;
  size_t column = 1;

public:
  StreamTracker(const std::string& s)
    : text(s)
    , start(s.data())
    , end(s.data() + s.size())
    , cur(start)
  {
  }

  bool is_end() const noexcept
  {
    return cur >= end;
  }

  char peek(size_t offset = 0) const noexcept
  {
    if (cur + offset >= end) return '\0';
    return *(cur + offset);
  }

  // Read char and update line and column
  bool next() noexcept
  {
    if (cur >= end) return false;

    char c = *cur++;

    if (c == '\n') {
      ++line;
      column = 1;
    } else {
      ++column;
    }

    return true;
  }

  bool match(char c) noexcept
  {
    return peek() == c && next();
  }
  size_t get_line() const noexcept
  {
    return line;
  }
  size_t get_column() const noexcept
  {
    return column;
  }

  size_t position() const noexcept
  {
    return cur - start;
  }

  bool check(char c) const noexcept
  {
    return peek() == c;
  }


  bool jump(size_t pos) noexcept
  {
    if (pos > end - start) return false;
    cur = start + pos;
    recompute_line_column();
    return true;
  }

  void go_back() noexcept
  {
    if (cur == start) return;

    --cur;

    if (*cur == '\n') {
      recompute_line_column();
    } else if (column < 1)
      --column;
  }

private:
  static bool is_ctrl(unsigned char c) noexcept
  {
    return (c < 32 || c == 127);
  }

  void recompute_line_column() noexcept
  {
    line   = 1;
    column = 1;

    for (auto p = start; p < cur; ++p) {
      if (*p == '\n') {
        ++line;
        column = 1;
      } else {
        ++column;
      }
    }
  }
};
