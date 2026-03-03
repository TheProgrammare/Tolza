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

bool parse_commands(int argc, const char* argv[]);

bool parse_package(int argc, const char* argv[]);
bool parse_create(bool short_command, int argc, const char* argv[]);
bool parse_gui(int argc, const char* argv[]);
bool parse_build(int argc, const char* argv[]);
bool parse_check(bool short_command, int argc, const char* argv[]);
bool parse_help(int argc, const char* argv[]);
bool parse_version(int argc, const char* argv[]);
bool parse_audit(int argc, const char* argv[]);
void invalid_command();

// %0 version
static constexpr const char* HELP_LIST_COMMANDS =
    R"(
USAGE: velox <command> [path|name|file] [options]

Available commands:
  build [path] [options]    Compile the project.
  b [path] [options]        (alias)
  > If no path is provided, the current directory will be used.
  > (the toolchain search velox.config in the current directory)

  create workspace [name] [path]
  crw [name] [path]         (alias)
                            Create a new workspace directory with name.
  > If no name is provided, the program will prompt for input.
  > If no path is provided, the current directory will be used.

  create config [name] [path]      
  crc [name] [path]         (alias)
                            Create a new .config file.
  > If no path is provided, the current directory will be used.

  gui [path]                Open the toolchain interface.
  ui [path]                 (alias)
  > If no path is provided, the current directory will be used.

  pkg install [package]     Install package from the velox repository.
  pkg remove [package]      Remove package.
  pkg info [package]        Show package description, version, ...
  pkg purge [package]       Remove package configuration.
  pkg check [package]       Check package integrity.
  pkg list                  Show all packages.
  pkg update                Update package cache.
  pkg upgrade               Update all packages.
  pkg clean                 Clean package cache list.
  pkg --help | pkg -h       Display package command helper.

  check workspace [path]    Check the sanity of the workspace.
  chw [path]                (alias)
  > If no path is provided, the current directory will be inspected.

  check config [file]       Check the sanity of the config file.
  chc [file]                (alias)
  > If no file is provided, the first .config will be inspected.

  audit [path]              Produce an audit report of the workspace scripts.
  a [path]                  (alias)
  > If no path is provided, the current directory will be used.
  > (recommended to be used inside a valid workspace)

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

Dot options:
  --dot-ast                 Generate graphviz for AST view.
  --dot-link                Generate graphviz for files linked.

Preprocessor options:       
  -D<name>[=value]          Define a macro (value defaults to 1).
  -U<name>                  Undefine a macro.

Code generation options:        
  --emit=<obj|asm|bc|bin>   Set the output format.
  --build=<dir>             Set the build directory.

Project options:        
  --project=<dir>           Set the project directory.
  --src=<dir>               Set the source code directory.
  --third-party=<dir>       Set the third party source code directory.
  --ffi-json=<dir>          Set the interop json ast directory.

Sanity options:
  --full                    Set the sanity checker in full mode.
  > (will inspect all sections and keys presence)

Options:        
  --help      | -h          Display this help message and exit.
  --version   | -v          Display the current version of the toolchain.
  --!<option> | -!<option>  Desactivate a bool/flag option.

package options:
  --installed               Show only installed package in list.
  --upgradable              Show only upgradable package in list.

Examples:
  $ velox build ./my_project --debug --emit=asm
  $ velox create MyProject
  $ velox create # will prompt for project name
  $ velox gui # will launch the toolchain qt interface
  $ velox pkg install scientific
  $ velox pkg list --upgradable
)";

static constexpr const char* HELP_LIST_PKG_COMMANDS =
    R"(
USAGE: velox pkg <command>

Available commands:
  pkg install [package]     Install package from the velox repository.
  pkg remove [package]      Remove package.
  pkg info [package]        Show package description, version, ...
  pkg purge [package]       Remove package configuration.
  pkg check [package]       Check package integrity.
  pkg list                  Show all packages.
  pkg update                Update package cache.
  pkg upgrade               Update all packages.
  pkg clean                 Clean package cache list.
  pkg --help | pkg -h       Display package command helper.

Options:
  --installed               Show only installed package in list.
  --upgradable              Show only upgradable package in list.

Examples:
  $ velox pkg install scientific
  $ velox pkg list --upgradable
)";