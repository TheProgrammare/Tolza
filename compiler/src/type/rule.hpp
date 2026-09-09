#pragma once

#include "id/typeid.hpp"
#include "nexus/forward.hpp"
#include "type/forward.hpp"

#include <vector>

namespace type::rule
{

[[nodiscard]] bool commutative_enum_union_castable(const type::Enum& _enum, const type::Union& _union) noexcept;
[[nodiscard]] bool is_facet_subset(const std::vector<type::ID>& base, const std::vector<type::ID>& subset) noexcept;
[[nodiscard]] bool can_explicit_cast(type::ID from, type::ID to) noexcept;
[[nodiscard]] bool can_implicit_cast(type::ID from, type::ID to) noexcept;
[[nodiscard]] bool can_op_primitive(type::EPrimitiveTypeKind term, ast::EOp_Bin op) noexcept;

} // namespace type::rule