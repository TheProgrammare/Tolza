#include "type/tool.hpp"

#include "ast/node/base.hpp"
#include "ast/node/declaration_extension.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/generic.hpp"
#include "ast/node/literal.hpp"
#include "ast/node/numeric_128_bits.hpp"
#include "ast/node/operation.hpp"
#include "ast/node/statement.hpp"
#include "common/compiler_options.hpp"
#include "compiler/compiler.hpp"
#include "id/base.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "lexer/data.hpp"
#include "nexus/forward.hpp"
#include "type/data.hpp"
#include "type/type.hpp"

#include <cassert>
#include <cstddef>
#include <string_view>
#include <vector>


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
  return t.header.tyid == tyid && t.nodeid == d->nodeid;
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
  case EPrimitiveTypeKind::_bsize:   return OPTIONS.target.get_arch_size();
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
