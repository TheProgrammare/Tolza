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

#pragma once

bool parse_command(int argc, const char* argv[]);
bool generate_ffi_json_command(int argc, const char* argv[]);
bool help_command();
bool version_command();
void invalid_command();

constexpr const char* HELP_LIST_COMMANDS =
    R"(
USAGE: velox-compiler <command> [options]

Available commands:
  build [options]           Compile the project.
  b [options]               (alias)
  > If no --src= is provided, the current directory will be used.

  generate-ffi-json <target_dir> <dest_dir> 
                            Translate .json ast ffi to .vlxb wrappers.
  gen-ffi <target_dir> <dest_dir> 
                            (alias)

Target options:     
  --abi=<abi>               Set the target ABI.
  --arch=<arch>             Set the target architecture.
  --bits=<number>           Set the target bits (e.g., 32, 64).
  --os=<os>                 Set the target operating system.
  --libc=<libc>             Set the target C library.
  --config=<path>           Set the config source file. 
  --config=self             Set the config on velox.config

Profile options:        
  --debug          | -d     Compile in debug mode.
  --release        | -r     Compile in release mode.
  --opt-level=0..3          Set optimization level.
  --size-opt       | -sopt  Enable size optimizations.

Logs options:       
  --log-all        | -lall  Log all passes.
  --log-filesystem | -lfs   Log the filesystem pass.
  --log-lexer      | -llex  Log the lexer pass.
  --log-pre        | -lpre  Log the preprocessor pass.
  --log-parser     | -lpar  Log the parser pass.
  --log-binder     | -lb    Log the binder pass.
  --log-exporter   | -lexp  Log the exporter pass.
  --log-resolver   | -lres  Log the resolver pass.
  --log-llvm       | -lllvm Log the LLVM IR code generation pass.
  --log-linker     | -llink Log the Linker pass.

Warnings options:
  --warn-all       | -wall  Warn all cases (override all).
  --warn-extra     | -wext  Warn more specific cases.
  --warn-pedantic  | -wpe   Warn standard derivation.
  --warn-level=0..3         Warn sensibility.
  --warn-unused    | -wun   Warn unused var, fn, ...
  --warn-dead-code | -wdc   Warn dead code, never used...
  --warn-as-error  | -wae   All warnings treated as errors.

AST options:
  --print-ast               Generate a file view of ast.

Preprocessor options:       
  -D<name>[=value]          Define a macro (value defaults to 1).
  -U<name>                  Undefine a macro.

Code generation options:        
  --emit=<obj|asm|bc|bin>   Set the output format.
  --build=<dir>             Set the build directory.

Project options:        
  --project=<dir>           Set the project directory.
  --src=<dir>               Set the source code directory.
  --vendor=<dir>            Set the vendor source code directory.
  --ffi-json=<dir>          Set the interop json ast directory.
  --binding=<dir>           Set the binding directory.

Options:        
  --help      | -h          Display this help message and exit.
  --version   | -v          Display the current version of the toolchain.
  --!<option> | -!<option>  Desactivate a bool/flag option.

Examples:
  $ velox-compiler build ./my_project --debug --emit=asm
  $ velox-compiler gen-ffi ./ffi-json ./binding
)";