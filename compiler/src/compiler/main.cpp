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

#include "compiler.hpp"
#include "compiler/compiler.hpp"
#include "compiler/parser_command.hpp"

#include <CLIUtils/CLI11.hpp>
#include <common/common.hpp>
#include <print>

int run_tolza(int argc, const char* argv[])
{
  CLI::App            app{std::format("Tolza compiler ({})", SOFTWARE_VERSION), SOFTWARE_NAME};
  compiler::Commander commander(app, argc, argv);

  CLI11_PARSE(app, argc, argv);

  if (compiler::COMPILER.run_requested) return compiler::COMPILER.start_compilation() ? 0 : 1;

  return 0;
}

int main(int argc, const char* argv[])
{
  try {
    return run_tolza(argc, argv);
  } catch (const std::exception& e) {
    std::println(stderr, "fatal error: {}", e.what());
  } catch (...) {
    std::println(stderr, "unknown fatal error");
  }

  return 1;
}
