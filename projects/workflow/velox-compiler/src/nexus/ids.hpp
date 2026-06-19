#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

#include <string>
#include <format>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>


constexpr uint32_t INVALID_ID = -1;
constexpr uint32_t WILCARD_ID = -2;
constexpr uint32_t PARENT_ID  = -3;
constexpr uint32_t MASK_32    = 0xFFFFFFFFUL;

namespace cu
{
class ID;
}

namespace ast
{
class ID;
} // namespace ast


namespace token
{
class _file_pos;
class ID;
} // namespace token

namespace metacode
{
class ID;
}

namespace type
{
class ID;
}

namespace module
{
class ID;
}

namespace scope
{
class ID;
}

namespace symbol
{
class ID;
}

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


#define BASE_ID(name, ...)                                                                                             \
  class name final                                                                                                     \
  {                                                                                                                    \
  public:                                                                                                              \
    name() = default;                                                                                                  \
                                                                                                                       \
    name(const name&) noexcept            = default;                                                                   \
    name& operator=(const name&) noexcept = default;                                                                   \
                                                                                                                       \
    name(name&&) noexcept            = default;                                                                        \
    name& operator=(name&&) noexcept = default;                                                                        \
                                                                                                                       \
  private:                                                                                                             \
    explicit constexpr name(uint64_t value) noexcept                                                                   \
      : id(value)                                                                                                      \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
  public:                                                                                                              \
    [[nodiscard]] static constexpr name invalid() noexcept                                                             \
    {                                                                                                                  \
      return name(std::numeric_limits<uint64_t>::max());                                                               \
    }                                                                                                                  \
    [[nodiscard]] constexpr bool is_valid() const noexcept                                                             \
    {                                                                                                                  \
      return id != std::numeric_limits<uint64_t>::max();                                                               \
    }                                                                                                                  \
                                                                                                                       \
    [[nodiscard]] std::string dec() const noexcept                                                                     \
    {                                                                                                                  \
      return std::to_string(id);                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    [[nodiscard]] std::string hex() const noexcept                                                                     \
    {                                                                                                                  \
      return std::format("0x{:x}", id);                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    explicit constexpr operator bool() const noexcept                                                                  \
    {                                                                                                                  \
      return is_valid();                                                                                               \
    }                                                                                                                  \
                                                                                                                       \
    constexpr bool operator==(const name&) const noexcept  = default;                                                  \
    constexpr auto operator<=>(const name&) const noexcept = default;                                                  \
                                                                                                                       \
    constexpr auto operator<=>(uint64_t r) const noexcept                                                              \
    {                                                                                                                  \
      return id <=> r;                                                                                                 \
    }                                                                                                                  \
    constexpr name operator+(uint64_t r) const noexcept                                                                \
    {                                                                                                                  \
      return name(id + r);                                                                                             \
    }                                                                                                                  \
    name& operator+=(uint64_t r) noexcept                                                                              \
    {                                                                                                                  \
      id += r;                                                                                                         \
      return *this;                                                                                                    \
    }                                                                                                                  \
    constexpr name operator-(uint64_t r) const noexcept                                                                \
    {                                                                                                                  \
      return name(id - r);                                                                                             \
    }                                                                                                                  \
    name& operator-=(uint64_t r) noexcept                                                                              \
    {                                                                                                                  \
      id -= r;                                                                                                         \
      return *this;                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    [[nodiscard]] constexpr uint64_t raw() const noexcept                                                              \
    {                                                                                                                  \
      return id;                                                                                                       \
    }                                                                                                                  \
                                                                                                                       \
    struct Hash {                                                                                                      \
      uint64_t operator()(const name& x) const noexcept                                                                \
      {                                                                                                                \
        return std::hash<uint64_t>{}(x.raw());                                                                         \
      }                                                                                                                \
    };                                                                                                                 \
                                                                                                                       \
  private:                                                                                                             \
    uint64_t id = std::numeric_limits<uint64_t>::max();                                                                \
                                                                                                                       \
    __VA_ARGS__                                                                                                        \
  };


#define DEF_ID(name, ...)                                                                                              \
  BASE_ID(                                                                                                             \
      name, public :                                                                                                   \
                                                                                                                       \
      [[nodiscard]] static constexpr name make(uint64_t value) noexcept { return name(value); }                        \
                                                                                                                       \
      __VA_ARGS__                                                                                                      \
                                                                                                                       \
  )


#define DEF_COMPOSIT_ID(name, ...)                                                                                     \
  BASE_ID(                                                                                                             \
      name, public :                                                                                                   \
                                                                                                                       \
      [[nodiscard]] cu::ID cu() const noexcept { return cu::ID::make(static_cast<uint64_t>(id >> 32U)); }              \
                                                                                                                       \
      [[nodiscard]] uint32_t offset() const noexcept { return id & MASK_32; }                                          \
                                                                                                                       \
      [[nodiscard]] static constexpr name make(cu::ID cuid, uint32_t offset) noexcept {                                \
        return name((size_t(cuid.raw()) << 32U) | offset);                                                             \
      }                                                                                                                \
                                                                                                                       \
      void set_cu(cu::ID cuid) noexcept { id = (id & 0x00000000FFFFFFFFULL) | (uint64_t(cuid.raw()) << 32); }          \
                                                                                                                       \
      void set_offset(uint32_t offset) noexcept { id = (id & 0xFFFFFFFF00000000ULL) | uint64_t(offset); }              \
                                                                                                                       \
      __VA_ARGS__                                                                                                      \
                                                                                                                       \
  )


namespace cu
{

struct CU;

// compilation unit identifier
DEF_ID(
    ID,

    public :
    // get compilation unit
    [[nodiscard]] cu::CU&             get() noexcept;
    // get compilation unit
    [[nodiscard]] const cu::CU&       get() const noexcept;
    // get the main compilation unit : index 0
    [[nodiscard]] static constexpr ID main() noexcept { return cu::ID::make(0); }

)

} // namespace cu

namespace ast
{

struct Node;

// node identifier
DEF_COMPOSIT_ID(ID,

                public :
                // get node token reference
                [[nodiscard]] token::ID  token() const noexcept;
                // node corresponding type if type declaration or infered type if expression
                // return NO_ID(-1) if no type applicable
                [[nodiscard]] type::ID   type() const noexcept;
                // check if node inferred
                [[nodiscard]] bool       is_inferred() const noexcept;
                // symbolic representation of the node only if it's a declaration
                [[nodiscard]] symbol::ID symbol() const noexcept;
                // scope node owner
                [[nodiscard]] scope::ID  scope() const noexcept;
                // module node owner
                [[nodiscard]] module::ID module() const noexcept;

                // get node reference
                [[nodiscard]] ast::Node * get() noexcept;
                // get node reference
                [[nodiscard]] const ast::Node* get() const noexcept;

                template <typename T> [[nodiscard]] T * as() noexcept;
                template <typename T> [[nodiscard]] const T* as() const noexcept;

)


} // namespace ast


namespace token
{

struct Token;

// token identifier
DEF_COMPOSIT_ID(ID,

                public :
                // get string representation on file
                [[nodiscard]] std::string_view str() const noexcept;
                // get file position
                [[nodiscard]] size_t           pos() const noexcept;
                // get line position
                [[nodiscard]] size_t           line() const noexcept;
                // get all line string respresentation on file
                [[nodiscard]] std::string_view line_str() const noexcept;
                // get token reference
                [[nodiscard]] token::Token & get() noexcept;
                // get token reference
                [[nodiscard]] const token::Token& get() const noexcept;

)

DEF_ID(_file_pos,

       public :
       // get all line string respresentation on file
       [[nodiscard]] std::string_view line_str() const noexcept;

)

} // namespace token

namespace metacode
{

// metacode identifier
DEF_ID(ID)

} // namespace metacode

namespace type
{

struct Type;
enum class EPrimitiveTypeKind : uint8_t;

// type identifier
DEF_COMPOSIT_ID(
    ID,

    public :
    // get symbol reference
    [[nodiscard]] symbol::ID symbol() const noexcept;
    // get node declaration reference
    [[nodiscard]] ast::ID    declaration() const noexcept;
    // get type
    [[nodiscard]] Type & get() noexcept;
    // get type
    [[nodiscard]] const Type& get() const noexcept;
    // get type
    template <typename T> [[nodiscard]] T * as() noexcept;
    // get type
    template <typename T> [[nodiscard]] const T* as() const noexcept;

    [[nodiscard]] static constexpr ID make_primitive(cu::ID cuid, EPrimitiveTypeKind prim) noexcept {
      auto raw = static_cast<size_t>(prim);
      if (raw == 0) return ID::invalid(); // invalid case

      return ID::make(cuid, raw);
    }

)

} // namespace type

namespace module
{
struct Module;

// module identifier
DEF_COMPOSIT_ID(ID,

                public :
                // get parent
                [[nodiscard]] ID parent() const noexcept;
                // get all children
                [[nodiscard]] std::vector<ID> & children() const noexcept;
                // get module node owner
                [[nodiscard]] ast::ID   node() const noexcept;
                // get module base scope
                [[nodiscard]] scope::ID scope() const noexcept;
                // get module
                [[nodiscard]] Module & get() noexcept;
                // get module
                [[nodiscard]] const Module& get() const noexcept;

)

} // namespace module

namespace scope
{
struct Scope;

// scope identifier
DEF_COMPOSIT_ID(ID,

                public :
                // get parent
                [[nodiscard]] ID parent() const noexcept;
                // get all children
                [[nodiscard]] std::vector<ID> & children() const noexcept;
                // get node scope owner
                [[nodiscard]] ast::ID    node() const noexcept;
                // get scope parent module
                [[nodiscard]] module::ID module() const noexcept;
                // get scope
                [[nodiscard]] Scope & get() noexcept;
                // get scope
                [[nodiscard]] const Scope& get() const noexcept;

)

} // namespace scope

namespace symbol
{
struct Symbol;

// symbol identifier
DEF_COMPOSIT_ID(ID,

                public :
                // get symbol type
                [[nodiscard]] type::ID   type() const noexcept;
                // get symbol node reference
                [[nodiscard]] ast::ID    node() const noexcept;
                // scope symbol owner
                [[nodiscard]] scope::ID  scope() const noexcept;
                // module symbol owner
                [[nodiscard]] module::ID module() const noexcept;
                // get symbol
                [[nodiscard]] Symbol & get() noexcept;
                // get symbol
                [[nodiscard]] const Symbol& get() const noexcept;

)

} // namespace symbol


#undef DEF_ID
#undef VALIDATION