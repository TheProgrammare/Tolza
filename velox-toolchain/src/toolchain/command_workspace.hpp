#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace command
{
namespace workspace
{
void generate_velox_workspace(const std::string& project_name, const fs::path& path);
bool write_file(const fs::path& path, const std::string& text);
bool write_config_file(const fs::path& path, const std::string& name, bool file_debug_mode);
void ask_new_workspace(const fs::path& ws_path);
bool new_velox_workspace();

// %0 project_name
// %1 abi
// %2 arch
// %3 bits
// %4 os
// %5 debug
inline constexpr const char* VELOX_CONFIG_TEMPLATE =
    R"(
# main velox toolchain config
# it's the default configuration
# set config field to specify a sub configuration to compile (use his name in sub_configs)

[target]
project_name = "%0"
abi   = "%1"
arch  = "%2"
bits  = %3
os    = "%4"
libc  = ""
# self is for default configuration
config = self

[profile]
debug           = %5
opt_level       = 0
size_opt        = false

[logs]
# override all log options
all             = %5
filesystem      = %5
lexer           = %5
preprocessor    = %5
parser          = %5
binder          = %5
exporter        = %5
resolver        = %5
LLVM_IR         = %5
Linker          = %5

[defines]
VERSION = "1.0"

[undefines]
EXAMPLE

[codegen]
# LLVM | OBJ | ASM | BC | BIN
emit_mode  = "BIN"
build_dir = "./build"

[project]
project_dir = "./"
source_dir  = "./src"
thrid_party_dir = "./thirdparty"
ffi_json_dir = "./ffi-json"

[sub_configs]
debug = "./config/debug.config"
)";

inline constexpr const char* VELOX_MAIN_TEMPLATE =
    R"(
import usr: core

fn main() {
  core::println("hello world!")
}

)";

inline constexpr const char* VELOX_CORE_TEMPLATE =
    R"(
import ext: C::stdio
import usr: ffi::C
    
export {
  fn println(ref msg: str) {
    C::printf(ffi::C::str_to_cstr(msg))
  }
}

)";

} // namespace workspace
} // namespace command