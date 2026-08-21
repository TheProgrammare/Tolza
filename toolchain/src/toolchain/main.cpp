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

#include <exception>
#include <print>

#include <CLIUtils/CLI11.hpp>

#include <common/common.hpp>
#include "common/environment.hpp"
#include "toolchain/parser_command.hpp"
#include "toolchain/toolchain.hpp"
#include <common/toolchain_options.hpp>


int run_toolchain(int argc, const char* argv[])
{
  common::toolchain::init_toolchain_context();

  CLI::App             app{std::format("Tolza toolchain ({})", SOFTWARE_VERSION), "tolza"};
  toolchain::Commander commander(app);

  toolchain::link_stdlib();

  CLI11_PARSE(app, argc, argv);

  return 0;
}

int main(int argc, const char* argv[])
{
  try {
    return run_toolchain(argc, argv);
  } catch (const std::exception& e) {
    std::println(stderr, "fatal error: {}", e.what());
  } catch (...) {
    std::println(stderr, "unknown fatal error");
  }

  return 0;
}
