#pragma once

#include <string>

namespace CLI
{
class App;
}

class Command
{
public:
  Command() = delete;
  Command(CLI::App& _app)
    : app(_app)
  {
    init_command_toolchain();
  }

private:
  CLI::App& app;

  std::string regex_name;
  std::string name;
  std::string dir_path;
  std::string file_path;
  std::string version;
  bool        all        = false;
  bool        latest     = false;
  bool        installed  = false;
  bool        upgradable = false;
  bool        full       = false;
  bool        force      = false;


  void init_command_toolchain();
  void init_command_package();
  void init_command_compiler();
  void init_command_workspace();
  void init_command_build();
  void init_command_ffi_json();
};