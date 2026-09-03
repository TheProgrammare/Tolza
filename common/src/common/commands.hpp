#pragma once

#include <string>
#include <vector>

namespace common::compiler
{
struct Profile;
}

namespace CLI
{
class App;
}

namespace common
{

class Commander
{
private:
  void init_common_commands() noexcept;

public:
  Commander() = delete;
  Commander(CLI::App& _app, int argc, const char* argv[]);


  common::compiler::Profile& opt;

  std::vector<std::string> args;

protected:
  CLI::App& app;

  std::string compiler_path;
  std::string from_path = ".";
  std::string to_path;
  bool        ffi_json_flag = false;
  bool        ffi_c_flag    = false;
  bool        compiler_all  = false;
  std::string compiler_version;
  bool        compiler_latest = false;
  bool        is_release      = true;
  bool        is_debug        = false;
  bool        no_env          = false;

  void compilation_args(CLI::App* build) noexcept;

  void         init_command_compiler() noexcept;
  virtual void init_command_build() noexcept = 0;
  void         init_command_ffi() noexcept;

  virtual void exec_ffi_command() noexcept = 0;
};

} // namespace common
