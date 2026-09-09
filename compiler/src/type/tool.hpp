#pragma once


#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "type/type.hpp"

#include <vector>

namespace type
{

[[nodiscard]] bool is_same_type(type::Primitive const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Ptr const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Array const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Buffer const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Slice const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Tuple const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Prototype const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Enum const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Flag const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Union const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Facet const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Form const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::View const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::Identifier const& t, ID tyid) noexcept;
[[nodiscard]] bool is_same_type(type::String const& t, ID tyid) noexcept;

ID get_prototype(ast::ID nodeid) noexcept;
ID get_inner(type::ID tyid) noexcept;
// ID   get_key(ast::ID nodeid) noexcept;
// ID   get_val(ast::ID nodeid) noexcept;


[[nodiscard]] std::vector<Prototype_Param> to_proto_params(const std::vector<ast::ID>& params) noexcept;

} // namespace type