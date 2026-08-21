#include "rule.hpp"

#include <algorithm>

#include "Neargye/magic_enum.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "nexus/extension.hpp"
#include "nexus/type/type.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"

bool type::rule::commutative_enum_union_castable(const type::Enum& _enum, const type::Union& _union) noexcept
{
  if (_enum.variants.size() != _union.variants.size()) return false;

  // order is important for memory layout !
  for (size_t i = 0; i < _enum.variants.size(); i++) {
    const auto& enum_elem  = _enum.variants[i];
    const auto& union_elem = _union.variants[i];

    if (enum_elem != union_elem) return false;
  }

  return true;
}
bool type::rule::is_facet_subset(const std::vector<type::ID>& base, const std::vector<type::ID>& subset) noexcept
{
  if (base.size() < subset.size()) return false;

  // no order needed
  for (auto facet_elem : subset) {
    if (!std::ranges::contains(base, facet_elem)) return false;
  }

  return true;
}

template <type::Generic T, type::Generic U>
std::pair<T*, U*> commutative_cast(type::ID from, type::ID to) noexcept
{
  T* from_ty1 = from.as<T>();
  U* to_ty1   = to.as<U>();
  // change way
  if (!from_ty1 && !to_ty1) {
    U* from_ty2 = from.as<U>();
    T* to_ty2   = to.as<T>();
    return {
        to_ty2,
        from_ty2,
    };
  }

  assert(from_ty1 && to_ty1 && "Invalid commutative type definition, one type is invalid");

  return {from_ty1, to_ty1};
}

template <type::Generic T>
T* commutative_one_cast(type::ID from, type::ID to) noexcept
{
  T* from_ty1 = from.as<T>();
  T* to_ty1   = to.as<T>();

  if (from_ty1) return from_ty1;
  if (to_ty1) return to_ty1;

  assert(false && "Both invalid type definition");
}

bool type::rule::can_explicit_cast(type::ID from, type::ID to) noexcept
{
  if (from == to) return true;

  // void and opaque type concerned
  if (from == type::TYPEID_u0 || to == type::TYPEID_u0) return false;
  if (from == type::TYPEID_opaque || to == type::TYPEID_opaque) return false;

  const auto& ty_from = from.get();
  const auto& ty_to   = to.get();

  auto commutative_types = [&](type::ETypeKind kind1, type::ETypeKind kind2) -> bool {
    return (ty_from.kind == kind1 && ty_to.kind == kind2) || (ty_from.kind == kind2 && ty_to.kind == kind1);
  };

  // ==============================
  // between primitives
  // ==============================
  // can be lossy but it's explicit
  if (commutative_types(type::ETypeKind::Primitive, type::ETypeKind::Primitive)) return true;

  // ==============================
  // between flag and primitive
  // ==============================
  // can be lossy if flag fields > byte size casted
  if (commutative_types(type::ETypeKind::Flag, type::ETypeKind::Primitive)) {
    const auto* prim = commutative_one_cast<type::Primitive>(from, to);
    return type::EPrimitiveTypeKind_to_bytes(prim->primitive);
  }

  // ==============================
  // between enum and union
  // ==============================
  if (commutative_types(type::ETypeKind::Enum, type::ETypeKind::Union)) {
    const auto& [_enum, _union] = commutative_cast<type::Enum, type::Union>(from, to);
    return commutative_enum_union_castable(*_enum, *_union);
  }

  // ==============================
  // between form to view
  // ==============================
  if (ty_from.kind == type::ETypeKind::Form && ty_from.kind == type::ETypeKind::View) {
    const auto* form = from.as<type::Form>();
    const auto* view = to.as<type::View>();
    assert(view);
    assert(form);
    return is_facet_subset(form->facets, view->facets);
  }

  // ==============================
  // between cune buffer to cstr or str
  // ==============================
  if (ty_from.kind == type::ETypeKind::Array && ty_to.kind == type::ETypeKind::String) {
    bool valid_to = false;

    if (const auto* to_ptr = ty_to.tyid.as<type::String>()) {
      valid_to =
          to_ptr->kind == ETextType::_str || to_ptr->kind == ETextType::_cstr || to_ptr->kind == ETextType::_cune;
    }

    if (valid_to) {
      if (const auto* from_ptr = ty_from.tyid.as<type::Array>()) {
        if (const auto* from_inner_ptr = from_ptr->inner.as<type::Primitive>()) {
          valid_to = from_inner_ptr->primitive == EPrimitiveTypeKind::_cune;
        }
      }
    }

    if (valid_to) return true;
  }

  if (ty_from.kind == type::ETypeKind::String && ty_from.kind == type::ETypeKind::String) {
    const auto* from_str = ty_from.tyid.as<type::String>();
    const auto* to_str   = ty_to.tyid.as<type::String>();

    if (from_str->kind == type::ETextType::_str && to_str->kind == type::ETextType::_cstr) return true;
    if (from_str->kind == type::ETextType::_cstr && to_str->kind == type::ETextType::_str) return true;

    return false;
  }

  // ==============================
  // between rune buffer to text
  // ==============================
  if (ty_from.kind == type::ETypeKind::Array && ty_to.kind == type::ETypeKind::String) {
    bool valid_to = false;

    if (const auto* to_ptr = ty_to.tyid.as<type::String>()) {
      valid_to = to_ptr->kind == ETextType::_text || to_ptr->kind == ETextType::_rune;
    }

    if (valid_to) {
      if (const auto* from_ptr = ty_from.tyid.as<type::Array>()) {
        if (const auto* from_inner_ptr = from_ptr->inner.as<type::Primitive>()) {
          valid_to = from_inner_ptr->primitive == EPrimitiveTypeKind::_rune;
        }
      }
    }

    if (valid_to) return true;
  }

  // ==============================
  // between view and facet
  // ==============================
  if (commutative_types(type::ETypeKind::View, type::ETypeKind::Facet)) {
    const auto* view = from.as<type::View>();
    assert(view);
    return std::ranges::contains(view->facets, to);
  }

  // ==============================
  // between form and facet
  // ==============================
  if (commutative_types(type::ETypeKind::Form, type::ETypeKind::Facet)) {
    const auto* form = from.as<type::Form>();
    assert(form);
    return std::ranges::contains(form->facets, to);
  }


  // ==============================
  // by from or to extensions
  // ==============================
  return extension::get_cast(from, to);
}

