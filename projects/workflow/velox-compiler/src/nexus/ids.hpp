#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

#include <functional>
#include <limits>


#define INVALID_ID uint32_t(-1)
#define WILCARD_ID uint32_t(-2)
#define PARENT_ID  uint32_t(-3)

#include <type_traits>
#include <cstdint>

template <typename T>
struct is_allowed_id_type : std::false_type {
};

template <>
struct is_allowed_id_type<uint32_t> : std::true_type {
};

template <>
struct is_allowed_id_type<uint64_t> : std::true_type {
};

// clang-format off

// no id returned -> empty id class
#define NO_ID {}

// clang-format on


#define __DEF_ID(name, type, ...)                                                                                      \
  static_assert(is_allowed_id_type<type>::value, "Invalid ID type: only uint32_t and uint64_t are allowed");           \
                                                                                                                       \
  class name                                                                                                           \
  {                                                                                                                    \
  public:                                                                                                              \
    name() = default;                                                                                                  \
                                                                                                                       \
    name(const name&)            = default;                                                                            \
    name& operator=(const name&) = default;                                                                            \
                                                                                                                       \
    name(name&&) noexcept            = default;                                                                        \
    name& operator=(name&&) noexcept = default;                                                                        \
                                                                                                                       \
    explicit constexpr name(type value) noexcept                                                                       \
      : id(value)                                                                                                      \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    constexpr bool operator==(const name&) const noexcept  = default;                                                  \
    constexpr auto operator<=>(const name&) const noexcept = default;                                                  \
                                                                                                                       \
    constexpr auto operator<=>(type r) const noexcept                                                                  \
    {                                                                                                                  \
      return id <=> r;                                                                                                 \
    }                                                                                                                  \
    constexpr name operator+(type r) const noexcept                                                                    \
    {                                                                                                                  \
      return name(id + r);                                                                                             \
    }                                                                                                                  \
    name& operator+=(type r) noexcept                                                                                  \
    {                                                                                                                  \
      id += r;                                                                                                         \
      return *this;                                                                                                    \
    }                                                                                                                  \
    constexpr name operator-(type r) const noexcept                                                                    \
    {                                                                                                                  \
      return name(id - r);                                                                                             \
    }                                                                                                                  \
    name& operator-=(type r) noexcept                                                                                  \
    {                                                                                                                  \
      id -= r;                                                                                                         \
      return *this;                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    constexpr type value() const noexcept                                                                              \
    {                                                                                                                  \
      return id;                                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    constexpr bool is_valid() const noexcept                                                                           \
    {                                                                                                                  \
      return id != std::numeric_limits<type>::max();                                                                   \
    }                                                                                                                  \
                                                                                                                       \
    explicit constexpr operator bool() const noexcept                                                                  \
    {                                                                                                                  \
      return is_valid();                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
  private:                                                                                                             \
    type id = std::numeric_limits<type>::max();                                                                        \
                                                                                                                       \
    __VA_ARGS__                                                                                                        \
  };                                                                                                                   \
                                                                                                                       \
  struct name##_hash {                                                                                                 \
    uint64_t operator()(const name& x) const noexcept                                                                  \
    {                                                                                                                  \
      return std::hash<uint64_t>{}(x.value());                                                                         \
    }                                                                                                                  \
  };


namespace script
{

__DEF_ID(_id, uint32_t)

}

namespace ast
{
class _gnid;

__DEF_ID(_id, uint32_t, public : _gnid get_gnid(script::_id scr) const noexcept;)
// Global node id ((script_id << 32) | node_id)
__DEF_ID(_gnid, uint64_t, public : script::_id get_script_id() const noexcept; _id get_node_id() const noexcept;)

} // namespace ast

namespace token
{

__DEF_ID(_file_pos, uint32_t)
__DEF_ID(_id, uint32_t)

} // namespace token

namespace metacode
{

__DEF_ID(_id, uint64_t)

}

namespace type
{

__DEF_ID(_id, uint64_t)

}

namespace module
{

__DEF_ID(_id, uint64_t)

}

namespace scope
{

__DEF_ID(_id, uint64_t)

}

namespace symbol
{

__DEF_ID(_id, uint64_t)

}


#undef __DEF_ID