#pragma once

#include "id/base.hpp"
#include "id/cuid.hpp"
#include "id/id_query.hpp"

#include <cstddef>
#include <cstdint>

namespace type
{

struct TypeHeader;
enum class EPrimitiveTypeKind : uint8_t;
enum class ETypeKind : uint8_t;

template <typename T>
concept Generic = requires(T obj) {
  obj.header;
  obj.static_kind;
  obj.tyid();
};

// type identifier
class ID final : public ::ID<ID, TypeHeader>
{
  ID_HEADER(TypeHeader)
public:
  // get the canonical type
  [[nodiscard]] type::ID        canonical() const noexcept;
  // get node token reference
  [[nodiscard]] type::ETypeKind kind() const noexcept;
  // get definition reference
  [[nodiscard]] definition::ID  def() const noexcept;

  // get type
  template <Generic T>
  [[nodiscard]] T* as() noexcept;
  // get type
  template <Generic T>
  [[nodiscard]] const T* as() const noexcept;
  // is type
  template <Generic T>
  [[nodiscard]] bool is() const noexcept;

  [[nodiscard]] static constexpr ID make_primitive(EPrimitiveTypeKind prim) noexcept
  {
    auto raw = static_cast<size_t>(prim);
    if (raw == 0) return ID::invalid(); // invalid case

    return ID::make(cu::ID::main(), raw);
  }
};

} // namespace type