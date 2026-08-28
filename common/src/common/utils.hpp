#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>


#define GET_ENUM_NAME(_enum)   std::string(magic_enum::enum_name(_enum))
#define GET_FLAGS_NAME(_flags) std::string(magic_enum::enum_flags_name(_flags, '|'))


#define STR_TO_ENUM(_input, _enum)                                                                                     \
  magic_enum::enum_cast<_enum>(common::utils::str_to_snake(_input), magic_enum::case_insensitive)
#define STR_TO_FLAGS(_input, _flags)                                                                                   \
  magic_enum::enum_flags_cast<_flags>(common::utils::str_to_snake(_input), '|', magic_enum::case_insensitive)


namespace common::utils
{


[[nodiscard]] std::vector<std::string> split_flags(std::string_view s, char separator = '|') noexcept;

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

static std::string str_to_kebab(std::string_view s) noexcept
{
  std::string out;
  out.reserve(s.size());

  for (char c : s) {
    if (c == '_')
      out.push_back('-');
    else
      out.push_back(c);
  }

  return out;
}

static std::string str_to_snake(std::string_view s) noexcept
{
  std::string out;
  out.reserve(s.size());

  for (char c : s) {
    if (c == '-')
      out.push_back('_');
    else
      out.push_back(c);
  }

  return out;
}


void fmt_template(std::string& template_str, const std::initializer_list<std::string>& args) noexcept;
void fmt_template(std::string& template_str, const std::initializer_list<std::string_view>& args) noexcept;
void fmt_template(std::string& template_str, const std::map<std::string_view, std::string_view>& args) noexcept;
void fmt_template(std::string& template_str, const std::map<std::string, std::string>& args) noexcept;


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
