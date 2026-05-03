#include "type.hpp"

#include "compiler/compiler.hpp"
#include "compiler_options.hpp"
#include "nexus/forward.hpp"
#include "nexus/symbol.hpp"
#include "nexus/lexer/token.hpp"
#include <cassert>
#include <variant>

type::Arena::Arena()
{
  types.reserve(37 * 2);

  // BAD_TYPE_ID = 0
  factory.make_primitive(EPrimitiveTypeKind::u0); // assume invalid
  factory.make_primitive(EPrimitiveTypeKind::u0);
  factory.make_primitive(EPrimitiveTypeKind::boolean);
  factory.make_primitive(EPrimitiveTypeKind::i8);
  factory.make_primitive(EPrimitiveTypeKind::i16);
  factory.make_primitive(EPrimitiveTypeKind::i32);
  factory.make_primitive(EPrimitiveTypeKind::i64);
  factory.make_primitive(EPrimitiveTypeKind::i128);
  factory.make_primitive(EPrimitiveTypeKind::iSize);
  factory.make_primitive(EPrimitiveTypeKind::u8);
  factory.make_primitive(EPrimitiveTypeKind::u16);
  factory.make_primitive(EPrimitiveTypeKind::u32);
  factory.make_primitive(EPrimitiveTypeKind::u64);
  factory.make_primitive(EPrimitiveTypeKind::u128);
  factory.make_primitive(EPrimitiveTypeKind::uSize);
  factory.make_primitive(EPrimitiveTypeKind::b8);
  factory.make_primitive(EPrimitiveTypeKind::b16);
  factory.make_primitive(EPrimitiveTypeKind::b32);
  factory.make_primitive(EPrimitiveTypeKind::b64);
  factory.make_primitive(EPrimitiveTypeKind::b128);
  factory.make_primitive(EPrimitiveTypeKind::bSize);
  factory.make_primitive(EPrimitiveTypeKind::f16);
  factory.make_primitive(EPrimitiveTypeKind::f32);
  factory.make_primitive(EPrimitiveTypeKind::f64);
  factory.make_primitive(EPrimitiveTypeKind::f80);
  factory.make_primitive(EPrimitiveTypeKind::f128);
  factory.make_primitive(EPrimitiveTypeKind::fSize);
  factory.make_primitive(EPrimitiveTypeKind::d32);
  factory.make_primitive(EPrimitiveTypeKind::d64);
  factory.make_primitive(EPrimitiveTypeKind::d128);
  factory.make_primitive(EPrimitiveTypeKind::dSize);
  factory.make_primitive(EPrimitiveTypeKind::ud32);
  factory.make_primitive(EPrimitiveTypeKind::ud64);
  factory.make_primitive(EPrimitiveTypeKind::ud128);
  factory.make_primitive(EPrimitiveTypeKind::udSize);
  factory.make_primitive(EPrimitiveTypeKind::ptrdiff);
  factory.make_primitive(EPrimitiveTypeKind::cune);
  factory.make_primitive(EPrimitiveTypeKind::rune);
  factory.make_string(ETextType::c_str);
  factory.make_string(ETextType::str);
  factory.make_string(ETextType::text);
}


