#include "type/pool.hpp"

#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/tool.hpp"
#include "compiler/compiler.hpp"
#include "id/base.hpp"
#include "id/cuid.hpp"
#include "lexer/data.hpp"
#include "nexus/forward.hpp"
#include "type/data.hpp"
#include "type/type.hpp"

#include <cassert>
#include <common/compiler_options.hpp>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <functional>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>


type::Arena::Arena(cu::ID _cuid)
  : cuid(_cuid)
{
}


type::ID type::Factory::make_primitive(EPrimitiveTypeKind p, Qualifier qualifier)
{
  assert(p != EPrimitiveTypeKind::NONE && "illegal primitive (EPrimitiveTypeKind::NONE)");

  if (qualifier.is_pure()) {
    const auto main_ty = ID::make_primitive(p);
    return main_ty;
  }

  type::Primitive t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Primitive;
  t.primitive        = p;
  return arena.intern(t);
}
type::ID type::Factory::make_ptr(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  Ptr t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Ptr;
  t.inner            = inner;
  return arena.intern(t);
}
type::ID type::Factory::make_static_array(type::ID inner, size_t size, ast::ID size_expr, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");
  assert(size != 0 || size_expr && "Impossible static size");

  Array t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Array;
  t.inner            = inner;
  t.size             = size;
  t.size_expression  = size_expr;
  return arena.intern(t);
}
type::ID type::Factory::make_dynamic_array(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  Buffer t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Buffer;
  t.inner            = inner;
  return arena.intern(t);
}
type::ID type::Factory::make_slice(type::ID inner, Qualifier qualifier, bool is_c_table)
{
  assert(inner && "Invalid identifier");

  Slice t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Slice;
  t.inner            = inner;
  t.is_c_table       = is_c_table;
  return arena.intern(t);
}
type::ID type::Factory::make_tuple(const std::vector<type::ID>& elems, Qualifier qualifier)
{
  assert(!elems.empty() && "Illegal empty tuple");

  Tuple t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Tuple;
  t.elems            = elems;
  return arena.intern(t);
}
type::ID type::Factory::make_prototype(const std::vector<type::Prototype_Param>& params, type::ID ret, bool is_variadic,
                                       Qualifier qualifier)
{
  assert(ret && "Return must have a type or u0");
  assert(params.size() != UINT64_MAX && "params corrupted");
  assert(params.capacity() != UINT64_MAX && "params corrupted");

  Prototype t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Prototype;
  t.params           = params;
  t.ret              = ret;
  t.is_variadic      = is_variadic;
  return arena.intern(t);
}
type::ID type::Factory::make_enum(const std::vector<type::ID>& variants, definition::ID sym, Qualifier qualifier)
{
  assert(!variants.empty() && "Illegal empty enum");

  Enum t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Enum;
  t.variants         = variants;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_flag(size_t size, definition::ID sym, Qualifier qualifier)
{
  assert(size != 0 && "Illegal empty flag");

  Flag t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Flag;
  t.size             = size;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_union(const std::vector<type::ID>& variants, definition::ID sym, Qualifier qualifier)
{
  assert(!variants.empty() && "Illegal empty union");

  Union t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Union;
  t.variants         = variants;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_facet(const std::vector<type::ID>& fields, definition::ID sym, Qualifier qualifier)
{
  Facet t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Facet;
  t.fields           = fields;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_view(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  assert(!facets.empty() && "Illegal empty view");

  View t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::View;
  t.facets           = facets;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_form(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  Form t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Form;
  t.facets           = facets;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Factory::make_forward_identifier(std::string_view name, Qualifier qualifier)
{
  assert(!name.empty() && "Illegal empty identifier name");
  assert(false && "INVALID FUNCTION");

  Identifier t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Identifier;
  return arena.intern(t);
}
type::ID type::Factory::make_identifier(std::string_view name, ast::ID nodeid, definition::ID sym, Qualifier qualifier)
{
  assert(!name.empty() && "Illegal empty identifier name");

  auto it = arena.resolved_identifiers.find(std::string(name));
  if (it != arena.resolved_identifiers.end()) return it->second;

  Identifier t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::Identifier;
  t.nodeid           = nodeid;
  t.def              = sym;
  auto tyid          = arena.intern(t);
  return tyid;
}
type::ID type::Factory::make_string(ETextType txt_ty, Qualifier qualifier)
{
  assert(txt_ty != ETextType::NONE && "Invalid text type");

  if (qualifier.is_pure()) {
    switch (txt_ty) {
    case ETextType::_str:  return TYPEID_str;
    case ETextType::_cstr: return TYPEID_cstr;
    case ETextType::_text: return TYPEID_text;
    case ETextType::_cune: return TYPEID_cune;
    case ETextType::_rune: return TYPEID_rune;
    default:               assert(txt_ty == ETextType::NONE && "Invalid text type");
    }
  }

  String t;
  t.header.qualifier = qualifier;
  t.header.kind      = ETypeKind::String;
  t.kind             = txt_ty;
  return arena.intern(t);
}

size_t type::Arena::hash_type(type::Primitive const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Primitive));
  h        = hash_combine(h, static_cast<size_t>(d.primitive));
  return h;
}

size_t type::Arena::hash_type(type::Ptr const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Ptr));
  h        = hash_combine(h, d.inner.index());
  return h;
}

size_t type::Arena::hash_type(type::Array const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Array));
  h        = hash_combine(h, d.inner.index());
  h        = hash_combine(h, d.size);
  return h;
}

