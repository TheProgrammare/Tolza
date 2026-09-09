#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <string_view>


// ============================================================================
// enum_lite - C++23, header-only, zero dependency
// ============================================================================

namespace enum_lite
{

// ============================================================================
// Static string list
// ============================================================================

template <std::size_t N>
struct static_string_list {
  std::array<std::string_view, N> storage{};
  std::size_t                     size = 0;

  constexpr std::size_t count() const noexcept
  {
    return size;
  }

  constexpr bool empty() const noexcept
  {
    return size == 0;
  }

  constexpr std::string_view operator[](std::size_t index) const noexcept
  {
    return storage[index];
  }

  constexpr auto begin() const noexcept
  {
    return storage.begin();
  }

  constexpr auto end() const noexcept
  {
    return storage.begin() + size;
  }

  constexpr operator std::span<const std::string_view>() const noexcept
  {
    return {storage.data(), size};
  }
};


// ============================================================================
// String normalization
// ============================================================================

constexpr char to_lower_ascii(char c) noexcept
{
  if (c >= 'A' && c <= 'Z') return static_cast<char>(c + ('a' - 'A'));

  return c;
}


constexpr std::string_view strip_enum_suffix(std::string_view name) noexcept
{
  // A trailing '_' is commonly used to avoid C++ keyword collisions:
  //
  //   enum class Foo { class_ };
  //
  // The exposed name is simply "class".

  if (!name.empty() && name.back() == '_') name.remove_suffix(1);

  return name;
}


constexpr std::string_view trim(std::string_view str) noexcept
{
  while (!str.empty() && (str.front() == ' ' || str.front() == '\t')) {
    str.remove_prefix(1);
  }

  while (!str.empty() && (str.back() == ' ' || str.back() == '\t')) {
    str.remove_suffix(1);
  }

  return str;
}


constexpr bool normalized_equal(std::string_view input, std::string_view name) noexcept
{
  // "_foo" matches "foo".
  if (!input.empty() && input.front() == '_') input.remove_prefix(1);

  // "foo_" is exposed as "foo".
  name = strip_enum_suffix(name);

  if (input.size() != name.size()) return false;

  for (std::size_t i = 0; i < input.size(); ++i) {
    char lhs = input[i];
    char rhs = name[i];

    // '-' and '_' are equivalent.
    if (lhs == '-') lhs = '_';

    if (rhs == '-') rhs = '_';

    if (to_lower_ascii(lhs) != to_lower_ascii(rhs)) return false;
  }

  return true;
}


// ============================================================================
// Preprocessor expansion
// ============================================================================
//
// The recursion itself is deliberately kept simple.  PP_EXPAND is only here
// because MSVC/GCC/Clang otherwise differ in how aggressively variadic macro
// arguments are expanded.
//

#define ENUM_LITE_PP_PARENS ()

#define ENUM_LITE_PP_EXPAND(...) ENUM_LITE_PP_EXPAND_1(__VA_ARGS__)

#define ENUM_LITE_PP_EXPAND_1(...)                                                                                     \
  ENUM_LITE_PP_EXPAND_2(ENUM_LITE_PP_EXPAND_2(ENUM_LITE_PP_EXPAND_2(ENUM_LITE_PP_EXPAND_2(__VA_ARGS__))))

#define ENUM_LITE_PP_EXPAND_2(...)                                                                                     \
  ENUM_LITE_PP_EXPAND_3(ENUM_LITE_PP_EXPAND_3(ENUM_LITE_PP_EXPAND_3(ENUM_LITE_PP_EXPAND_3(__VA_ARGS__))))

#define ENUM_LITE_PP_EXPAND_3(...)                                                                                     \
  ENUM_LITE_PP_EXPAND_4(ENUM_LITE_PP_EXPAND_4(ENUM_LITE_PP_EXPAND_4(ENUM_LITE_PP_EXPAND_4(__VA_ARGS__))))

#define ENUM_LITE_PP_EXPAND_4(...)                                                                                     \
  ENUM_LITE_PP_EXPAND_5(ENUM_LITE_PP_EXPAND_5(ENUM_LITE_PP_EXPAND_5(ENUM_LITE_PP_EXPAND_5(__VA_ARGS__))))

#define ENUM_LITE_PP_EXPAND_5(...) __VA_ARGS__


// ============================================================================
// Pair iteration
//
// Input:
//
//   NAME, VALUE, NAME, VALUE, NAME, VALUE
//
// Example:
//
//   RED, 1, GREEN, 2, BLUE, 4
// ============================================================================

#define ENUM_LITE_PP_FOR_EACH_PAIR(macro, ...)                                                                         \
  __VA_OPT__(ENUM_LITE_PP_EXPAND(ENUM_LITE_PP_FOR_EACH_PAIR_I(macro, __VA_ARGS__)))

#define ENUM_LITE_PP_FOR_EACH_PAIR_I(macro, name, value, ...)                                                          \
  macro(name, value) __VA_OPT__(ENUM_LITE_PP_FOR_EACH_PAIR_AGAIN ENUM_LITE_PP_PARENS(macro, __VA_ARGS__))

#define ENUM_LITE_PP_FOR_EACH_PAIR_AGAIN() ENUM_LITE_PP_FOR_EACH_PAIR_I


// ============================================================================
// Pair iteration with context
//
//   macro(Type, name, value)
// ============================================================================

#define ENUM_LITE_PP_FOR_EACH_PAIR_CTX(macro, context, ...)                                                            \
  __VA_OPT__(ENUM_LITE_PP_EXPAND(ENUM_LITE_PP_FOR_EACH_PAIR_CTX_I(macro, context, __VA_ARGS__)))

#define ENUM_LITE_PP_FOR_EACH_PAIR_CTX_I(macro, context, name, value, ...)                                             \
  macro(context, name, value)                                                                                          \
      __VA_OPT__(ENUM_LITE_PP_FOR_EACH_PAIR_CTX_AGAIN ENUM_LITE_PP_PARENS(macro, context, __VA_ARGS__))

#define ENUM_LITE_PP_FOR_EACH_PAIR_CTX_AGAIN() ENUM_LITE_PP_FOR_EACH_PAIR_CTX_I


// ============================================================================
// Regular enum generation
// ============================================================================

#define ENUM_LITE_DETAIL_DECLARE(name, value) name = value,

#define ENUM_LITE_DETAIL_NAME(name, value) std::string_view{#name},

#define ENUM_LITE_DETAIL_VALUES(name, value) ", " #name

#define ENUM_LITE_DETAIL_TO_STRING(Type, name, value)                                                                  \
  case Type::name: return enum_lite::strip_enum_suffix(#name);

#define ENUM_LITE_DETAIL_FROM_STRING(Type, name, value)                                                                \
  if (enum_lite::normalized_equal(str, #name)) return Type::name;

#define ENUM_LITE_DETAIL_FLAG_IS_VALID_STRING(Type, name, value)                                                       \
  if (enum_lite::normalized_equal(token, #name)) known = true;

// ============================================================================
// Regular enum
// ============================================================================

#define ENUM_LITE_DEFINE_ENUM(Type, Underlying, ...)                                                                   \
  enum class Type : Underlying { NONE = 0, ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_DECLARE, __VA_ARGS__) };        \
                                                                                                                       \
  constexpr auto Type##_names =                                                                                        \
      std::array{std::string_view{"NONE"}, ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_NAME, __VA_ARGS__)};            \
                                                                                                                       \
  constexpr std::string_view Type##_values = "NONE" ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_VALUES, __VA_ARGS__);  \
                                                                                                                       \
  constexpr std::string_view Type##_to_str(Type value) noexcept                                                        \
  {                                                                                                                    \
    switch (value) {                                                                                                   \
    case Type::NONE: return {}; ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_TO_STRING, Type, __VA_ARGS__)          \
    }                                                                                                                  \
                                                                                                                       \
    return {};                                                                                                         \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type Type##_from_str(std::string_view str) noexcept                                                        \
  {                                                                                                                    \
    ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_FROM_STRING, Type, __VA_ARGS__)                                    \
                                                                                                                       \
    return Type::NONE;                                                                                                 \
  }                                                                                                                    \
                                                                                                                       \
  constexpr bool Type##_is_valid(std::string_view str) noexcept                                                        \
  {                                                                                                                    \
    if (enum_lite::normalized_equal(str, "NONE")) return true;                                                         \
                                                                                                                       \
    ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_IS_VALID_STRING, Type, __VA_ARGS__)                                \
                                                                                                                       \
    return false;                                                                                                      \
  }


// ============================================================================
// Flags generation
// ============================================================================

#define ENUM_LITE_DETAIL_FLAG_OR_VALUE(name, value) | value

#define ENUM_LITE_DETAIL_FLAG_VALUE_STRING(name, value) "|" #name

#define ENUM_LITE_DETAIL_FLAG_TO_STRING(Type, name, val)                                                               \
  if (Type##_has_flag(value, Type::name)) {                                                                            \
    if (!result.empty()) result += separator;                                                                          \
                                                                                                                       \
    result += enum_lite::strip_enum_suffix(#name);                                                                     \
  }

#define ENUM_LITE_DETAIL_FLAG_TO_VECTOR(Type, name, val)                                                               \
  if (Type##_has_flag(value, Type::name)) result.storage[result.size++] = enum_lite::strip_enum_suffix(#name);

#define ENUM_LITE_DETAIL_FLAG_FROM_STRING(Type, name, value)                                                           \
  if (enum_lite::normalized_equal(token, #name)) result |= Type::name;

#define ENUM_LITE_DETAIL_IS_VALID_STRING(Type, name, value)                                                            \
  if (enum_lite::normalized_equal(str, #name)) return true;


// ============================================================================
// Flags
// ============================================================================

#define ENUM_LITE_DEFINE_FLAGS(Type, Underlying, ...)                                                                  \
  enum class Type : Underlying {                                                                                       \
    NONE = 0,                                                                                                          \
    ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_DECLARE, __VA_ARGS__) ALL =                                            \
        0 ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_FLAG_OR_VALUE, __VA_ARGS__)                                      \
  };                                                                                                                   \
                                                                                                                       \
  constexpr auto Type##_names =                                                                                        \
      std::array{std::string_view{"NONE"},                                                                             \
                 ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_NAME, __VA_ARGS__) std::string_view{"ALL"}};              \
                                                                                                                       \
  constexpr std::string_view Type##_values =                                                                           \
      "NONE" ENUM_LITE_PP_FOR_EACH_PAIR(ENUM_LITE_DETAIL_VALUES, __VA_ARGS__) ", ALL";                                 \
                                                                                                                       \
  constexpr Type operator|(Type lhs, Type rhs) noexcept                                                                \
  {                                                                                                                    \
    using U = std::underlying_type_t<Type>;                                                                            \
    return static_cast<Type>(static_cast<U>(lhs) | static_cast<U>(rhs));                                               \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type operator&(Type lhs, Type rhs) noexcept                                                                \
  {                                                                                                                    \
    using U = std::underlying_type_t<Type>;                                                                            \
    return static_cast<Type>(static_cast<U>(lhs) & static_cast<U>(rhs));                                               \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type operator^(Type lhs, Type rhs) noexcept                                                                \
  {                                                                                                                    \
    using U = std::underlying_type_t<Type>;                                                                            \
    return static_cast<Type>(static_cast<U>(lhs) ^ static_cast<U>(rhs));                                               \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type operator~(Type value) noexcept                                                                        \
  {                                                                                                                    \
    using U = std::underlying_type_t<Type>;                                                                            \
    return static_cast<Type>(~static_cast<U>(value));                                                                  \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type& operator|=(Type& lhs, Type rhs) noexcept                                                             \
  {                                                                                                                    \
    lhs = lhs | rhs;                                                                                                   \
    return lhs;                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type& operator&=(Type& lhs, Type rhs) noexcept                                                             \
  {                                                                                                                    \
    lhs = lhs & rhs;                                                                                                   \
    return lhs;                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type& operator^=(Type& lhs, Type rhs) noexcept                                                             \
  {                                                                                                                    \
    lhs = lhs ^ rhs;                                                                                                   \
    return lhs;                                                                                                        \
  }                                                                                                                    \
                                                                                                                       \
  constexpr bool Type##_has_flag(Type value, Type flag) noexcept                                                       \
  {                                                                                                                    \
    using U = std::underlying_type_t<Type>;                                                                            \
                                                                                                                       \
    const U f = static_cast<U>(flag);                                                                                  \
                                                                                                                       \
    if (f == 0) return static_cast<U>(value) == 0;                                                                     \
                                                                                                                       \
    return (static_cast<U>(value) & f) == f;                                                                           \
  }                                                                                                                    \
                                                                                                                       \
  constexpr std::string Type##_to_str(Type value, char separator = '|')                                                \
  {                                                                                                                    \
    if (value == Type::NONE) return "NONE";                                                                            \
                                                                                                                       \
    std::string result;                                                                                                \
                                                                                                                       \
    ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_FLAG_TO_STRING, Type, __VA_ARGS__)                                 \
                                                                                                                       \
    return result;                                                                                                     \
  }                                                                                                                    \
                                                                                                                       \
  constexpr auto Type##_to_vec_str(Type value) noexcept                                                                \
  {                                                                                                                    \
    enum_lite::static_string_list<Type##_names.size()> result;                                                         \
                                                                                                                       \
    if (value == Type::NONE) {                                                                                         \
      result.storage[result.size++] = "NONE";                                                                          \
      return result;                                                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_FLAG_TO_VECTOR, Type, __VA_ARGS__)                                 \
                                                                                                                       \
    return result;                                                                                                     \
  }                                                                                                                    \
                                                                                                                       \
  constexpr Type Type##_from_str(std::string_view str, char separator = '|') noexcept                                  \
  {                                                                                                                    \
    Type result = Type::NONE;                                                                                          \
                                                                                                                       \
    while (!str.empty()) {                                                                                             \
      const std::size_t      pos   = str.find(separator);                                                              \
      const std::string_view token = enum_lite::trim(str.substr(0, pos));                                              \
                                                                                                                       \
      if (token == "NONE") {                                                                                           \
        /* NONE contributes no bits. */                                                                                \
      } else {                                                                                                         \
        ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_FLAG_FROM_STRING, Type, __VA_ARGS__)                           \
      }                                                                                                                \
                                                                                                                       \
      if (pos == std::string_view::npos) break;                                                                        \
                                                                                                                       \
      str.remove_prefix(pos + 1);                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    return result;                                                                                                     \
  }                                                                                                                    \
                                                                                                                       \
  constexpr bool Type##_is_valid(std::string_view str, std::string_view separator = "|") noexcept                      \
  {                                                                                                                    \
    if (str.empty()) return false;                                                                                     \
                                                                                                                       \
    while (!str.empty()) {                                                                                             \
      const std::size_t      pos   = str.find(separator);                                                              \
      const std::string_view token = enum_lite::trim(str.substr(0, pos));                                              \
                                                                                                                       \
      if (token.empty()) return false;                                                                                 \
                                                                                                                       \
      bool known = false;                                                                                              \
                                                                                                                       \
      if (enum_lite::normalized_equal(token, "NONE")) {                                                                \
        known = true;                                                                                                  \
      } else {                                                                                                         \
        ENUM_LITE_PP_FOR_EACH_PAIR_CTX(ENUM_LITE_DETAIL_FLAG_IS_VALID_STRING, Type, __VA_ARGS__)                       \
      }                                                                                                                \
                                                                                                                       \
      if (!known) return false;                                                                                        \
                                                                                                                       \
      if (pos == std::string_view::npos) break;                                                                        \
                                                                                                                       \
      str.remove_prefix(pos + 1);                                                                                      \
    }                                                                                                                  \
                                                                                                                       \
    return true;                                                                                                       \
  }


#define DEFINE_ENUM(Type, Underlying, ...) ENUM_LITE_DEFINE_ENUM(Type, Underlying, __VA_ARGS__)

#define DEFINE_FLAGS(Type, Underlying, ...) ENUM_LITE_DEFINE_FLAGS(Type, Underlying, __VA_ARGS__)


} // namespace enum_lite