type::_id type::Arena::Factory::make_primitive(EPrimitiveTypeKind p, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Primitive;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::PrimitiveData>();
  d.primitive = p;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_ptr(type::_id inner, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Primitive;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::PtrData>();
  d.inner     = inner;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_static_array(type::_id inner, size_t size, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::StaticArray;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::StaticArrayData>();
  d.inner     = inner;
  d.size      = size;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_dynamic_array(type::_id inner, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::DynamicArray;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::DynamicArrayData>();
  d.inner     = inner;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_tuple(std::vector<type::_id> elems, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Tuple;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::TupleData>();
  d.elems     = elems;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_prototype(std::vector<type::_id> params, type::_id ret, bool is_variadic,
                                               symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind        = ETypeKind::Prototype;
  t.decorator   = decorator;
  auto& d       = t.data.emplace<Type::PrototypeData>();
  d.params      = params;
  d.ret         = ret;
  d.is_variadic = is_variadic;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_enum(std::vector<std::vector<type::_id>> variants, symbol::_id sym,
                                          Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Enum;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::EnumData>();
  d.variants  = variants;
  d.sym       = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_flag(size_t size, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Flag;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::FlagData>();
  d.size      = size;
  d.sym       = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_union(std::vector<type::_id> variants, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Union;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::UnionData>();
  d.variants  = variants;
  d.sym       = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_component(std::vector<type::_id> fields, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Component;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::ComponentData>();
  d.fields    = fields;
  d.sym       = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_role(std::vector<type::_id> components, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind       = ETypeKind::Role;
  t.decorator  = decorator;
  auto& d      = t.data.emplace<Type::RoleData>();
  d.components = components;
  d.sym        = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_entity(std::vector<type::_id> components, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind       = ETypeKind::Entity;
  t.decorator  = decorator;
  auto& d      = t.data.emplace<Type::EntityData>();
  d.components = components;
  d.sym        = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_identifier(ast::_gnid gnid, symbol::_id sym, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::Identifier;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::IdentifierData>();
  d.gnid      = gnid;
  d.sym       = sym;
  return arena.intern(std::move(t));
}
type::_id type::Arena::Factory::make_string(ETextType txt_ty, Decorator decorator)
{
  Type t;
  t.kind      = ETypeKind::String;
  t.decorator = decorator;
  auto& d     = t.data.emplace<Type::StringData>();
  d.kind      = txt_ty;
  return arena.intern(std::move(t));
}


type::_id type::Type::IdentifierData::get_type() const
{
  auto& ty = compiler::COMPILER.symbols.get(sym);
  return ty.type;
}


size_t type::Arena::hash_type(Type const& t) const
{
  size_t h = hash_val(static_cast<uint8_t>(t.kind));

  switch (t.kind) {
  case ETypeKind::Primitive: {
    auto const& d = std::get<Type::PrimitiveData>(t.data);
    h             = hash_combine(h, static_cast<size_t>(d.primitive));
    break;
  }

  case ETypeKind::Ptr: {
    auto const& d = std::get<Type::PtrData>(t.data);
    h             = hash_combine(h, d.inner.value());
    break;
  }

  case ETypeKind::StaticArray: {
    auto const& d = std::get<Type::StaticArrayData>(t.data);
    h             = hash_combine(h, d.inner.value());
    h             = hash_combine(h, d.size);
    break;
  }

  case ETypeKind::DynamicArray: {
    auto const& d = std::get<Type::DynamicArrayData>(t.data);
    h             = hash_combine(h, d.inner.value());
    break;
  }

  case ETypeKind::Tuple: {
    auto const& d = std::get<Type::TupleData>(t.data);
    for (auto const& e : d.elems) h = hash_combine(h, e.value());
    break;
  }

  case ETypeKind::Prototype: {
    auto const& d = std::get<Type::PrototypeData>(t.data);

    for (auto const& p : d.params) h = hash_combine(h, p.value());

    h = hash_combine(h, d.ret.value());
    h = hash_combine(h, d.is_variadic);
    break;
  }

  case ETypeKind::Enum: {
    auto const& d = std::get<Type::EnumData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Flag: {
    auto const& d = std::get<Type::FlagData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Union: {
    auto const& d = std::get<Type::UnionData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Component: {
    auto const& d = std::get<Type::ComponentData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Entity: {
    auto const& d = std::get<Type::EntityData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Role: {
    auto const& d = std::get<Type::RoleData>(t.data);
    h             = hash_combine(h, d.sym.value());
    break;
  }

  case ETypeKind::Identifier: {
    auto const& d = std::get<Type::IdentifierData>(t.data);
    h             = hash_combine(h, d.gnid.value());
    break;
  }

  case ETypeKind::String: {
    auto const& d = std::get<Type::StringData>(t.data);
    h             = hash_combine(h, static_cast<uint8_t>(d.kind));
    break;
  }

  case ETypeKind::NONE: {
    assert(false);
  }
  }

  return h;
}

symbol::_id type::Type::get_sym_id() const
{
  switch (kind) {
  case ETypeKind::Component:  return std::get<type::Type::ComponentData>(data).sym;
  case ETypeKind::Entity:     return std::get<type::Type::EntityData>(data).sym;
  case ETypeKind::Enum:       return std::get<type::Type::EnumData>(data).sym;
  case ETypeKind::Flag:       return std::get<type::Type::FlagData>(data).sym;
  case ETypeKind::Union:      return std::get<type::Type::UnionData>(data).sym;
  case ETypeKind::Identifier: return std::get<type::Type::IdentifierData>(data).sym;
  default:                    return NO_ID;
  }
}

bool type::TypeEq::operator()(Type const& a, Type const& b) const noexcept
{
  if (a.kind != b.kind) return false;

  switch (a.kind) {
  case ETypeKind::Primitive: {
    return std::get<Type::PrimitiveData>(a.data).primitive == std::get<Type::PrimitiveData>(b.data).primitive;
  }

  case ETypeKind::Ptr: {
    return std::get<Type::PtrData>(a.data).inner == std::get<Type::PtrData>(b.data).inner;
  }

  case ETypeKind::StaticArray: {
    auto const& da = std::get<Type::StaticArrayData>(a.data);
    auto const& db = std::get<Type::StaticArrayData>(b.data);

    return da.inner == db.inner && da.size == db.size;
  }

  case ETypeKind::DynamicArray: {
    return std::get<Type::DynamicArrayData>(a.data).inner == std::get<Type::DynamicArrayData>(b.data).inner;
  }

  case ETypeKind::Tuple: {
    return std::get<Type::TupleData>(a.data).elems == std::get<Type::TupleData>(b.data).elems;
  }

  case ETypeKind::Prototype: {
    auto const& da = std::get<Type::PrototypeData>(a.data);
    auto const& db = std::get<Type::PrototypeData>(b.data);

    return da.params == db.params && da.ret == db.ret;
  }

  case ETypeKind::Enum: {
    return std::get<Type::EnumData>(a.data).sym == std::get<Type::EnumData>(b.data).sym;
  }

  case ETypeKind::Flag: {
    return std::get<Type::FlagData>(a.data).sym == std::get<Type::FlagData>(b.data).sym;
  }

  case ETypeKind::Union: {
    return std::get<Type::UnionData>(a.data).sym == std::get<Type::UnionData>(b.data).sym;
  }

  case ETypeKind::Component: {
    return std::get<Type::ComponentData>(a.data).sym == std::get<Type::ComponentData>(b.data).sym;
  }

  case ETypeKind::Role: {
    return std::get<Type::RoleData>(a.data).sym == std::get<Type::RoleData>(b.data).sym;
  }

  case ETypeKind::Entity: {
    return std::get<Type::EntityData>(a.data).sym == std::get<Type::EntityData>(b.data).sym;
  }

  case ETypeKind::Identifier: {
    return std::get<Type::IdentifierData>(a.data).gnid == std::get<Type::IdentifierData>(b.data).gnid;
  }

  case ETypeKind::NONE: {
    assert(false);
  }
  }

  return false;
}


std::string_view type::EPrimitiveTypeKind_to_str(EPrimitiveTypeKind type)
{
  switch (type) {
  case EPrimitiveTypeKind::boolean: return "boolean";
  case EPrimitiveTypeKind::cune:    return "cunei";
  case EPrimitiveTypeKind::rune:    return "rune";

  case EPrimitiveTypeKind::ptrdiff: return "ptrdiff";

  case EPrimitiveTypeKind::iSize:   return "iSize";
  case EPrimitiveTypeKind::i8:      return "i8";
  case EPrimitiveTypeKind::i16:     return "i16";
  case EPrimitiveTypeKind::i32:     return "i32";
  case EPrimitiveTypeKind::i64:     return "i64";
  case EPrimitiveTypeKind::i128:    return "i128";

  case EPrimitiveTypeKind::uSize:   return "uSize";
  case EPrimitiveTypeKind::u8:      return "u8";
  case EPrimitiveTypeKind::u16:     return "u16";
  case EPrimitiveTypeKind::u32:     return "u32";
  case EPrimitiveTypeKind::u64:     return "u64";
  case EPrimitiveTypeKind::u128:    return "u128";

  case EPrimitiveTypeKind::bSize:   return "bSize";
  case EPrimitiveTypeKind::b8:      return "b8";
  case EPrimitiveTypeKind::b16:     return "b16";
  case EPrimitiveTypeKind::b32:     return "b32";
  case EPrimitiveTypeKind::b64:     return "b64";
  case EPrimitiveTypeKind::b128:    return "b128";

  case EPrimitiveTypeKind::fSize:   return "fSize";
  case EPrimitiveTypeKind::f16:     return "f6";
  case EPrimitiveTypeKind::f32:     return "f32";
  case EPrimitiveTypeKind::f64:     return "f64";
  case EPrimitiveTypeKind::f80:     return "f80";
  case EPrimitiveTypeKind::f128:    return "f128";

  case EPrimitiveTypeKind::dSize:   return "dSize";
  case EPrimitiveTypeKind::d32:     return "d32";
  case EPrimitiveTypeKind::d64:     return "d64";
  case EPrimitiveTypeKind::d128:    return "d128";

  case EPrimitiveTypeKind::udSize:  return "udSize";
  case EPrimitiveTypeKind::ud32:    return "ud32";
  case EPrimitiveTypeKind::ud64:    return "ud64";
  case EPrimitiveTypeKind::ud128:   return "ud128";

  case EPrimitiveTypeKind::u0:      return "Void";

  case EPrimitiveTypeKind::NONE:    return "NO PRIMITIVE TYPE";
  case EPrimitiveTypeKind::COUNT:   return "NO PRIMITIVE TYPE";
  }
}

std::string_view type::EPrimitiveTypeKind_to_mangle(EPrimitiveTypeKind type)
{
  switch (type) {
  case EPrimitiveTypeKind::boolean: return "b";
  case EPrimitiveTypeKind::cune:    return "aii";
  case EPrimitiveTypeKind::rune:    return "utf";

  case EPrimitiveTypeKind::u0:      return "u0";
  case EPrimitiveTypeKind::ptrdiff: return "pdif";

  case EPrimitiveTypeKind::iSize:   return "isz";
  case EPrimitiveTypeKind::i8:      return "i8";
  case EPrimitiveTypeKind::i16:     return "i16";
  case EPrimitiveTypeKind::i32:     return "i32";
  case EPrimitiveTypeKind::i64:     return "i64";
  case EPrimitiveTypeKind::i128:    return "i128";

  case EPrimitiveTypeKind::uSize:   return "usz";
  case EPrimitiveTypeKind::u8:      return "u8";
  case EPrimitiveTypeKind::u16:     return "u16";
  case EPrimitiveTypeKind::u32:     return "u32";
  case EPrimitiveTypeKind::u64:     return "u64";
  case EPrimitiveTypeKind::u128:    return "u128";

  case EPrimitiveTypeKind::bSize:   return "bsz";
  case EPrimitiveTypeKind::b8:      return "b8";
  case EPrimitiveTypeKind::b16:     return "b16";
  case EPrimitiveTypeKind::b32:     return "b32";
  case EPrimitiveTypeKind::b64:     return "b64";
  case EPrimitiveTypeKind::b128:    return "b128";

  case EPrimitiveTypeKind::fSize:   return "fsz";
  case EPrimitiveTypeKind::f16:     return "f16";
  case EPrimitiveTypeKind::f32:     return "f32";
  case EPrimitiveTypeKind::f64:     return "f64";
  case EPrimitiveTypeKind::f80:     return "f80";
  case EPrimitiveTypeKind::f128:    return "f128";

  case EPrimitiveTypeKind::dSize:   return "dSize";
  case EPrimitiveTypeKind::d32:     return "d32";
  case EPrimitiveTypeKind::d64:     return "d64";
  case EPrimitiveTypeKind::d128:    return "d128";

  case EPrimitiveTypeKind::udSize:  return "udSize";
  case EPrimitiveTypeKind::ud32:    return "ud32";
  case EPrimitiveTypeKind::ud64:    return "ud64";
  case EPrimitiveTypeKind::ud128:   return "ud128";

  case EPrimitiveTypeKind::NONE:    return "NO PRIMITIVE TYPE";
  case EPrimitiveTypeKind::COUNT:   return "NO PRIMITIVE TYPE";
  }
}

type::EPrimitiveTypeKind type::ETokenKind_to_EPrimitiveTypeKind(token::ETokenKind tok)
{
  switch (tok) {
  case token::ETokenKind::T_BOOL:  return EPrimitiveTypeKind::boolean;
  case token::ETokenKind::T_RUNE:  return EPrimitiveTypeKind::rune;
  case token::ETokenKind::T_CUNE:  return EPrimitiveTypeKind::cune;

  case token::ETokenKind::T_ISIZE: return EPrimitiveTypeKind::iSize;
  case token::ETokenKind::T_I8:    return EPrimitiveTypeKind::i8;
  case token::ETokenKind::T_I16:   return EPrimitiveTypeKind::i16;
  case token::ETokenKind::T_I32:   return EPrimitiveTypeKind::i32;
  case token::ETokenKind::T_I64:   return EPrimitiveTypeKind::i64;
  case token::ETokenKind::T_I128:  return EPrimitiveTypeKind::i128;

  case token::ETokenKind::T_USIZE: return EPrimitiveTypeKind::uSize;
  case token::ETokenKind::T_U8:    return EPrimitiveTypeKind::u8;
  case token::ETokenKind::T_U16:   return EPrimitiveTypeKind::u16;
  case token::ETokenKind::T_U32:   return EPrimitiveTypeKind::u32;
  case token::ETokenKind::T_U64:   return EPrimitiveTypeKind::u64;
  case token::ETokenKind::T_U128:  return EPrimitiveTypeKind::u128;

  case token::ETokenKind::T_BSIZE: return EPrimitiveTypeKind::bSize;
  case token::ETokenKind::T_B8:    return EPrimitiveTypeKind::b8;
  case token::ETokenKind::T_B16:   return EPrimitiveTypeKind::b16;
  case token::ETokenKind::T_B32:   return EPrimitiveTypeKind::b32;
  case token::ETokenKind::T_B64:   return EPrimitiveTypeKind::b64;
  case token::ETokenKind::T_B128:  return EPrimitiveTypeKind::b128;

  case token::ETokenKind::T_FSIZE: return EPrimitiveTypeKind::fSize;
  case token::ETokenKind::T_F16:   return EPrimitiveTypeKind::f16;
  case token::ETokenKind::T_F32:   return EPrimitiveTypeKind::f32;
  case token::ETokenKind::T_F64:   return EPrimitiveTypeKind::f64;
  case token::ETokenKind::T_F80:   return EPrimitiveTypeKind::f80;
  case token::ETokenKind::T_F128:  return EPrimitiveTypeKind::f128;

  case token::ETokenKind::T_U0:    return EPrimitiveTypeKind::u0;

  default:                         return EPrimitiveTypeKind::NONE;
  }
}

std::string_view type::EPtrType_to_str(EPtrType type)
{
  switch (type) {
  case EPtrType::raw_ptr:    return "ptr";
  case EPtrType::unique_ptr: return "uptr";
  case EPtrType::shared_ptr: return "sptr";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}

std::string_view type::EPtrType_to_mangle(EPtrType type)
{
  switch (type) {
  case EPtrType::raw_ptr:    return "p";
  case EPtrType::unique_ptr: return "up";
  case EPtrType::shared_ptr: return "sp";
  case EPtrType::NONE:       return "NO POINTER TYPE";
  }
}


size_t type::EPrimitiveTypeKind_to_bits(EPrimitiveTypeKind type)
{
  switch (type) {
  case EPrimitiveTypeKind::boolean: return 1;
  case EPrimitiveTypeKind::cune:    return 8;
  case EPrimitiveTypeKind::rune:    return 32;
  case EPrimitiveTypeKind::ptrdiff:
  case EPrimitiveTypeKind::fSize:
  case EPrimitiveTypeKind::dSize:
  case EPrimitiveTypeKind::udSize:
  case EPrimitiveTypeKind::iSize:
  case EPrimitiveTypeKind::uSize:
  case EPrimitiveTypeKind::bSize:   return compiler::COMPILER_OPTIONS.get_arch_size();
  case EPrimitiveTypeKind::i8:
  case EPrimitiveTypeKind::u8:
  case EPrimitiveTypeKind::b8:      return 8;
  case EPrimitiveTypeKind::f16:
  case EPrimitiveTypeKind::i16:
  case EPrimitiveTypeKind::u16:
  case EPrimitiveTypeKind::b16:     return 16;
  case EPrimitiveTypeKind::f32:
  case EPrimitiveTypeKind::d32:
  case EPrimitiveTypeKind::ud32:
  case EPrimitiveTypeKind::i32:
  case EPrimitiveTypeKind::u32:
  case EPrimitiveTypeKind::b32:     return 32;
  case EPrimitiveTypeKind::f64:
  case EPrimitiveTypeKind::d64:
  case EPrimitiveTypeKind::ud64:
  case EPrimitiveTypeKind::i64:
  case EPrimitiveTypeKind::u64:
  case EPrimitiveTypeKind::b64:     return 64;
  case EPrimitiveTypeKind::f128:
  case EPrimitiveTypeKind::d128:
  case EPrimitiveTypeKind::ud128:
  case EPrimitiveTypeKind::i128:
  case EPrimitiveTypeKind::u128:
  case EPrimitiveTypeKind::b128:    return 128;
  case EPrimitiveTypeKind::f80:     return 80;
  case EPrimitiveTypeKind::u0:
  case EPrimitiveTypeKind::NONE:
  case EPrimitiveTypeKind::COUNT:   return 0;
  }
}

size_t type::EPrimitiveTypeKind_to_bytes(EPrimitiveTypeKind type)
{
  return EPrimitiveTypeKind_to_bits(type) / 8;
}

bool type::is_op_handled(EPrimitiveTypeKind src, ast::EBinOpType op)
{
  // return
  // op_primitive[static_cast<uint8_t>(op)][static_cast<uint8_t>(src)];
  return false;
}

bool type::is_cast_explicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target)
{
  // return
  // cast_explicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}

bool type::is_cast_implicit(EPrimitiveTypeKind src, EPrimitiveTypeKind target)
{
  // return
  // cast_implicit[static_cast<uint8_t>(src)][static_cast<uint8_t>(target)];
  return false;
}