size_t type::Arena::hash_type(type::Buffer const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Buffer));
  h        = hash_combine(h, d.inner.index());
  return h;
}

size_t type::Arena::hash_type(type::Slice const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Slice));
  h        = hash_combine(h, d.inner.index());
  h        = hash_combine(h, d.is_c_table);
  return h;
}

size_t type::Arena::hash_type(type::Tuple const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Tuple));

  for (auto const& e : d.elems) h = hash_combine(h, e.index());

  return h;
}

size_t type::Arena::hash_type(type::Prototype const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Prototype));

  for (auto const& p : d.params) h = hash_combine(h, p.type.index());

  h = hash_combine(h, d.ret.index());
  h = hash_combine(h, d.is_variadic);

  return h;
}

size_t type::Arena::hash_type(type::Enum const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Enum));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::Flag const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Flag));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::Union const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Union));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::Facet const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Facet));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::Form const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Form));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::View const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::View));
  h        = hash_combine(h, d.def.index());
  return h;
}

size_t type::Arena::hash_type(type::Identifier const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::Identifier));

  std::hash<std::string> hasher;
  h = hash_combine(h, d.nodeid.raw());
  h = hash_combine(h, d.tyid().index());

  return h;
}

size_t type::Arena::hash_type(type::String const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::String));
  h        = hash_combine(h, static_cast<uint8_t>(d.kind));
  return h;
}


void type::initialization() noexcept
{
  primitives.reserve(TYPEID_USER_START);
#define prim(prim_ty)                                                                                                  \
  auto* name##prim_ty        = new type::Primitive();                                                                  \
  name##prim_ty->header.tyid = TYPEID##prim_ty;                                                                        \
  name##prim_ty->primitive   = EPrimitiveTypeKind::prim_ty;                                                            \
  primitives.emplace(TYPEID##prim_ty, new TypeEntry(name##prim_ty, TYPEID##prim_ty, ETypeKind::Primitive));

  prim(_u0);
  prim(_bool);
  prim(_cune);
  prim(_rune);
  prim(_ssize);
  prim(_s8);
  prim(_s16);
  prim(_s32);
  prim(_s64);
  prim(_s128);
  prim(_usize);
  prim(_u8);
  prim(_u16);
  prim(_u32);
  prim(_u64);
  prim(_u128);
  prim(_bsize);
  prim(_b8);
  prim(_b16);
  prim(_b32);
  prim(_b64);
  prim(_b128);
  prim(_ptrdiff);
  prim(_fsize);
  prim(_f16);
  prim(_f32);
  prim(_f64);
  prim(_f80);
  prim(_f128);
  prim(_dsize);
  prim(_d32);
  prim(_d64);
  prim(_d128);
  prim(_udsize);
  prim(_ud32);
  prim(_ud64);
  prim(_ud128);
  prim(_opaque);

#undef prim

#define prim_txt(prim_ty)                                                                                              \
  auto* name##prim_ty        = new type::String();                                                                     \
  name##prim_ty->header.tyid = TYPEID##prim_ty;                                                                        \
  name##prim_ty->kind        = ETextType::prim_ty;                                                                     \
  primitives.emplace(type::TYPEID##prim_ty, new TypeEntry(name##prim_ty, TYPEID##prim_ty, ETypeKind::String));

  prim_txt(_cstr);
  prim_txt(_str);
  prim_txt(_text);

#undef prim_txt
}


type::TypeHeader& type::Arena::get(ID tyid) noexcept
{
  assert(tyid.cu() == cuid);

  size_t index = tyid.index();
  if (index >= 0 && index < TYPEID_USER_START) {
    auto* data = type::primitives.at(type::ID::make(cu::ID::main(), index))->data;
    return *static_cast<TypeHeader*>(data);
  }
  // canonical
  auto it = canon_identifier.find(tyid);
  if (it != canon_identifier.end()) return get(it->second);

  index -= TYPEID_USER_START;

  assert(index < entries.size());
  return *static_cast<TypeHeader*>(entries[index]->data);
}
const type::TypeHeader& type::Arena::get(ID id) const noexcept
{
  return const_cast<Arena*>(this)->get(id);
}
