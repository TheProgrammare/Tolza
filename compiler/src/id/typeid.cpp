#include "id/typeid.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "id/defid.hpp"
#include "pool/node_to_inf.hpp"
#include "type/pool.hpp"
#include "type/type.hpp"

#include <cassert>
#include <string>


type::ID type::ID::canonical() const noexcept
{
  assert(*this && "Must be valid id");
  return get().tyid; // get is always canonical
}
type::ETypeKind type::ID::kind() const noexcept
{
  assert(*this && "Must be valid id");
  return get().kind;
}
definition::ID type::ID::def() const noexcept
{
  assert(*this && "Must be valid id");
  return COMPILER.inference.get_declaration(*this).def();
}

template <type::Generic T>
T* type::ID::as() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->as<T>(*this);
}
template <type::Generic T>
const T* type::ID::as() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->as<T>(*this);
}

template <type::Generic T>
bool type::ID::is() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->get(*this).kind == T::static_kind;
}


#define TYPE_GET_INSTANCE(T)                                                                                           \
  template T*       type::ID::as<T>() noexcept;                                                                        \
  template const T* type::ID::as<T>() const noexcept;                                                                  \
  template bool     type::ID::is<T>() const noexcept;

TYPE_GET_INSTANCE(type::Primitive)
TYPE_GET_INSTANCE(type::String)
TYPE_GET_INSTANCE(type::Tuple)
TYPE_GET_INSTANCE(type::Array)
TYPE_GET_INSTANCE(type::Buffer)
TYPE_GET_INSTANCE(type::Slice)
TYPE_GET_INSTANCE(type::Ptr)
TYPE_GET_INSTANCE(type::Prototype)
TYPE_GET_INSTANCE(type::Facet)
TYPE_GET_INSTANCE(type::View)
TYPE_GET_INSTANCE(type::Form)
TYPE_GET_INSTANCE(type::Enum)
TYPE_GET_INSTANCE(type::Flag)
TYPE_GET_INSTANCE(type::Union)
TYPE_GET_INSTANCE(type::Identifier)
TYPE_GET_INSTANCE(type::Range)

#undef TYPE_GET_INSTANCE