bool type::rule::can_implicit_cast(type::ID from, type::ID to) noexcept
{
  return type::ETypeKind_is_iterable(from.kind()) && to.kind() == type::ETypeKind::Slice;
}


bool type::rule::can_op_primitive(type::EPrimitiveTypeKind term, ast::EOp_Bin op) noexcept
{
  switch (term) {
  case type::EPrimitiveTypeKind::_bool:    return ast::EOp_Bin_is_boolean(op);
  case type::EPrimitiveTypeKind::_cune:
  case type::EPrimitiveTypeKind::_rune:    return ast::EOp_Bin_is_textual(op);
  case type::EPrimitiveTypeKind::_ssize:
  case type::EPrimitiveTypeKind::_s8:
  case type::EPrimitiveTypeKind::_s16:
  case type::EPrimitiveTypeKind::_s32:
  case type::EPrimitiveTypeKind::_s64:
  case type::EPrimitiveTypeKind::_s128:
  case type::EPrimitiveTypeKind::_ptrdiff:
  case type::EPrimitiveTypeKind::_usize:
  case type::EPrimitiveTypeKind::_u8:
  case type::EPrimitiveTypeKind::_u16:
  case type::EPrimitiveTypeKind::_u32:
  case type::EPrimitiveTypeKind::_u64:
  case type::EPrimitiveTypeKind::_u128:    return ast::EOp_Bin_is_integral(op);
  case type::EPrimitiveTypeKind::_bsize:
  case type::EPrimitiveTypeKind::_b8:
  case type::EPrimitiveTypeKind::_b16:
  case type::EPrimitiveTypeKind::_b32:
  case type::EPrimitiveTypeKind::_b64:
  case type::EPrimitiveTypeKind::_b128:    return ast::EOp_Bin_is_bitwise(op);
  case type::EPrimitiveTypeKind::_dsize:
  case type::EPrimitiveTypeKind::_d32:
  case type::EPrimitiveTypeKind::_d64:
  case type::EPrimitiveTypeKind::_d128:
  case type::EPrimitiveTypeKind::_udsize:
  case type::EPrimitiveTypeKind::_ud32:
  case type::EPrimitiveTypeKind::_ud64:
  case type::EPrimitiveTypeKind::_ud128:
  case type::EPrimitiveTypeKind::_fsize:
  case type::EPrimitiveTypeKind::_f16:
  case type::EPrimitiveTypeKind::_f32:
  case type::EPrimitiveTypeKind::_f64:
  case type::EPrimitiveTypeKind::_f80:
  case type::EPrimitiveTypeKind::_f128:    return ast::EOp_Bin_is_decimal(op);
  case type::EPrimitiveTypeKind::_opaque:  return ast::EOp_Bin_is_memory(op);
  case type::EPrimitiveTypeKind::_u0:
  case type::EPrimitiveTypeKind::NONE:     return false;
  }
}


