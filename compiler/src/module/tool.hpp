#pragma once


#include "nexus/forward.hpp"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace module
{


// will mangle his canonical name
// start module -> parent modules -> file module
// e.g. file_module_name.parent_modules_names.start_module
[[nodiscard]] std::string mangle_canonical_module_path(ID p_start_module) noexcept;


// will generates / returns compilation units and modules for each path segment
// if one script doesn't exists, a runtime error will return unexpected
[[nodiscard]] ID build_module_from_path(cu::ID parent_cuid, const std::vector<std::string>& p_path,
                                        cu::EFileSource p_file_source, std::string& str_err) noexcept;


[[nodiscard]] ID resolve_anchor(ID ctx, ast::EPathAnchor anchor) noexcept;
[[nodiscard]] ID resolve_from_children(ID ctx, const std::vector<std::string>& path, size_t index) noexcept;
[[nodiscard]] ID resolve_from_import(ID ctx, const std::vector<std::string>& path, size_t index) noexcept;
[[nodiscard]] ID resolve_from_reexport(ID ctx, const std::vector<std::string>& path, size_t index) noexcept;
[[nodiscard]] std::pair<ID, definition::ID> resolve_definition_from_children(ID                              ctx,
                                                                             const std::vector<std::string>& path,
                                                                             size_t                          index,
                                                                             const std::string& sym) noexcept;
[[nodiscard]] std::pair<ID, definition::ID> resolve_definition_from_import(ID ctx, const std::vector<std::string>& path,
                                                                           size_t             index,
                                                                           const std::string& sym) noexcept;
[[nodiscard]] std::pair<ID, definition::ID> resolve_definition_from_reexport(ID                              ctx,
                                                                             const std::vector<std::string>& path,
                                                                             size_t                          index,
                                                                             const std::string& sym) noexcept;
[[nodiscard]] ID resolve_module_path(ID ctx, const std::vector<std::string>& path, ast::EPathAnchor anchor) noexcept;
[[nodiscard]] definition::ID resolve_path_symbol(ID ctx, const std::vector<std::string>& path, ast::EPathAnchor anchor,
                                                 const std::string& sym_name) noexcept;

[[nodiscard]] std::vector<module::ID>& get_children(ID id) noexcept;

} // namespace module
