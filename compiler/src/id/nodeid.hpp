#pragma once

#include "id/base.hpp"
#include "id/id_query.hpp"

#include <cstdint>


namespace semantic
{
struct Metadata;
}

namespace ast
{

enum class ENodeKind : uint8_t;
struct NodeHeader;

template <typename T>
concept Generic = requires(T obj) {
  obj.header;
  obj.static_kind;
  obj.nodeid();
};

// node identifier
class ID final : public ::ID<ID, NodeHeader>
{
  ID_HEADER(NodeHeader)

public:
  // get real node
  [[nodiscard]] ast::ID        canonical() const noexcept;
  // get node token reference
  [[nodiscard]] ast::ENodeKind kind() const noexcept;
  // get node token reference
  [[nodiscard]] token::ID      token() const noexcept;
  // node corresponding type if type declaration or infered type if expression
  // return NO_ID(-1) if no type applicable
  [[nodiscard]] type::ID       type() const noexcept;
  // check if node inferred
  [[nodiscard]] bool           is_inferred() const noexcept;
  // definitionic representation of the node only if it's a declaration
  [[nodiscard]] definition::ID def() const noexcept;
  // check if node resolved
  [[nodiscard]] bool           is_resolved() const noexcept;
  // scope node owner
  [[nodiscard]] scope::ID      scope() const noexcept;
  // module node owner
  [[nodiscard]] module::ID     module() const noexcept;

  // get node expression or literal value affected
  [[nodiscard]] ast::ID value() const noexcept;

  // from expression
  [[nodiscard]] bool is_rvalue() const noexcept;
  // from expression
  [[nodiscard]] bool is_lvalue() const noexcept;

  template <Generic T>
  [[nodiscard]] T* as() noexcept;
  template <Generic T>
  [[nodiscard]] const T* as() const noexcept;
  // is node
  template <Generic T>
  [[nodiscard]] bool is() const noexcept;

  [[nodiscard]] bool is_builtin() const noexcept;

  [[nodiscard]] semantic::Metadata*       sem() noexcept;
  [[nodiscard]] const semantic::Metadata* sem() const noexcept;
};


} // namespace ast
