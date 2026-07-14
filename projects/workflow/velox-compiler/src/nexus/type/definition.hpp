#pragma once

#include "nexus/type/data.hpp"

namespace type
{


struct Type {
  ID tyid;

  Qualifier qualifier;

  [[nodiscard]] definition::ID get_def_id() const noexcept;
  [[nodiscard]] bool           set_def_id(definition::ID defid) noexcept;

  [[nodiscard]] ETypeKind kind() const noexcept
  {
    return _kind;
  }

  Type(const Type&)            = delete;
  Type& operator=(const Type&) = delete;

protected:
  const ETypeKind _kind = ETypeKind::NONE;
  Type(ETypeKind k)
    : _kind(k)
  {
  }

public:
  virtual ~Type() = default;
};

template <ETypeKind K>
struct TypeBase : Type {
  static constexpr ETypeKind static_kind = K;
  TypeBase()
    : Type(K)
  {
  }
};

#define DEF_TYPE(name) struct name final : TypeBase<ETypeKind::name>


DEF_TYPE(Primitive)
{
  EPrimitiveTypeKind primitive;
};

DEF_TYPE(Ptr)
{
  ID inner;
};

DEF_TYPE(String)
{
  ETextType kind = ETextType::_str;
};

DEF_TYPE(Array)
{
  ID      inner;
  size_t  size;
  ast::ID size_expression;
};

DEF_TYPE(Buffer)
{
  ID inner;
};

DEF_TYPE(Slice)
{
  ID   inner;
  bool is_c_table = false;
};

DEF_TYPE(Tuple)
{
  std::vector<ID> elems;
};

DEF_TYPE(Prototype)
{
  std::vector<Prototype_Param> params;
  ID                           ret;
  bool                         is_variadic     = false;
  bool                         is_explicit_ret = false;
};

DEF_TYPE(Enum)
{
  std::vector<ID> variants;
  definition::ID  def;
};

DEF_TYPE(Flag)
{
  size_t         size = 1;
  definition::ID def;
};

DEF_TYPE(Union)
{
  std::vector<ID> variants;
  definition::ID  def;
};

DEF_TYPE(Facet)
{
  std::vector<ID> fields;
  definition::ID  def;
};

DEF_TYPE(View)
{
  std::vector<ID> facets;
  definition::ID  def;
};

DEF_TYPE(Form)
{
  std::vector<ID> facets;
  definition::ID  def;
};

DEF_TYPE(Identifier)
{
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


template <typename T>
concept IsDataType = std::is_base_of_v<Type, T>;

} // namespace type