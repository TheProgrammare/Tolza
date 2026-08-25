#pragma once

#include <string>

#include <common/commands.hpp>


namespace CLI
{
class App;
}

namespace toolchain
{

class Commander final : common::Commander
{
public:
  Commander() = delete;
  Commander(CLI::App& _app)
    : common::Commander(_app)
  {
    init_commands();
  }

private:
  std::string name;
  std::string regex_name;
  bool        installed  = false;
  bool        upgradable = false;
  bool        full       = false;
  bool        force      = false;

  void init_commands() noexcept;

  void init_command_package() noexcept;
  void init_command_toolchain() noexcept;
  void init_command_new() noexcept;
  void init_command_check() noexcept;
  void init_command_build() noexcept override;

  void exec_ffi_command() noexcept override;
};

} // namespace toolchain