constexpr auto prim_size = magic_enum::enum_count<type::EPrimitiveTypeKind>();

// type cast oracle
// clang-format off
  constexpr std::array<std::array<bool, prim_size>, prim_size> table = {
	//             NONE	  u0      bool	  cune	  rune	  ssize	  s8	    s16	    s32	    s64	    s128	  usize	  u8	    u16	    u32	    u64	    u128	  bsize	  b8	    b16	    b32	    b64	    b128	  ptrdiff	fsize	  f16     f32	    f64	    f80     f128	  dsize	  d32     d64     d128    udsize  ud32    ud64    ud128   ptr
  /*NONE*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, /*NONE*/
  /*u0*/         false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false, /*u0*/
  /*bool*/       false,	false, 	true,	  false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, /*bool*/
  /*cune*/       false,	false,	false, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, /*cune*/
  /*rune*/       false,	false,	false,	false, 	true, 	true, 	false, 	false,	true,	  false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, /*rune*/
  /*ssize*/      false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*ssize*/
  /*s8*/         false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*s8*/
  /*s16*/        false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*s16*/
  /*s32*/        false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*s32*/
  /*s64*/        false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*s64*/
  /*s128*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*s128*/
  /*usize*/      false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*usize*/
  /*u8*/         false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*u8*/
  /*u16*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*u16*/
  /*u32*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*u32*/
  /*u64*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*u64*/
  /*u128*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*u128*/
  /*bsize*/      false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*bsize*/
  /*b8*/         false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*b8*/
  /*b16*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*b16*/
  /*b32*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*b32*/
  /*b64*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*b64*/
  /*b128*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*b128*/
  /*ptrdiff*/    false,	false,	false,	false,	false,	false, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*ptrdiff*/
  /*fsize*/      false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*fsize*/
  /*f16*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true,	  true,	  false,	false,	false,	false,	false,	false,	false,  false,	false, /*f16*/
  /*f32*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true, 	true, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false, /*f32*/
  /*f64*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false, /*f64*/
  /*f80*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false, 	false, 	true,	  true,		false,	false,	false,	false,	false,	false,	false,  false,	false, /*f80*/
  /*f128*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,  true,		false,	false,	false,	false,	false,	false,	false,  false,	false, /*f128*/
  /*dsize*/      false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	false,	false,	false,	false,	false,  false,	false, /*dsize*/
  /*d32*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false, 	true,	  true,	  false,	false,	false,	false,	false,  false,	false, /*d32*/
  /*d64*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true, 	false,	false,	false,	false,  false,	false, /*d64*/
  /*d128*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true, 	true, 	false,	false,	false,  false,	false, /*d128*/
  /*usSize*/     false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	true, 	true, 	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  false,	false,  false,	false, /*usSize*/
  /*ud32*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	true, 	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  false,  false,	false, /*ud32*/
  /*ud64*/       false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	true, 	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true,   false,	false, /*ud64*/
  /*ud128*/      false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true, 	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	true,	  true,	  true,   true,	  false, /*ud128*/
  /*ptr*/        false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,	false,  false,	false, /*ptr*/
	//             NONE	  u0      bool	  cune	  rune	  ssize	  s8	    s16	    s32	    s64	    s128	  usize	  u8	    u16	    u32	    u64	    u128	  bsize	  b8	    b16	    b32	    b64	    b128	  ptrdiff	fsize	  f16     f32	    f64	    f80     f128	  dsize	  d32     d64     d128    udsize  ud32    ud64    ud128   ptr
  };
// clang-format on