#pragma once

#include <string>


namespace common
{

std::string get_local_data_dir();
std::string get_templates_dir();

std::string get_compilers_dir();
std::string get_stdlib_dir();
std::string get_packages_dir();
std::string get_config_dir();


inline constexpr char DETECTED_OS_NAME[] =
#ifdef _WIN32
    "windows";
#elif __APPLE__
    "mac";
#elif __linux__
    "linux";
#else
    "unknown";
#endif

inline constexpr char DETECTED_ARCH[] =
#if defined(__x86_64__) || defined(_M_X64)
    "amd64";
#elif defined(__i386) || defined(_M_IX86)
    "x86";
#elif defined(__aarch64__)
    "arm64";
#elif defined(__arm__)
    "arm";
#else
        "unknown";
#endif

inline constexpr char DETECTED_BITS[] =
#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
    "64";
#elif defined(__i386) || defined(_M_IX86) || defined(__arm__)
    "32";
#else
    "0";
#endif

inline constexpr char DETECTED_ABI[] =
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__)
    "LP64";
#elif defined(__ILP32__) || defined(__i386)
    "ILP32";
#else
    "unknown";
#endif


constexpr const char* HELP_LIST =
    R"(
usage: velox <command> [path/name/file] [--options/-options]

legend:
  command                   Description
  <argument>                Mandatory argument.
  [argument]                Optional argument.
  > info                    Command information.
  . command                 Command variation.
  --version                 Long option.
  -v                        Short option. 

Build commands:
  build [path] [options]    Compile the project.
  . b [path] [options]      
  > If no path provided, the current directory will be used.
  > (the toolchain search velox.config in the current directory)

  build -help, -h         Display build command helper.

  generate-ffi-json <target_dir> <dest_dir> 
  . gen-ffi <target_dir> <dest_dir> 
                            Translate .json ast ffi to .vlxb wrappers.

Compiler manager commands:
  compiler <command>        Compiler subcommand.
  . c                         

  compiler find [path] [options]          
                            Search of velox-compiler.
  . compiler find --all     Search all velox-compiler installed.
  . compiler find --version=<yyyy-mm-dd>       
                            Find specific velox-compiler version.
  . compiler find --lastest Search the lastest velox-compiler.

  compiler set [path] [option]
                            Set the compiler used in the toolchain.confg file.
  . compiler set --lastest  Set the lastest velox-compiler used.
  . compiler set --version=<yyyy-mm-dd>
                            Set specific velox-compiler version used.

  compiler --version, -v    Display the current compiler version used.

Workspace commands:                          
  create workspace [name] [path]
  . crw [name] [path]         
                            Create a new workspace directory with name.
  > If no name is provided, the program will prompt for input.
  > If no path provided, the current directory will be used.

  create config [name] [path]      
  . crc [name] [path]
                            Create a new .config file.
  > If no path provided, the current directory will be used.

  gui [path]                Open the toolchain interface.
  . ui [path]
  > If no path provided, the current directory will be used.

  check workspace [path]    Check the check of the workspace.
  . chw [path]
  > If no path provided, the current directory will be inspected.

  check config [file]       Check the check of the config file.
  . chc [file]
  > If no file is provided, the first .config will be inspected.

  audit [path]              Produce an audit report of the workspace scripts.
  . a [path]
  > If no path provided, the current directory will be used.
  > (recommended to be used inside a valid workspace)

Package manager commands:
  pkg install [package]     Install package from the velox repository.
  pkg remove [package]      Remove package.
  pkg info [package]        Show package description, version, ...
  pkg purge [package]       Remove package configuration.
  pkg check [package]       Check package integrity.
  pkg list                  Show all packages.
  pkg update                Update package cache.
  pkg upgrade               Update all packages.
  pkg clean                 Clean package cache list.
  pkg --help, -h            Display package command helper.


Build target options:     
  --abi=<abi>               Set the target ABI.
  --arch=<arch>             Set the target architecture.
  --bits=<number>           Set the target bits (e.g., 32, 64).
  --os=<os>                 Set the target operating system.
  --libc=<libc>             Set the target C library.
  --config=<path>           Set the config source file. 
  --config=self             Set the config on velox.config

Build Profile options:        
  --debug, -d               Compile in debug mode.
  --release, -r             Compile in release mode.
  --opt-level=0..3          Set optimization level.
  . -O0..3                  Set optimization level.
  --size-opt, -Os           Enable size optimizations.
  --exterm-size-opt, -Oz    Enable extrem size optimizations.

Build Logs options:       
  --log-all, -lall          Log all passes.
  --log-filesystem, -lfs    Log the filesystem pass.
  --log-lexer, -llex        Log the lexer pass.
  --log-pre, -lpre          Log the preprocessor pass.
  --log-parser, -lpar       Log the parser pass.
  --log-binder, -lb         Log the binder pass.
  --log-exporter, -lexp     Log the exporter pass.
  --log-resolver, -lres     Log the resolver pass.
  --log-llvm, -lllvm        Log the LLVM IR code generation pass.
  --log-linker, -llink      Log the Linker pass.

