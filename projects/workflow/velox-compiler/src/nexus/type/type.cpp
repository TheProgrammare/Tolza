#include "type.hpp"

#include <cassert>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string_view>

#include <common/compiler_options.hpp>
#include <common/utils.hpp>

#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "binder/binder_ffi.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "binder/binder_ffi.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"


#include <Neargye/magic_enum.hpp>


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

  auto t       = std::make_unique<type::Primitive>();
  t->qualifier = qualifier;
  t->primitive = p;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_ptr(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  auto t       = std::make_unique<type::Ptr>();
  t->qualifier = qualifier;
  t->inner     = inner;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_static_array(type::ID inner, size_t size, ast::ID size_expr, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  auto t             = std::make_unique<type::Array>();
  t->qualifier       = qualifier;
  t->inner           = inner;
  t->size            = size;
  t->size_expression = size_expr;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_dynamic_array(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  auto t       = std::make_unique<type::Buffer>();
  t->qualifier = qualifier;
  t->inner     = inner;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_slice(type::ID inner, Qualifier qualifier, bool is_c_table)
{
  assert(inner && "Invalid identifier");

  auto t        = std::make_unique<type::Slice>();
  t->qualifier  = qualifier;
  t->inner      = inner;
  t->is_c_table = is_c_table;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_tuple(const std::vector<type::ID>& elems, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Tuple>();
  t->qualifier = qualifier;
  t->elems     = elems;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_prototype(const std::vector<type::Prototype_Param>& params, type::ID ret,
                                              bool is_variadic, Qualifier qualifier)
{
  auto t         = std::make_unique<type::Prototype>();
  t->qualifier   = qualifier;
  t->params      = params;
  t->ret         = ret;
  t->is_variadic = is_variadic;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_enum(const std::vector<type::ID>& variants, definition::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Enum>();
  t->qualifier = qualifier;
  t->variants  = variants;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_flag(size_t size, definition::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Flag>();
  t->qualifier = qualifier;
  t->size      = size;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_union(const std::vector<type::ID>& variants, definition::ID sym,
                                          Qualifier qualifier)
{
  auto t       = std::make_unique<type::Union>();
  t->qualifier = qualifier;
  t->variants  = variants;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_facet(const std::vector<type::ID>& fields, definition::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Facet>();
  t->qualifier = qualifier;
  t->fields    = fields;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_view(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::View>();
  t->qualifier = qualifier;
  t->facets    = facets;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_form(const std::vector<type::ID>& facets, definition::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Form>();
  t->qualifier = qualifier;
  t->facets    = facets;
  t->def       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_forward_identifier(std::string_view name, Qualifier qualifier)
{
  auto t          = std::make_unique<type::Identifier>();
  t->qualifier    = qualifier;
  t->forward_name = std::string(name);
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_identifier(std::string_view name, ast::ID nodeid, definition::ID sym,
                                               Qualifier qualifier)
{
  auto it = arena.resolved_identifiers.find(std::string(name));
  if (it != arena.resolved_identifiers.end()) return it->second;

  auto t          = std::make_unique<type::Identifier>();
  t->qualifier    = qualifier;
  t->forward_name = std::string(name);
  t->nodeid       = nodeid;
  t->def          = sym;
  auto tyid       = arena.intern(std::move(t));
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

  auto t       = std::make_unique<type::String>();
  t->qualifier = qualifier;
  t->kind      = txt_ty;
  return arena.intern(std::move(t));
}


size_t type::Arena::hash_type(Type const& t) noexcept
{
  size_t h = hash_val(static_cast<uint8_t>(t.kind()));

  switch (t.kind()) {
  case ETypeKind::Primitive: {
    const auto* d = static_cast<const type::Primitive*>(&t);
    h             = hash_combine(h, static_cast<size_t>(d->primitive));
    break;
  }

  case ETypeKind::Ptr: {
    const auto* d = static_cast<const type::Ptr*>(&t);
    h             = hash_combine(h, d->inner.offset());
    break;
  }

  case ETypeKind::Array: {
    const auto* d = static_cast<const type::Array*>(&t);
    h             = hash_combine(h, d->inner.offset());
    h             = hash_combine(h, d->size);
    break;
  }

  case ETypeKind::Buffer: {
    const auto* d = static_cast<const type::Buffer*>(&t);
    h             = hash_combine(h, d->inner.offset());
    break;
  }

  case ETypeKind::Slice: {
    const auto* d = static_cast<const type::Slice*>(&t);
    h             = hash_combine(h, d->inner.offset());
    h             = hash_combine(h, d->is_c_table);
    break;
  }

  case ETypeKind::Tuple: {
    const auto* d = static_cast<const type::Tuple*>(&t);
    for (auto const& e : d->elems) h = hash_combine(h, e.offset());
    break;
  }

  case ETypeKind::Prototype: {
    const auto* d = static_cast<const type::Prototype*>(&t);

    for (auto const& p : d->params) h = hash_combine(h, p.type.offset());

    h = hash_combine(h, d->ret.offset());
    h = hash_combine(h, d->is_variadic);
    break;
  }

  case ETypeKind::Enum: {
    const auto* d = static_cast<const type::Enum*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::Flag: {
    const auto* d = static_cast<const type::Flag*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::Union: {
    const auto* d = static_cast<const type::Union*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::Facet: {
    const auto* d = static_cast<const type::Facet*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::Form: {
    const auto* d = static_cast<const type::Form*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::View: {
    const auto* d = static_cast<const type::View*>(&t);
    h             = hash_combine(h, d->def.offset());
    break;
  }

  case ETypeKind::Identifier: {
    auto const&            d = static_cast<const type::Identifier*>(&t);
    std::hash<std::string> hasher;
    auto                   h2 = hasher(d->forward_name);

    h = hash_combine(h, h2);
    h = hash_combine(h, d->tyid.offset());
    break;
  }

  case ETypeKind::String: {
    const auto* d = static_cast<const type::String*>(&t);
    h             = hash_combine(h, static_cast<uint8_t>(d->kind));
    break;
  }

  case ETypeKind::NONE: {
    assert(false);
  }
  }

  return h;
}

definition::ID type::Type::get_def_id() const noexcept
{
  switch (kind()) {
  case ETypeKind::Facet:      return static_cast<const type::Facet*>(this)->def;
  case ETypeKind::Form:       return static_cast<const type::Form*>(this)->def;
  case ETypeKind::Enum:       return static_cast<const type::Enum*>(this)->def;
  case ETypeKind::Flag:       return static_cast<const type::Flag*>(this)->def;
  case ETypeKind::Union:      return static_cast<const type::Union*>(this)->def;
  case ETypeKind::Identifier: return static_cast<const type::Identifier*>(this)->def;
  default:                    return NO_ID;
  }
}

bool type::Type::set_def_id(definition::ID defid) noexcept
{
  switch (kind()) {
  case ETypeKind::Facet:      static_cast<type::Facet*>(this)->def = defid; return true;
  case ETypeKind::Form:       static_cast<type::Form*>(this)->def = defid; return true;
  case ETypeKind::Enum:       static_cast<type::Enum*>(this)->def = defid; return true;
  case ETypeKind::Flag:       static_cast<type::Flag*>(this)->def = defid; return true;
  case ETypeKind::Union:      static_cast<type::Union*>(this)->def = defid; return true;
  case ETypeKind::Identifier: static_cast<type::Identifier*>(this)->def = defid; return true;
  default:                    return false;
  }
}


bool type::TypeEq::operator()(Type const& a, Type const& b) const noexcept
{
  if (a.kind() != b.kind()) return false;

  switch (a.kind()) {
  case ETypeKind::Primitive: {
    const auto* pa = static_cast<const type::Primitive*>(&a);
    const auto* pb = static_cast<const type::Primitive*>(&b);
    return pa->primitive == pb->primitive;
  }

  case ETypeKind::Ptr: {
    return static_cast<const type::Ptr*>(&a)->inner == static_cast<const type::Ptr*>(&b)->inner;
  }

  case ETypeKind::Array: {
    auto const& da = static_cast<const type::Array*>(&a);
    auto const& db = static_cast<const type::Array*>(&b);

    return da->inner == db->inner && da->size == db->size;
  }

  case ETypeKind::Buffer: {
    return static_cast<const type::Buffer*>(&a)->inner == static_cast<const type::Buffer*>(&b)->inner;
  }

  case ETypeKind::Slice: {
    return static_cast<const type::Slice*>(&a)->inner == static_cast<const type::Slice*>(&b)->inner;
  }

  case ETypeKind::Tuple: {
    return static_cast<const type::Tuple*>(&a)->elems == static_cast<const type::Tuple*>(&b)->elems;
  }

  case ETypeKind::Prototype: {
    auto const& da = static_cast<const type::Prototype*>(&a);
    auto const& db = static_cast<const type::Prototype*>(&b);

    return da->params == db->params && da->ret == db->ret;
  }

  case ETypeKind::Enum: {
    return static_cast<const type::Enum*>(&a)->def == static_cast<const type::Enum*>(&b)->def;
  }

  case ETypeKind::Flag: {
    return static_cast<const type::Flag*>(&a)->def == static_cast<const type::Flag*>(&b)->def;
  }

  case ETypeKind::Union: {
    return static_cast<const type::Union*>(&a)->def == static_cast<const type::Union*>(&b)->def;
  }

  case ETypeKind::Facet: {
    return static_cast<const type::Facet*>(&a)->def == static_cast<const type::Facet*>(&b)->def;
  }

  case ETypeKind::View: {
    return static_cast<const type::View*>(&a)->def == static_cast<const type::View*>(&b)->def;
  }

  case ETypeKind::Form: {
    return static_cast<const type::Form*>(&a)->def == static_cast<const type::Form*>(&b)->def;
  }

  case ETypeKind::Identifier: {
    const auto* a_id = static_cast<const type::Identifier*>(&a);
    const auto* b_id = static_cast<const type::Identifier*>(&b);
    return a_id->tyid == b_id->tyid && a_id->forward_name == b_id->forward_name;
  }

  case ETypeKind::String: break;
  case ETypeKind::NONE:   {
    assert(false);
  }
  }

  return false;
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

type::ID type::get_inner(type::ID nodeid) noexcept
{
  if (const auto* n = nodeid.as<type::Ptr>()) return n->inner;
  if (const auto* n = nodeid.as<type::Array>()) return n->inner;
  if (const auto* n = nodeid.as<type::Buffer>()) return n->inner;
  if (const auto* n = nodeid.as<type::Slice>()) return n->inner;

  return NO_ID;
}


void type::initialization() noexcept
{
  primitives.reserve(TYPEID_USER_START);
#define prim(prim_ty)                                                                                                  \
  auto* name##prim_ty         = new type::Primitive();                                                                 \
  name##prim_ty->tyid         = TYPEID##prim_ty;                                                                       \
  name##prim_ty->primitive    = EPrimitiveTypeKind::prim_ty;                                                           \
  primitives[TYPEID##prim_ty] = std::unique_ptr<type::Primitive>(name##prim_ty);

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
  auto* name##prim_ty               = new type::String();                                                              \
  name##prim_ty->tyid               = TYPEID##prim_ty;                                                                 \
  name##prim_ty->kind               = ETextType::prim_ty;                                                              \
  primitives[type::TYPEID##prim_ty] = std::unique_ptr<type::String>(name##prim_ty);

  prim_txt(_cstr);
  prim_txt(_str);
  prim_txt(_text);

#undef prim_txt
}

std::string type::dump(ID tyid) noexcept
{
  const auto& ptr_ty = tyid.get();

  switch (tyid.kind()) {
  case ETypeKind::NONE:      return "";
  case ETypeKind::Primitive: return std::string(magic_enum::enum_name(tyid.as<type::Primitive>()->primitive).substr(1));
  case ETypeKind::String:    return std::string(magic_enum::enum_name(tyid.as<type::String>()->kind).substr(1));
  case ETypeKind::Tuple:     {
    std::string out;
    const auto* tu = static_cast<const type::Tuple*>(&ptr_ty);
    for (const auto& tyid : tu->elems) {
      out += tyid.dump();
      out += ", ";
    }

    return "(" + out.substr(0, out.size() - 2) + ")";
  }
  case ETypeKind::Array: {
    const auto* ptr = static_cast<const type::Array*>(&ptr_ty);
    return "[" + ptr->inner.dump() + "; " + std::to_string(ptr->size) + "]";
  }
  case ETypeKind::Buffer: {
    const auto* ptr = static_cast<const type::Buffer*>(&ptr_ty);
    return "[" + ptr->inner.dump() + "; _]";
  }
  case ETypeKind::Slice: {
    const auto*       ptr  = static_cast<const type::Slice*>(&ptr_ty);
    const std::string mode = ptr->is_c_table ? "c" : "..";
    return "[" + ptr->inner.dump() + "; " + mode + "]";
  }
  case ETypeKind::Ptr: {
    const auto* ptr = static_cast<const type::Ptr*>(&ptr_ty);
    return "ptr'" + ptr->inner.dump();
  }
  case ETypeKind::Prototype: {
    std::string params;
    const auto* proto = static_cast<const type::Prototype*>(&ptr_ty);
    for (const auto& param : proto->params) {
      params += param.type.dump();
      params += ", ";
    }
    params = params.substr(0, params.size() - 2);

    return "fn(" + params + ") -> " + proto->ret.dump();
  }
  case ETypeKind::Facet: {
    const auto* facet = static_cast<const type::Facet*>(&ptr_ty);
    const auto* node  = facet->def.node().as<ast::SFM_Facet>();
    assert(node && "type node must be a facet");

    std::string fields;
    for (const auto& field : node->fields) {
      const auto* f_node = field.as<ast::SFM_Facet_Field>();
      assert(f_node && "a facet must have field nodes");

      fields += std::string(f_node->name) + ": " + f_node->type.dump() + "\n";
    }

    return "facet" + std::string(node->name) + " {\n" + fields + "}\n";
  }
  case ETypeKind::View: {
    const auto* view = static_cast<const type::View*>(&ptr_ty);
    assert(view->def && "a view type must refer to a symbol");
  }
  case ETypeKind::Identifier: {
    const auto* id = static_cast<const type::Identifier*>(&ptr_ty);
    return id->forward_name;
  }
  case ETypeKind::Form:
  case ETypeKind::Enum:
  case ETypeKind::Flag:
  case ETypeKind::Union: break;
  }

  return "";
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


type::Type& type::Arena::get(ID id) noexcept
{
  size_t offset = id.offset();
  if (offset >= 0 && offset < TYPEID_USER_START)
    return *type::primitives.at(type::ID::make(cu::ID::main(), offset)).get();

  // canonical
  auto it = canon_identifier.find(id);
  if (it != canon_identifier.end()) return get(it->second);

  offset -= TYPEID_USER_START;

  assert(offset < types.size());
  return *types[offset];
}
const type::Type& type::Arena::get(ID id) const noexcept
{
  size_t offset = id.offset();
  if (offset >= 0 && offset < TYPEID_USER_START)
    return *type::primitives.at(type::ID::make(cu::ID::main(), offset)).get();

  // canonical
  auto it = canon_identifier.find(id);
  if (it != canon_identifier.end()) return get(it->second);

  offset -= TYPEID_USER_START;

  assert(offset < types.size());
  return *types[offset];
}

type::Type& type::get(ID id) noexcept
{
  size_t offset = id.offset();
  if (offset >= 0 && offset < TYPEID_USER_START) return *primitives.at(type::ID::make(cu::ID::main(), offset)).get();

  auto& cu = id.cu().get();
  return cu.types->get(id);
}

template <type::IsDataType T>
T* type::as(ID id) noexcept
{
  auto* ty = &get(id);
  assert(ty->kind() == T::static_kind && "Must derivate to Type");
  return static_cast<T*>(ty);
}


#define TYPE_GET_INSTANCE(T) template T* type::as<T>(ID id) noexcept;

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

#undef TYPE_GET_INSTANCE
