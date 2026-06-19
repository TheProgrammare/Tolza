#pragma once

#include <string>
#include <string_view>

namespace common::compiler
{
struct Compiler_Options;
}

namespace command::workspace
{

[[nodiscard]] std::string generate_velox_workspace(std::string_view project_name, std::string_view path,
                                                   bool force = false) noexcept;
[[nodiscard]] std::string write_file(std::string_view path, std::string_view text, bool verbose = true) noexcept;
void                      ask_new_workspace(std::string_view ws_path, std::string_view name) noexcept;
// will generate all necessary barrels, indexing subdir scripts inside,
// moving any user script with the same barrel name into subdir/mod.vlx or disable the user script
void                      synchronize(std::string_view path) noexcept;
[[nodiscard]] std::string new_velox_workspace() noexcept;

} // namespace command::workspace