Build Warnings options:
  --warn-all, -wall         Warn all cases (override all).
  --warn-extra, -wext       Warn more specific cases.
  --warn-pedantic, -wpe     Warn standard derivation.
  --warn-level=0..3         Warn sensibility.
  --warn-unused, -wun       Warn unused var, fn, ...
  --warn-dead-code, -wdc    Warn dead code, never used...
  --warn-as-error, -wae     All warnings treated as errors.

Build debug options:
  --print-ast               Generate a file view of ast.

Build preprocessor options:       
  -D<name>[=value]          Define a macro (value defaults to 1).
  -U<name>                  Undefine a macro.

Build Code generation options:        
  --emit-obj                Emit object .o files.
  --emit-asm                Emit assembly .asm files.
  --emit-bc                 Emit byte code .bc file.
  --emit-bin                Emit binary file.
  --emit-llvm               Emit llvm-ir .ll files.
  --build=<dir>             Set the build directory.

Build project options:        
  --project=<dir>           Set the project directory.
  --src=<dir>               Set the source code directory.
  --vendor=<dir>            Set the vendor source code directory.
  --ffi-json=<dir>          Set the interop json ast directory.
  --binding=<dir>           Set the binding directory.

Compiler manager options:
  --lastest, -l             Find or set the lastest velox-compiler.
  --all                     Find all velox-compiler installed.
  --version=<yyyy-mm-dd>    Find or set specific velox-compiler version.

Workspace check options:
  --full                    Set the check checker in full mode.
  > (will inspect all sections and keys presence)

Toolchain options:        
  --help, -h                Display this help message and exit.
  --version, -v             Display the current version of the toolchain.
  --!<option>, -!<option>   Desactivate a bool/flag option.

Package manager options:
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

constexpr const char* HELP_LIST_PKG =
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


constexpr const char* HELP_LIST_BUILD =
    R"(
usage: velox build <command>

Build commands:
  build [path] [options]    Compile the project.
  . b [path] [options]
  > If no path provided, the current directory will be used.
  > (the toolchain search velox.config in the current directory)

  generate-ffi-json <target_dir> <dest_dir> 
  . gen-ffi <target_dir> <dest_dir> 
                            Translate .json ast ffi to .vlxb wrappers.
  

Build target options:     
  --abi=<abi>               Set the target ABI.
  --arch=<arch>             Set the target architecture.
  --bits=<number>           Set the target bits (e.g., 32, 64).
  --os=<os>                 Set the target operating system.
  --libc=<libc>             Set the target C library.
  --config=<path>           Set the config source file. 
  --config=self             Set the config on velox.config

Build Profile options:        
  --debug, -d               Compile in debug mode.
  --release, -r             Compile in release mode.
  --opt-level=0..3          Set optimization level.
  . -O0..3                  Set optimization level.
  --size-opt, -Os           Enable size optimizations.
  --exterm-size-opt, -Oz    Enable extrem size optimizations.

Build Logs options:       
  --log-all, -lall          Log all passes.
  --log-filesystem, -lfs    Log the filesystem pass.
  --log-lexer, -llex        Log the lexer pass.
  --log-pre, -lpre          Log the preprocessor pass.
  --log-parser, -lpar       Log the parser pass.
  --log-binder, -lb         Log the binder pass.
  --log-exporter, -lexp     Log the exporter pass.
  --log-resolver, -lres     Log the resolver pass.
  --log-llvm, -lllvm        Log the LLVM IR code generation pass.
  --log-linker, -llink      Log the Linker pass.

Build Warnings options:
  --warn-all, -wall         Warn all cases (override all).
  --warn-extra, -wext       Warn more specific cases.
  --warn-pedantic, -wpe     Warn standard derivation.
  --warn-level=0..3         Warn sensibility.
  --warn-unused, -wun       Warn unused var, fn, ...
  --warn-dead-code, -wdc    Warn dead code, never used...
  --warn-as-error, -wae     All warnings treated as errors.

Build debug options:
  --print-ast               Generate a file view of ast.

Build preprocessor options:       
  -D<name>[=value]          Define a macro (value defaults to 1).
  -U<name>                  Undefine a macro.

Build Code generation options:        
  --emit-obj                Emit object .o files.
  --emit-asm                Emit assembly .asm files.
  --emit-bc                 Emit byte code .bc file.
  --emit-bin                Emit binary file.
  --emit-llvm               Emit llvm-ir .ll files.
  --build=<dir>             Set the build directory.

Build project options:        
  --project=<dir>           Set the project directory.
  --src=<dir>               Set the source code directory.
  --vendor=<dir>            Set the vendor source code directory.
  --ffi-json=<dir>          Set the interop json ast directory.
  --binding=<dir>           Set the binding directory.

Examples:
  $ velox build ./my_project --debug --emit=asm
)";


} // namespace common