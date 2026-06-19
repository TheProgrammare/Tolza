#include "type.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <string_view>

#include <common/compiler_options.hpp>
#include <common/utils.hpp>

#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"


#include <Neargye/magic_enum.hpp>


type::Arena::Arena(cu::ID _cuid)
  : cuid(_cuid)
{
}


type::ID type::Arena::Factory::make_primitive(EPrimitiveTypeKind p, Qualifier qualifier)
{
  assert(p != EPrimitiveTypeKind::NONE && "illegal primitive (EPrimitiveTypeKind::NONE)");

  if (qualifier.is_pure()) {
    const auto main_ty = ID::make_primitive(cu::ID::main(), p);
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
type::ID type::Arena::Factory::make_static_array(type::ID inner, size_t size, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  auto t       = std::make_unique<type::StaticArray>();
  t->qualifier = qualifier;
  t->inner     = inner;
  t->size      = size;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_dynamic_array(type::ID inner, Qualifier qualifier)
{
  assert(inner && "Invalid identifier");

  auto t       = std::make_unique<type::DynamicArray>();
  t->qualifier = qualifier;
  t->inner     = inner;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_tuple(const std::vector<type::ID>& elems, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Tuple>();
  t->qualifier = qualifier;
  t->elems     = elems;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_prototype(const std::vector<type::Prototype::Param>& params, type::ID ret,
                                              bool is_variadic, Qualifier qualifier)
{
  auto t         = std::make_unique<type::Prototype>();
  t->qualifier   = qualifier;
  t->params      = params;
  t->ret         = ret;
  t->is_variadic = is_variadic;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_enum(const std::vector<type::ID>& variants, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Enum>();
  t->qualifier = qualifier;
  t->variants  = variants;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_flag(size_t size, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Flag>();
  t->qualifier = qualifier;
  t->size      = size;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_union(const std::vector<type::ID>& variants, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Union>();
  t->qualifier = qualifier;
  t->variants  = variants;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_facet(const std::vector<type::ID>& fields, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Facet>();
  t->qualifier = qualifier;
  t->fields    = fields;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_view(const std::vector<type::ID>& facets, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::View>();
  t->qualifier = qualifier;
  t->facets    = facets;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_form(const std::vector<type::ID>& facets, symbol::ID sym, Qualifier qualifier)
{
  auto t       = std::make_unique<type::Form>();
  t->qualifier = qualifier;
  t->facets    = facets;
  t->sym       = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_forward_identifier(std::string_view name, Qualifier qualifier)
{
  auto t          = std::make_unique<type::Identifier>();
  t->qualifier    = qualifier;
  t->forward_name = std::string(name);
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_identifier(std::string_view name, ast::ID nodeid, symbol::ID sym,
                                               Qualifier qualifier)
{
  auto t          = std::make_unique<type::Identifier>();
  t->qualifier    = qualifier;
  t->forward_name = std::string(name);
  t->nodeid       = nodeid;
  t->sym          = sym;
  return arena.intern(std::move(t));
}
type::ID type::Arena::Factory::make_string(ETextType txt_ty, Qualifier qualifier)
{
  assert(txt_ty != ETextType::NONE && "Invalid text type");

  if (qualifier.is_pure()) {
    switch (txt_ty) {
    case ETextType::_str:   return TYPEID_str;
    case ETextType::_c_str: return TYPEID_c_str;
    case ETextType::_text:  return TYPEID_text;
    case ETextType::_cune:  return TYPEID_cune;
    case ETextType::_rune:  return TYPEID_rune;
    default:                assert(txt_ty == ETextType::NONE && "Invalid text type");
    }
  }

  auto t       = std::make_unique<type::String>();
  t->qualifier = qualifier;
  t->kind      = txt_ty;
  return arena.intern(std::move(t));
}

type::ID type::Identifier::get_type() const noexcept
{
  return sym.node().type();
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

  case ETypeKind::StaticArray: {
    const auto* d = static_cast<const type::StaticArray*>(&t);
    h             = hash_combine(h, d->inner.offset());
    h             = hash_combine(h, d->size);
    break;
  }

  case ETypeKind::DynamicArray: {
    const auto* d = static_cast<const type::DynamicArray*>(&t);
    h             = hash_combine(h, d->inner.offset());
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
    h             = hash_combine(h, d->sym.offset());
    break;
  }

  case ETypeKind::Flag: {
    const auto* d = static_cast<const type::Flag*>(&t);
    h             = hash_combine(h, d->sym.offset());
    break;
  }

  case ETypeKind::Union: {
    const auto* d = static_cast<const type::Union*>(&t);
    h             = hash_combine(h, d->sym.offset());
    break;
  }

  case ETypeKind::Facet: {
    const auto* d = static_cast<const type::Facet*>(&t);
    h             = hash_combine(h, d->sym.offset());
    break;
  }

  case ETypeKind::Form: {
    const auto* d = static_cast<const type::Form*>(&t);
    h             = hash_combine(h, d->sym.offset());
    break;
  }

  case ETypeKind::View: {
    const auto* d = static_cast<const type::View*>(&t);
    h             = hash_combine(h, d->sym.offset());
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

symbol::ID type::Type::get_sym_id() const noexcept
{
  switch (kind()) {
  case ETypeKind::Facet:      return static_cast<const type::Facet*>(this)->sym;
  case ETypeKind::Form:       return static_cast<const type::Form*>(this)->sym;
  case ETypeKind::Enum:       return static_cast<const type::Enum*>(this)->sym;
  case ETypeKind::Flag:       return static_cast<const type::Flag*>(this)->sym;
  case ETypeKind::Union:      return static_cast<const type::Union*>(this)->sym;
  case ETypeKind::Identifier: return static_cast<const type::Identifier*>(this)->sym;
  default:                    return NO_ID;
  }
}

bool type::Type::set_sym_id(symbol::ID symid) noexcept
{
  switch (kind()) {
  case ETypeKind::Facet:      static_cast<type::Facet*>(this)->sym = symid; return true;
  case ETypeKind::Form:       static_cast<type::Form*>(this)->sym = symid; return true;
  case ETypeKind::Enum:       static_cast<type::Enum*>(this)->sym = symid; return true;
  case ETypeKind::Flag:       static_cast<type::Flag*>(this)->sym = symid; return true;
  case ETypeKind::Union:      static_cast<type::Union*>(this)->sym = symid; return true;
  case ETypeKind::Identifier: static_cast<type::Identifier*>(this)->sym = symid; return true;
  default:                    return false;
  }
}

std::string type::Type::velox_codegen() const noexcept
{
  switch (kind()) {
  case ETypeKind::NONE:      return "";
  case ETypeKind::Primitive: return GET_ENUM_NAME(static_cast<const type::Primitive*>(this)->primitive).substr(1);
  case ETypeKind::String:    return GET_ENUM_NAME(static_cast<const type::String*>(this)->kind).substr(1);
  case ETypeKind::Tuple:     {
    std::string out;
    const auto* tu = static_cast<const type::Tuple*>(this);
    for (const auto& tyid : tu->elems) {
      out += tyid.get().velox_codegen();
      out += ", ";
    }

    return "(" + out.substr(0, out.size() - 2) + ")";
  }
  case ETypeKind::StaticArray:
  case ETypeKind::Ptr:         {
    const auto* ptr = static_cast<const type::Ptr*>(this);
    return "ptr'" + ptr->inner.get().velox_codegen();
  }
  case ETypeKind::DynamicArray:
  case ETypeKind::Prototype:    {
    std::string params;
    const auto* proto = static_cast<const type::Prototype*>(this);
    for (const auto& param : proto->params) {
      params += param.type.get().velox_codegen();
      params += ", ";
    }
    params = params.substr(0, params.size() - 2);

    return "(" + params + ") -> " + proto->ret.get().velox_codegen();
  }
  case ETypeKind::Facet: {
    const auto* facet = static_cast<const type::Facet*>(this);
    const auto* node  = facet->sym.node().as<ast::SFM_Facet>();
    assert(node && "type node must be a facet");

    std::string fields;
    for (const auto& field : node->fields) {
      const auto* f_node = field.as<ast::SFM_Facet_Field>();
      assert(f_node && "a facet must have field nodes");

      fields += std::string(f_node->name) + ": " + f_node->type.get().velox_codegen() + "\n";
    }

    return "facet" + std::string(node->name) + " {\n" + fields + "}\n";
  }
  case ETypeKind::View: {
    const auto* view = static_cast<const type::View*>(this);
    assert(view->sym && "a view type must refer to a symbol");
  }
  case ETypeKind::Identifier: {
    const auto* id = static_cast<const type::Identifier*>(this);
    return id->forward_name;
  }
  case ETypeKind::Form:
  case ETypeKind::Enum:
  case ETypeKind::Flag:
  case ETypeKind::Union: break;
  }

  return "";
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

  case ETypeKind::StaticArray: {
    auto const& da = static_cast<const type::StaticArray*>(&a);
    auto const& db = static_cast<const type::StaticArray*>(&b);

    return da->inner == db->inner && da->size == db->size;
  }

  case ETypeKind::DynamicArray: {
    return static_cast<const type::DynamicArray*>(&a)->inner == static_cast<const type::DynamicArray*>(&b)->inner;
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
    return static_cast<const type::Enum*>(&a)->sym == static_cast<const type::Enum*>(&b)->sym;
  }

  case ETypeKind::Flag: {
    return static_cast<const type::Flag*>(&a)->sym == static_cast<const type::Flag*>(&b)->sym;
  }

  case ETypeKind::Union: {
    return static_cast<const type::Union*>(&a)->sym == static_cast<const type::Union*>(&b)->sym;
  }

  case ETypeKind::Facet: {
    return static_cast<const type::Facet*>(&a)->sym == static_cast<const type::Facet*>(&b)->sym;
  }

  case ETypeKind::View: {
    return static_cast<const type::View*>(&a)->sym == static_cast<const type::View*>(&b)->sym;
  }

  case ETypeKind::Form: {
    return static_cast<const type::Form*>(&a)->sym == static_cast<const type::Form*>(&b)->sym;
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
  case EPrimitiveTypeKind::_ptr:     return "ptrt";
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
  case EPrimitiveTypeKind::_ptr:
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
  case EPrimitiveTypeKind::_u0:
  case EPrimitiveTypeKind::NONE:     return 0;
  }
}

size_t type::EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type) noexcept
{
  return EPrimitiveTypeKind_to_bits(type) / 8;
}

bool type::is_op_handled(EPrimitiveTypeKind src, ast::EBinOpType op) noexcept
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
  case token::ETokenKind::T_TEXT:     return ETextType::_text;
  case token::ETokenKind::T_STRING:   return ETextType::_str;
  case token::ETokenKind::T_C_STRING: return ETextType::_c_str;
  case token::ETokenKind::T_CUNE:     return ETextType::_cune;
  case token::ETokenKind::T_RUNE:     return ETextType::_rune;
  default:                            return ETextType::NONE;
  }
}


type::ID type::get_prototype(ast::ID nodeid) noexcept
{
  if (const auto* n = nodeid.as<ast::Global_Function>()) return n->prototype;
  if (const auto* n = nodeid.as<ast::Local_Lambda>()) return n->prototype;
  if (const auto* n = nodeid.as<ast::SFM_Rule>()) return n->prototype;

  return NO_ID;
}


void type::initialization() noexcept
{
  primitives.reserve(TYPEID_USER_START);

#define prim(prim_ty)                                                                                                  \
  auto* name##prim_ty         = new type::Primitive();                                                                 \
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
  prim(_ptr);

#undef prim

#define prim_txt(prim_ty)                                                                                              \
  auto* name##prim_ty               = new type::String();                                                              \
  name##prim_ty->kind               = ETextType::prim_ty;                                                              \
  primitives[type::TYPEID##prim_ty] = std::unique_ptr<type::String>(name##prim_ty);

  prim_txt(_c_str);
  prim_txt(_str);
  prim_txt(_text);

#undef prim_txt
}


type::Type& type::Arena::get(ID id) noexcept
{
  size_t offset = id.offset();
  if (offset >= 0 && offset < TYPEID_USER_START)
    return *type::primitives.at(type::ID::make(cu::ID::main(), offset)).get();

  offset -= TYPEID_USER_START;

  assert(offset < types.size());
  return *types[offset];
}
const type::Type& type::Arena::get(ID id) const noexcept
{
  size_t offset = id.offset();
  if (offset >= 0 && offset < TYPEID_USER_START)
    return *type::primitives.at(type::ID::make(cu::ID::main(), offset)).get();

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
TYPE_GET_INSTANCE(type::StaticArray)
TYPE_GET_INSTANCE(type::Ptr)
TYPE_GET_INSTANCE(type::DynamicArray)
TYPE_GET_INSTANCE(type::Prototype)
TYPE_GET_INSTANCE(type::Facet)
TYPE_GET_INSTANCE(type::View)
TYPE_GET_INSTANCE(type::Form)
TYPE_GET_INSTANCE(type::Enum)
TYPE_GET_INSTANCE(type::Flag)
TYPE_GET_INSTANCE(type::Union)
TYPE_GET_INSTANCE(type::Identifier)

#undef TYPE_GET_INSTANCE
