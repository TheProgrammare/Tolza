#pragma once


#include <compare>
#include <cstdint>

constexpr uint64_t FLAG_TEMP_CU = 1U << 31;
constexpr uint64_t INVALID_ID   = UINT64_MAX;


namespace cu
{
struct CU;
struct TEMP_CU;

class ID
{
public:
  ID() = default;

  ID(const ID&) noexcept            = default;
  ID& operator=(const ID&) noexcept = default;

  ID(ID&&) noexcept            = default;
  ID& operator=(ID&&) noexcept = default;

private:
  explicit constexpr ID(uint64_t value) noexcept
    : id(value)
  {
  }

public:
  [[nodiscard]] static constexpr ID make(uint64_t value) noexcept
  {
    return ID(value);
  }

  [[nodiscard]] static constexpr ID invalid() noexcept
  {
    return ID(INVALID_ID);
  }
  [[nodiscard]] constexpr bool is_valid() const noexcept
  {
    return id != INVALID_ID;
  }

  explicit constexpr operator bool() const noexcept
  {
    return is_valid();
  }

  constexpr bool operator==(const ID&) const noexcept    = default;
  constexpr auto operator<=>(const ID& r) const noexcept = default;

  constexpr auto operator<=>(uint64_t r) const noexcept
  {
    return id <=> r;
  }
  constexpr ID operator+(uint64_t r) const noexcept
  {
    return ID(id + r);
  }
  ID& operator+=(uint64_t r) noexcept
  {
    id += r;
    return *this;
  }
  constexpr ID operator-(uint64_t r) const noexcept
  {
    return ID(id - r);
  }
  ID& operator-=(uint64_t r) noexcept
  {
    id -= r;
    return *this;
  }

  [[nodiscard]] constexpr uint64_t raw() const noexcept
  {
    return id;
  }

  struct Hash {
    uint64_t operator()(const ID& x) const noexcept;
  };
  struct Compare {
    bool operator()(const ID& a, const ID& b) const
    {
      return a.raw() < b.raw();
    }
  };

  // get compilation unit
  [[nodiscard]] cu::CU&       get() noexcept;
  // get compilation unit
  [[nodiscard]] const cu::CU& get() const noexcept;
  // check if is a temporary compilation unit
  [[nodiscard]] bool          is_temp() const noexcept
  {
    return id & FLAG_TEMP_CU;
  }
  // get the main compilation unit : index 0
  [[nodiscard]] static constexpr ID main() noexcept
  {
    return cu::ID::make(0);
  }
  // get the cu offset
  [[nodiscard]] uint32_t offset() const noexcept
  {
    return id & ~FLAG_TEMP_CU;
  };
  // set the offset
  void set_offset(uint32_t offset) noexcept
  {
    id = (id & FLAG_TEMP_CU) | (offset & ~FLAG_TEMP_CU);
  }
  // set temp
  void set_temp() noexcept
  {
    id |= FLAG_TEMP_CU;
  };
  // unset temp
  void unset_temp() noexcept
  {
    id &= ~FLAG_TEMP_CU;
  };

private:
  uint64_t id = INVALID_ID;
};

} // namespace cu