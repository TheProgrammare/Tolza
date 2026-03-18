#pragma once

#include <string>

class StreamTracker
{
  const char* start;
  const char* end;
  const char* cur;

  std::string text;

  size_t line     = 1;
  size_t column   = 0;
  char   lastChar = '\0';

public:
  StreamTracker(const std::string& s)
    : text(s)
    , start(s.begin().base())
    , end(s.end().base())
    , cur(start)
  {
  }

  // Read char and update line and column
  bool get(char& c)
  {
    if (cur == end) return false;
    c = *cur;
    cur++;
    if (lastChar == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }
    lastChar = c;
    return true;
  }

  char peek()
  {
    if (cur == end) return EOF;
    return *cur;
  }

  size_t get_line() const
  {
    return line;
  }
  size_t get_column() const
  {
    return column;
  }

  void go_back()
  {
    if (cur == start) return;

    cur--;
    if (*cur == '\n') {
      line--;
      column = 1;
    } else if (!is_ctrl(*cur)) {
      column--;
      if (column < 1) column = 1;
    }
  }

  char get_last_ch()
  {
    return lastChar;
  }

private:
  bool is_ctrl(unsigned char c) noexcept
  {
    return (c < 32 || c == 127);
  }
};
