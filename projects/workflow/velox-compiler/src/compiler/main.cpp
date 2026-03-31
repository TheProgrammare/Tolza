/*
 *	The Velox programming language - Apache License, Version 2.0
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

#include <CLIUtils/CLI11.hpp>
#include "compiler/parser_command.hpp"
#include "common.hpp"

int main(int argc, const char* argv[])
{
  CLI::App app{"Velox compiler (" + common::SOFTWARE_VERSION + ")", common::SOFTWARE_NAME};
  Command  command(app, argc, argv);


  CLI11_PARSE(app, argc, argv);
}
