#include "type.hpp"

#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"

#include <Neargye/magic_enum.hpp>
#include <cassert>
#include <common/compiler_options.hpp>
#include <common/utils.hpp>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string_view>


type::Arena::Arena(cu::ID _cuid)
  : cuid(_cuid)
{
}


type::ID type::Arena::Factory::make_primitive(EPrimitiveTypeKind p, Qualifier qualifier)
{
  assert(p != EPrimitiveTypeKind::NONE && "illegal primitive (EPrimitiveTypeKind::NONE)");

  if (qualifier.is_pure()) {
    const auto main_ty = ID::make_primitive(p);
    return main_ty;
  }

  type::Primitive t;
  t.header.qualifier = qualifier;
  t.primitive        = p;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_ptr(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  Ptr t;
  t.header.qualifier = qualifier;
  t.inner            = inner;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_static_array(type::ID inner, size_t size, ast::ID size_expr, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");
  assert(size != 0 || size_expr && "Impossible static size");

  Array t;
  t.header.qualifier = qualifier;
  t.inner            = inner;
  t.size             = size;
  t.size_expression  = size_expr;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_dynamic_array(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  Buffer t;
  t.header.qualifier = qualifier;
  t.inner            = inner;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_slice(type::ID inner, Qualifier qualifier, bool is_c_table)
{
  assert(inner && "Invalid identifier");

  Slice t;
  t.header.qualifier = qualifier;
  t.inner            = inner;
  t.is_c_table       = is_c_table;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_tuple(const std::vector<type::ID>& elems, Qualifier qualifier)
{
  assert(!elems.empty() && "Illegal empty tuple");

  Tuple t;
  t.header.qualifier = qualifier;
  t.elems            = elems;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_prototype(const std::vector<type::Prototype_Param>& params, type::ID ret,
                                              bool is_variadic, Qualifier qualifier)
{
  assert(ret && "Return must have a type or u0");
  assert(params.size() != std::numeric_limits<size_t>::max() && "params corrupted");
  assert(params.capacity() != std::numeric_limits<size_t>::max() && "params corrupted");

  Prototype t;
  t.header.qualifier = qualifier;
  t.params           = params;
  t.ret              = ret;
  t.is_variadic      = is_variadic;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_enum(const std::vector<type::ID>& variants, definition::ID sym, Qualifier qualifier)
{
  assert(!variants.empty() && "Illegal empty enum");

  Enum t;
  t.header.qualifier = qualifier;
  t.variants         = variants;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_flag(size_t size, definition::ID sym, Qualifier qualifier)
{
  assert(size != 0 && "Illegal empty flag");

  Flag t;
  t.header.qualifier = qualifier;
  t.size             = size;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_union(const std::vector<type::ID>& variants, definition::ID sym,
                                          Qualifier qualifier)
{
  assert(!variants.empty() && "Illegal empty union");

  Union t;
  t.header.qualifier = qualifier;
  t.variants         = variants;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_facet(const std::vector<type::ID>& fields, definition::ID sym, Qualifier qualifier)
{
  Facet t;
  t.header.qualifier = qualifier;
  t.fields           = fields;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_view(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  assert(!facets.empty() && "Illegal empty view");

  View t;
  t.header.qualifier = qualifier;
  t.facets           = facets;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_form(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  Form t;
  t.header.qualifier = qualifier;
  t.facets           = facets;
  t.def              = sym;
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_forward_identifier(std::string_view name, Qualifier qualifier)
{
  assert(!name.empty() && "Illegal empty identifier name");

  Identifier t;
  t.header.qualifier = qualifier;
  t.forward_name     = std::string(name);
  return arena.intern(t);
}
type::ID type::Arena::Factory::make_identifier(std::string_view name, ast::ID nodeid, definition::ID sym,
                                               Qualifier qualifier)
{
  assert(!name.empty() && "Illegal empty identifier name");

  auto it = arena.resolved_identifiers.find(std::string(name));
  if (it != arena.resolved_identifiers.end()) return it->second;

  Identifier t;
  t.header.qualifier = qualifier;
  t.forward_name     = std::string(name);
  t.nodeid           = nodeid;
  t.def              = sym;
  auto tyid          = arena.intern(t);
  return tyid;
}
type::ID type::Arena::Factory::make_string(ETextType txt_ty, Qualifier qualifier)
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
  h = hash_combine(h, hasher(d.forward_name));
  h = hash_combine(h, d.tyid().index());

  return h;
}

size_t type::Arena::hash_type(type::String const& d) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(ETypeKind::String));
  h        = hash_combine(h, static_cast<uint8_t>(d.kind));
  return h;
}


bool type::is_same_type(type::Primitive const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Primitive>();
  return t.primitive == d->primitive;
}

bool type::is_same_type(type::Ptr const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Ptr>();
  return t.inner == d->inner;
}

bool type::is_same_type(type::Array const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Array>();
  return t.inner == d->inner && t.size == d->size;
}

bool type::is_same_type(type::Buffer const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Buffer>();
  return t.inner == d->inner;
}

bool type::is_same_type(type::Slice const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Slice>();
  return t.inner == d->inner && t.is_c_table == d->is_c_table;
}

bool type::is_same_type(type::Tuple const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Tuple>();
  return t.elems == d->elems;
}

bool type::is_same_type(type::Prototype const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Prototype>();
  return t.params == d->params && t.ret == d->ret && t.is_variadic == d->is_variadic;
}

bool type::is_same_type(type::Enum const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Enum>();
  return t.def == d->def;
}

bool type::is_same_type(type::Flag const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Flag>();
  return t.def == d->def;
}

bool type::is_same_type(type::Union const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Union>();
  return t.def == d->def;
}

bool type::is_same_type(type::Facet const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Facet>();
  return t.def == d->def;
}

bool type::is_same_type(type::Form const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Form>();
  return t.def == d->def;
}

bool type::is_same_type(type::View const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::View>();
  return t.def == d->def;
}

bool type::is_same_type(type::Identifier const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::Identifier>();
  return t.header.tyid == tyid && t.forward_name == d->forward_name;
}

bool type::is_same_type(type::String const& t, ID tyid) noexcept
{
  if (t.header.kind != tyid.kind()) return false;

  auto const* d = tyid.as<type::String>();
  return t.kind == d->kind;
}


std::string_view type::EPrimitiveTypeKind_to_mangle(EPrimitiveTypeKind type) noexcept
{
  switch (type) {
  case EPrimitiveTypeKind::_bool:    return "b";
  case EPrimitiveTypeKind::_cune:    return "cu";
  case EPrimitiveTypeKind::_rune:    return "ru";

  case EPrimitiveTypeKind::_u0:      return "u0";
  case EPrimitiveTypeKind::_opaque:  return "ptrt";
  case EPrimitiveTypeKind::_ptrdiff: return "pdif";

  case EPrimitiveTypeKind::_ssize:   return "isz";
  case EPrimitiveTypeKind::_s8:      return "i8";
  case EPrimitiveTypeKind::_s16:     return "i16";
  case EPrimitiveTypeKind::_s32:     return "i32";
  case EPrimitiveTypeKind::_s64:     return "i64";
  case EPrimitiveTypeKind::_s128:    return "i128";

  case EPrimitiveTypeKind::_usize:   return "usz";
  case EPrimitiveTypeKind::_u8:      return "u8";
  case EPrimitiveTypeKind::_u16:     return "u16";
  case EPrimitiveTypeKind::_u32:     return "u32";
  case EPrimitiveTypeKind::_u64:     return "u64";
  case EPrimitiveTypeKind::_u128:    return "u128";

  case EPrimitiveTypeKind::_bsize:   return "bsz";
  case EPrimitiveTypeKind::_b8:      return "b8";
  case EPrimitiveTypeKind::_b16:     return "b16";
  case EPrimitiveTypeKind::_b32:     return "b32";
  case EPrimitiveTypeKind::_b64:     return "b64";
  case EPrimitiveTypeKind::_b128:    return "b128";

  case EPrimitiveTypeKind::_fsize:   return "fsz";
  case EPrimitiveTypeKind::_f16:     return "f16";
  case EPrimitiveTypeKind::_f32:     return "f32";
  case EPrimitiveTypeKind::_f64:     return "f64";
  case EPrimitiveTypeKind::_f80:     return "f80";
  case EPrimitiveTypeKind::_f128:    return "f128";

  case EPrimitiveTypeKind::_dsize:   return "dsz";
  case EPrimitiveTypeKind::_d32:     return "d32";
  case EPrimitiveTypeKind::_d64:     return "d64";
  case EPrimitiveTypeKind::_d128:    return "d128";

  case EPrimitiveTypeKind::_udsize:  return "udsz";
  case EPrimitiveTypeKind::_ud32:    return "ud32";
  case EPrimitiveTypeKind::_ud64:    return "ud64";
  case EPrimitiveTypeKind::_ud128:   return "ud128";

  case EPrimitiveTypeKind::NONE:     return "NO PRIMITIVE TYPE";
  }
}

type::EPrimitiveTypeKind type::ETokenKind_to_EPrimitiveTypeKind(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::T_BOOL:  return EPrimitiveTypeKind::_bool;
  case token::ETokenKind::T_RUNE:  return EPrimitiveTypeKind::_rune;
  case token::ETokenKind::T_CUNE:  return EPrimitiveTypeKind::_cune;

  case token::ETokenKind::T_SSIZE: return EPrimitiveTypeKind::_ssize;
  case token::ETokenKind::T_S8:    return EPrimitiveTypeKind::_s8;
  case token::ETokenKind::T_S16:   return EPrimitiveTypeKind::_s16;
  case token::ETokenKind::T_S32:   return EPrimitiveTypeKind::_s32;
  case token::ETokenKind::T_S64:   return EPrimitiveTypeKind::_s64;
  case token::ETokenKind::T_S128:  return EPrimitiveTypeKind::_s128;

  case token::ETokenKind::T_USIZE: return EPrimitiveTypeKind::_usize;
  case token::ETokenKind::T_U8:    return EPrimitiveTypeKind::_u8;
  case token::ETokenKind::T_U16:   return EPrimitiveTypeKind::_u16;
  case token::ETokenKind::T_U32:   return EPrimitiveTypeKind::_u32;
  case token::ETokenKind::T_U64:   return EPrimitiveTypeKind::_u64;
  case token::ETokenKind::T_U128:  return EPrimitiveTypeKind::_u128;

  case token::ETokenKind::T_BSIZE: return EPrimitiveTypeKind::_bsize;
  case token::ETokenKind::T_B8:    return EPrimitiveTypeKind::_b8;
  case token::ETokenKind::T_B16:   return EPrimitiveTypeKind::_b16;
  case token::ETokenKind::T_B32:   return EPrimitiveTypeKind::_b32;
  case token::ETokenKind::T_B64:   return EPrimitiveTypeKind::_b64;
  case token::ETokenKind::T_B128:  return EPrimitiveTypeKind::_b128;

  case token::ETokenKind::T_FSIZE: return EPrimitiveTypeKind::_fsize;
  case token::ETokenKind::T_F16:   return EPrimitiveTypeKind::_f16;
  case token::ETokenKind::T_F32:   return EPrimitiveTypeKind::_f32;
  case token::ETokenKind::T_F64:   return EPrimitiveTypeKind::_f64;
  case token::ETokenKind::T_F80:   return EPrimitiveTypeKind::_f80;
  case token::ETokenKind::T_F128:  return EPrimitiveTypeKind::_f128;

  case token::ETokenKind::T_U0:    return EPrimitiveTypeKind::_u0;

  default:                         return EPrimitiveTypeKind::NONE;
  }
}

std::string_view type::EPtrType_to_str(EPtrType type) noexcept
{
  switch (type) {
  case EPtrType::raw_ptr:    return "ptr";
  case EPtrType::unique_ptr: return "uptr";
  case EPtrType::shared_ptr: return "sptr";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}

std::string_view type::EPtrType_to_mangle(EPtrType type) noexcept
{
  switch (type) {
  case EPtrType::raw_ptr:    return "p";
  case EPtrType::unique_ptr: return "up";
  case EPtrType::shared_ptr: return "sp";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}


size_t type::EPrimitiveTypeKind_to_bits(EPrimitiveTypeKind type) noexcept
{
  switch (type) {
  case EPrimitiveTypeKind::_bool:    return 1;
  case EPrimitiveTypeKind::_cune:    return 8;
  case EPrimitiveTypeKind::_rune:    return 32;
  case EPrimitiveTypeKind::NONE:
  case EPrimitiveTypeKind::_u0:
  case EPrimitiveTypeKind::_opaque:
  case EPrimitiveTypeKind::_ptrdiff:
  case EPrimitiveTypeKind::_fsize:
  case EPrimitiveTypeKind::_dsize:
  case EPrimitiveTypeKind::_udsize:
  case EPrimitiveTypeKind::_ssize:
  case EPrimitiveTypeKind::_usize:
  case EPrimitiveTypeKind::_bsize:   return compiler::OPTIONS.target.get_arch_size();
  case EPrimitiveTypeKind::_s8:
  case EPrimitiveTypeKind::_u8:
  case EPrimitiveTypeKind::_b8:      return 8;
  case EPrimitiveTypeKind::_f16:
  case EPrimitiveTypeKind::_s16:
  case EPrimitiveTypeKind::_u16:
  case EPrimitiveTypeKind::_b16:     return 16;
  case EPrimitiveTypeKind::_f32:
  case EPrimitiveTypeKind::_d32:
  case EPrimitiveTypeKind::_ud32:
  case EPrimitiveTypeKind::_s32:
  case EPrimitiveTypeKind::_u32:
  case EPrimitiveTypeKind::_b32:     return 32;
  case EPrimitiveTypeKind::_f64:
  case EPrimitiveTypeKind::_d64:
  case EPrimitiveTypeKind::_ud64:
  case EPrimitiveTypeKind::_s64:
  case EPrimitiveTypeKind::_u64:
  case EPrimitiveTypeKind::_b64:     return 64;
  case EPrimitiveTypeKind::_f128:
  case EPrimitiveTypeKind::_d128:
  case EPrimitiveTypeKind::_ud128:
  case EPrimitiveTypeKind::_s128:
  case EPrimitiveTypeKind::_u128:
  case EPrimitiveTypeKind::_b128:    return 128;
  case EPrimitiveTypeKind::_f80:     return 80;
  }
}

size_t type::EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type) noexcept
{
  return EPrimitiveTypeKind_to_bits(type) / 8;
}

bool type::is_op_handled(EPrimitiveTypeKind src, ast::EOp_Bin op) noexcept
{
  // return
  // op_primitive[static_cast<uint8_t>(op)][static_cast<uint8_t>(src)];
  return false;
}

bool type::is_cast_explicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept
{
  // return
  // cast_explicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}

bool type::is_cast_implicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target) noexcept
{
  // return
  // cast_implicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}

type::ETextType type::ETokenKind_to_ETextType(token::ETokenKind tok) noexcept
{
  switch (tok) {
  case token::ETokenKind::T_TEXT: return ETextType::_text;
  case token::ETokenKind::T_STR:  return ETextType::_str;
  case token::ETokenKind::T_CSTR: return ETextType::_cstr;
  case token::ETokenKind::T_CUNE: return ETextType::_cune;
  case token::ETokenKind::T_RUNE: return ETextType::_rune;
  default:                        return ETextType::NONE;
  }
}


type::ID type::get_prototype(ast::ID nodeid) noexcept
{
  if (const auto* n = nodeid.as<ast::Global_Function>()) return n->prototype;
  if (const auto* n = nodeid.as<ast::Local_Lambda>()) return n->prototype;
  if (const auto* n = nodeid.as<ast::SFM_Rule>()) return n->prototype;

  return NO_ID;
}

type::ID type::get_inner(type::ID tyid) noexcept
{
  if (const auto* n = tyid.as<type::Ptr>()) return n->inner;
  if (const auto* n = tyid.as<type::Array>()) return n->inner;
  if (const auto* n = tyid.as<type::Buffer>()) return n->inner;
  if (const auto* n = tyid.as<type::Slice>()) return n->inner;

  return NO_ID;
}


void type::initialization() noexcept
{
  primitives.reserve(TYPEID_USER_START);
#define prim(prim_ty)                                                                                                  \
  ;                                                                                                                    \
  auto name##prim_ty        = type::Primitive();                                                                       \
  name##prim_ty.header.tyid = TYPEID##prim_ty;                                                                         \
  name##prim_ty.primitive   = EPrimitiveTypeKind::prim_ty;                                                             \
  primitives.emplace(TYPEID##prim_ty, TypeEntry(TYPEID##prim_ty, ETypeKind::Primitive, std::move(name##prim_ty)));

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
  auto name##prim_ty        = type::String();                                                                          \
  name##prim_ty.header.tyid = TYPEID##prim_ty;                                                                         \
  name##prim_ty.kind        = ETextType::prim_ty;                                                                      \
  primitives.emplace(type::TYPEID##prim_ty, TypeEntry(TYPEID##prim_ty, ETypeKind::String, std::move(name##prim_ty)));

  prim_txt(_cstr);
  prim_txt(_str);
  prim_txt(_text);

#undef prim_txt
}

std::string type::dump(ID tyid) noexcept
{
  const auto& ptr_ty = tyid.get();

  switch (tyid.kind()) {
  case ETypeKind::NONE:      return {};
  case ETypeKind::Primitive: return std::string(magic_enum::enum_name(tyid.as<type::Primitive>()->primitive).substr(1));
  case ETypeKind::String:    return std::string(magic_enum::enum_name(tyid.as<type::String>()->kind).substr(1));
  case ETypeKind::Tuple:     {
    std::string out;
    const auto* tu = tyid.as<type::Tuple>();
    out.reserve(tu->elems.size() * 32);
    for (const auto& tyid : tu->elems) {
      std::format_to(std::back_inserter(out), "{}, ", tyid.dump());
    }

    return std::format("({})", out.substr(0, out.size() - 2));
  }
  case ETypeKind::Array: {
    const auto* ptr = tyid.as<type::Array>();
    return std::format("[{}; {}]", ptr->inner.dump(), std::to_string(ptr->size));
  }
  case ETypeKind::Buffer: {
    const auto* ptr = tyid.as<type::Buffer>();
    return std::format("[{}; _]", ptr->inner.dump());
  }
  case ETypeKind::Slice: {
    const auto* ptr = tyid.as<type::Slice>();
    return std::format("[{}; {}]", ptr->inner.dump(), ptr->is_c_table ? "c" : "..");
  }
  case ETypeKind::Ptr: {
    const auto* ptr = tyid.as<type::Ptr>();
    return std::format("ptr'{}", ptr->inner.dump());
  }
  case ETypeKind::Prototype: {
    std::string params;
    params.reserve(params.size() * 32);
    const auto* proto = tyid.as<type::Prototype>();
    for (const auto& param : proto->params) {
      std::format_to(std::back_inserter(params), "{} {}, ", magic_enum::enum_name(param.passmode), param.type.dump());
    }
    params = params.substr(0, params.size() - 2);

    return std::format("fn({}) -> {}", params, proto->ret.dump());
  }
  case ETypeKind::Facet: {
    const auto* facet = tyid.as<type::Facet>();
    const auto* node  = facet->def.node().as<ast::SFM_Facet>();
    assert(node && "type node must be a facet");

    std::string fields;
    fields.reserve(node->fields.size() * 32);
    for (const auto& field : node->fields) {
      const auto* f_node = field.as<ast::SFM_Facet_Field>();
      assert(f_node && "a facet must have field nodes");

      std::format_to(std::back_inserter(fields), "{}: {}\n", f_node->name, f_node->type.dump());
    }

    return std::format("facet {}{{\n{}}}\n", node->name, fields);
  }
  case ETypeKind::View: {
    const auto* view = tyid.as<type::View>();
    assert(view->def && "a view type must refer to a symbol");
  }
  case ETypeKind::Identifier: {
    const auto* id = tyid.as<type::Identifier>();
    return id->forward_name;
  }
  case ETypeKind::Form:
  case ETypeKind::Enum:
  case ETypeKind::Flag:
  case ETypeKind::Union: break;
  }

  return {};
}

std::vector<type::Prototype_Param> type::to_proto_params(const std::vector<ast::ID>& params) noexcept
{
  std::vector<type::Prototype_Param> out;
  out.reserve(params.size());

  for (auto nodeid : params) {
    auto* param = nodeid.as<ast::Local_Parameter>();
    assert(param);

    out.emplace_back(type::Prototype_Param{
        .passmode    = param->passmode,
        .type        = param->type,
        .nodeid      = nodeid,
        .is_restrict = param->is_restrict,
    });
  }

  return out;
}


type::TypeHeader& type::Arena::get(ID id) noexcept
{
  size_t index = id.index();
  if (index >= 0 && index < TYPEID_USER_START) {
    auto& data = type::primitives.at(type::ID::make(cu::ID::main(), index)).data;
    return std::visit([](auto& value) -> TypeHeader& { return value.header; }, data);
  }
  // canonical
  auto it = canon_identifier.find(id);
  if (it != canon_identifier.end()) return get(it->second);

  index -= TYPEID_USER_START;

  assert(index < types.size());
  return std::visit([](auto& value) -> TypeHeader& { return value.header; }, types[index].data);
}
const type::TypeHeader& type::Arena::get(ID id) const noexcept
{
  size_t index = id.index();
  if (index >= 0 && index < TYPEID_USER_START) {
    auto& data = type::primitives.at(type::ID::make(cu::ID::main(), index)).data;
    return std::visit([](auto& value) -> const TypeHeader& { return value.header; }, data);
  }

  // canonical
  auto it = canon_identifier.find(id);
  if (it != canon_identifier.end()) return get(it->second);

  index -= TYPEID_USER_START;

  assert(index < types.size());
  return std::visit([](auto& value) -> const TypeHeader& { return value.header; }, types[index].data);
}
