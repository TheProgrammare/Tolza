#pragma once


#include <set>
#include <string_view>


namespace common::fileutils
{


[[nodiscard]] bool is_sub_path(std::string_view base, std::string_view path) noexcept;

[[nodiscard]] std::string resolve_path(std::string_view current, std::string_view relative = "") noexcept;

constexpr std::string_view TOLZA_FILE_EXTENSION = ".tlz";

// check if extension is "tlz", "tlzbind", "tlzlib"
[[nodiscard]] bool                  is_tolza_extension(std::string_view extension) noexcept;
// check if file exists then has a tolza extension (or attribute .tlz for no extension)
[[nodiscard]] bool                  is_tolza_file(std::string_view file_path) noexcept;
[[nodiscard]] std::string           get_tolza_file(std::string_view path) noexcept;
[[nodiscard]] std::set<std::string> find_tolza_files(std::string_view target_dir, bool is_recursive) noexcept;
[[nodiscard]] std::string           find_tolza_toml(std::string_view file_path) noexcept;

[[nodiscard]] bool is_barrel_file(std::string_view file_path) noexcept;
[[nodiscard]] bool is_barrel_usercode(std::string_view file_path) noexcept;

void write_barrel(std::string_view target_dir, std::string_view common_alias) noexcept;


// %vc_version
// %date
constexpr std::string_view BARREL_FILE_HEADER =
    R"(# barrel

/*
 * =============================================================================
 *  Tolza - Generated Barrel File
 * =============================================================================
 *
 * Tolza Version           : {0}
 * Generated on            : {1}
 * 
 * -----------------------------------------------------------------------------
 * WARNING: This file is auto-generated.
 * Do not edit manually.
 * -----------------------------------------------------------------------------
 */

)";


} // namespace common::fileutils