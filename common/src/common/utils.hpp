#pragma once

#include "common/forward.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace common::utils
{


[[nodiscard]] bool is_valid_identifier(std::string_view s, bool path_possible = false) noexcept;

[[nodiscard]] inline bool is_space(unsigned char c) noexcept
{
  return (c == ' ' || (c >= '\t' && c <= '\r'));
}

[[nodiscard]] inline bool is_ctrl(unsigned char c) noexcept
{
  return (c < 32 || c == 127);
}

[[nodiscard]] inline bool is_digit(unsigned char c) noexcept
{
  return c >= '0' && c <= '9';
}

[[nodiscard]] inline bool is_hex(unsigned char c) noexcept
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}


[[nodiscard]] inline int hex_value(char c) noexcept
{
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
  if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
  return -1; // invalid
}


[[nodiscard]] inline bool is_alnum(unsigned char c) noexcept
{
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

[[nodiscard]] inline bool is_alpha(unsigned char c) noexcept
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}


void fmt_template(std::string& s, const std::initializer_list<std::string>& args) noexcept;
void fmt_template(std::string& s, const std::initializer_list<std::string_view>& args) noexcept;
void fmt_template(std::string&                                                                s,
                  const std::initializer_list<std::pair<std::string_view, std::string_view>>& args) noexcept;
void fmt_template(std::string& s, const std::initializer_list<std::pair<std::string, std::string>>& args) noexcept;


void merge_list_cstr(std::vector<const char*>& _dest, const std::vector<const char*>& _val,
                     compiler::EMergeMode mode) noexcept;
void merge_list_str(std::vector<std::string>& _dest, const std::vector<std::string>& _val,
                    compiler::EMergeMode mode) noexcept;
void merge_map(std::vector<std::pair<std::string, std::string>>&       _dest,
               const std::vector<std::pair<std::string, std::string>>& _val, compiler::EMergeMode mode) noexcept;


struct FastRNG {
  uint64_t state;

  explicit FastRNG(uint64_t seed = 0x123456789abcdef0ULL)
    : state(seed)
  {
  }

  [[nodiscard]] uint64_t next() noexcept
  {
    uint64_t x = state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    state = x;
    return x * 2685821657736338717ULL;
  }

  [[nodiscard]] size_t next_size_t(size_t min, size_t max) noexcept
  {
    return min + (next() % (max - min + 1));
  }
};

extern FastRNG RAND;

} // namespace common::utils
