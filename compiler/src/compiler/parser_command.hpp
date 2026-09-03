/*
 *	The Tolza programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*
 * This program include and use the CLIUtils/CLI11 project
 * You can find this project at
 *
 *     https://github.com/CLIUtils/CLI11
 *
 * Used for the command parser.
 */

#pragma once

#include <common/commands.hpp>

namespace CLI
{
class App;
}

namespace common::compiler
{
struct Profile;
}

namespace compiler
{

class Commander : common::Commander
{
public:
  Commander(CLI::App& _app, int argc, const char* argv[]);


private:
  void init_commands() noexcept;

  void init_command_cogito() noexcept;
  void init_command_ffi() noexcept;

  void init_command_build() noexcept override;
  void exec_ffi_command() noexcept override;
};

} // namespace compiler
