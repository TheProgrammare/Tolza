#pragma once

#include "nexus/type/data.hpp"

namespace type
{

#define TYPE_HEADER(name)                                                                                              \
  static constexpr ETypeKind static_kind = ETypeKind::name;                                                            \
  TypeHeader                 header      = TypeHeader(ETypeKind::name);                                                \
  [[nodiscard]] ID           tyid() const noexcept                                                                     \
  {                                                                                                                    \
    return header.tyid;                                                                                                \
  }

struct TypeHeader final {
  ID        tyid;
  Qualifier qualifier;
  ETypeKind kind = ETypeKind::NONE;

  explicit TypeHeader(ETypeKind k)
    : kind(k)
  {
  }
};


struct Primitive final {
  TYPE_HEADER(Primitive);

  EPrimitiveTypeKind primitive;
};

struct Ptr final {
  TYPE_HEADER(Ptr);

  ID inner;
};

struct String final {
  TYPE_HEADER(String);

  ETextType kind = ETextType::_str;
};

struct Array final {
  TYPE_HEADER(Array);

  ID      inner;
  size_t  size;
  ast::ID size_expression;
};

struct Buffer final {
  TYPE_HEADER(Buffer);

  ID inner;
};

struct Slice final {
  TYPE_HEADER(Slice);

  ID   inner;
  bool is_c_table = false;
};

struct Tuple final {
  TYPE_HEADER(Tuple);

  std::vector<ID> elems;
};

struct Prototype final {
  TYPE_HEADER(Prototype);

  std::vector<Prototype_Param> params;
  ID                           ret;
  bool                         is_variadic     = false;
  bool                         is_explicit_ret = false;
};

struct Enum final {
  TYPE_HEADER(Enum);

  std::vector<ID> variants;
  definition::ID  def;
};

struct Flag final {
  TYPE_HEADER(Flag);

  size_t         size = 1;
  definition::ID def;
};

struct Union final {
  TYPE_HEADER(Union);

  std::vector<ID> variants;
  definition::ID  def;
};

struct Facet final {
  TYPE_HEADER(Facet);

  std::vector<ID> fields;
  definition::ID  def;
};

struct View final {
  TYPE_HEADER(View);

  std::vector<ID> facets;
  definition::ID  def;
};

struct Form final {
  TYPE_HEADER(Form);

  std::vector<ID> facets;
  definition::ID  def;
};

struct Identifier final {
  TYPE_HEADER(Identifier);

  ast::ID        nodeid;
  definition::ID def;
  std::string    forward_name;

  [[nodiscard]] bool is_resolved() const noexcept
  {
    return bool(def);
  }
  [[nodiscard]] ID get_type() const noexcept
  {
    return def.node().type();
  }
};


using Variant = std::variant<Primitive, String, Tuple, Array, Buffer, Slice, Ptr, Prototype, Facet, View, Form, Enum,
                             Flag, Union, Identifier>;

template <typename T>
concept AllTypes = requires {
  typename std::variant_size<Variant>;
  std::variant<T>();
};


#undef TYPE_HEADER


} // namespace type