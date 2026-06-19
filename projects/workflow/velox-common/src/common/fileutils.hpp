#pragma once


#include <set>
#include <string_view>


namespace common::fileutils
{


[[nodiscard]] bool is_sub_path(std::string_view base, std::string_view path) noexcept;

[[nodiscard]] std::string resolve_path(std::string_view s, std::string_view relative = "") noexcept;

constexpr std::string_view VELOX_FILE_EXTENSION = ".vlx";

// check if extension is "vlx", "vlxbind", "vlxlib"
[[nodiscard]] bool                  is_velox_extension(std::string_view extension) noexcept;
// check if file exists then has a velox extension (or attribute .vlx for no extension)
[[nodiscard]] bool                  is_velox_file(std::string_view file_path) noexcept;
[[nodiscard]] std::string           get_velox_file(std::string_view path) noexcept;
[[nodiscard]] std::set<std::string> find_velox_files(std::string_view target_dir, bool is_recursive) noexcept;

[[nodiscard]] bool is_barrel_file(std::string_view file_path) noexcept;
[[nodiscard]] bool is_barrel_usercode(std::string_view file_path) noexcept;

void write_barrel(std::string_view target_dir, std::string_view common_alias) noexcept;


// %vc_version
// %date
constexpr std::string_view BARREL_FILE_HEADER =
    R"(# barrel

/*
 * =============================================================================
 *  Velox Compiler - Generated Barrel File
 * =============================================================================
 *
 * Velox Compiler Version  : %vc_version
 * Generated on            : %date
 * 
 * -----------------------------------------------------------------------------
 * WARNING: This file is auto-generated.
 * Do not edit manually.
 * -----------------------------------------------------------------------------
 */

)";


} // namespace common::fileutils