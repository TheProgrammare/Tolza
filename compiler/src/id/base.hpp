#pragma once

#include "id/cuid.hpp"
#include "id/id_query.hpp"

#include <compare>
#include <cstdint>

constexpr uint32_t WILCARD_ID = -2;
constexpr uint32_t PARENT_ID  = -3;
constexpr uint32_t MASK_32    = 0xFFFFFFFFUL;


uint64_t base_hash(uint64_t x) noexcept;

// no id returned -> empty id class
#define NO_ID                                                                                                          \
  {                                                                                                                    \
  }

template <typename DERIVED, typename TYPE>
class ID
{
public:
  ID& operator=(ID&&) noexcept      = default;
  ID& operator=(const ID&) noexcept = default;

private:
  ID()                   = default;
  ID(const ID&) noexcept = default;
  ID(ID&&) noexcept      = default;

  explicit constexpr ID(uint64_t value) noexcept
    : id(value)
  {
  }

public:
  [[nodiscard]] TYPE& get() noexcept
  {
    return id_query<TYPE>::get(static_cast<DERIVED&>(*this));
  }

  [[nodiscard]] const TYPE& get() const noexcept
  {
    return id_query<TYPE>::get(static_cast<const DERIVED&>(*this));
  }

  [[nodiscard]] static constexpr DERIVED make(uint64_t value) noexcept
  {
    return DERIVED(value);
  }

  [[nodiscard]] static constexpr DERIVED invalid() noexcept
  {
    return DERIVED::make(INVALID_ID);
  }
  [[nodiscard]] constexpr bool is_valid() const noexcept
  {
    return id != INVALID_ID;
  }

  explicit constexpr operator bool() const noexcept
  {
    return is_valid();
  }

  constexpr bool operator==(const ID&) const noexcept  = default;
  constexpr auto operator<=>(const ID&) const noexcept = default;

  constexpr auto operator<=>(uint64_t r) const noexcept
  {
    return id <=> r;
  }
  constexpr DERIVED operator+(uint64_t r) const noexcept
  {
    return DERIVED(id + r);
  }
  DERIVED& operator+=(uint64_t r) noexcept
  {
    id += r;
    return *this;
  }
  constexpr DERIVED operator-(uint64_t r) const noexcept
  {
    return DERIVED(id - r);
  }
  DERIVED& operator-=(uint64_t r) noexcept
  {
    id -= r;
    return *this;
  }

  [[nodiscard]] constexpr uint64_t raw() const noexcept
  {
    return id;
  }

  struct Hash {
    uint64_t operator()(const DERIVED& x) const noexcept
    {
      return base_hash(x.raw());
    }
  };
  struct Compare {
    bool operator()(const DERIVED& a, const DERIVED& b) const
    {
      return a.raw() < b.raw();
    }
  };

  [[nodiscard]] cu::ID cu() const noexcept
  {
    return cu::ID::make(static_cast<uint64_t>(id >> 32U));
  }

  [[nodiscard]] uint32_t index() const noexcept
  {
    return id & MASK_32;
  }

  [[nodiscard]] static constexpr DERIVED make(cu::ID cuid, uint32_t index) noexcept
  {
    return DERIVED((cuid.raw() << 32U) | index);
  }

  void set_cu(cu::ID cuid) noexcept
  {
    id = (id & 0x00000000FFFFFFFFULL) | (cuid.raw() << 32);
  }

  void set_index(uint32_t index) noexcept
  {
    id = (id & 0xFFFFFFFF00000000ULL) | uint64_t(index);
  }

private:
  uint64_t id = INVALID_ID;
  friend DERIVED;
};

#define ID_HEADER(TYPE)                                                                                                \
public:                                                                                                                \
  ID() = default;                                                                                                      \
                                                                                                                       \
  ID(const ID&) noexcept            = default;                                                                         \
  ID& operator=(const ID&) noexcept = default;                                                                         \
                                                                                                                       \
  ID(ID&&) noexcept            = default;                                                                              \
  ID& operator=(ID&&) noexcept = default;                                                                              \
                                                                                                                       \
  explicit constexpr ID(uint64_t value) noexcept                                                                       \
    : ::ID<ID, TYPE>(value)                                                                                            \
  {                                                                                                                    \
  }
