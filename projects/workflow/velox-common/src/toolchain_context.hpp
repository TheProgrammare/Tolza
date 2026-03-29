#pragma once

#include <string>
#include <vector>

namespace command
{
namespace compiler
{

std::vector<std::string> find_all_compilers(const std::string& dir_search);
std::string              find_latest_compiler(const std::string& dir_search);

} // namespace compiler
} // namespace command

namespace common
{

void init_toolchain_context();


struct ToolchainCtx {
  std::string custom_compiler_dir;
  std::string compiler_used;

  void apply_context();
};

inline ToolchainCtx TOOL_CTX;

static const std::string toolchain_config =
    R"(
# velox-toolchain config file

# DO NOT remove or add any section nor field

[compiler]
custom_compiler_dir     = "%custom_compiler_dir"
compiler_used           = "%compiler_used"

)";
} // namespace common