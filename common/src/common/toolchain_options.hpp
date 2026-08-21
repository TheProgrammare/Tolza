#pragma once

#include <string>
#include <string_view>


namespace common::toolchain
{

void init_toolchain_context() noexcept;


struct Options {
  bool is_valid = false;

  std::string custom_compiler_dir;
  std::string compiler_used;

  [[nodiscard]] static Options read_config(std::string_view path) noexcept;
  void                         write_config(std::string_view path) noexcept;

  static void apply_compiler(std::string_view file) noexcept;
  static void cogito_compiler(std::string_view file) noexcept;
};

inline Options OPTIONS;

constexpr std::string_view TOOLCHAIN_CONFIG =
    R"(
# tolza-toolchain config file

# DO NOT remove or add any section nor field

[compiler]
custom_compiler_dir     = "{}"
compiler_used           = "{}"

)";
} // namespace common::toolchain