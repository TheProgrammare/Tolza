#pragma once


#include "id/nodeid.hpp"

#include <string>

namespace ast
{

[[nodiscard]] std::string get_decl_name(ID nodeid) noexcept;
[[nodiscard]] std::string get_mangled_id(ID id) noexcept;
[[nodiscard]] std::string debug_node_on_line(ID id) noexcept;
[[nodiscard]] std::string dump_debug(ID id) noexcept;
[[nodiscard]] std::string dump(ID id) noexcept;

} // namespace ast