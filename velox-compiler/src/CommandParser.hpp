#pragma once

bool parse_commands(int argc, const char* argv[]);


// %0 version
static constexpr const char* HELP_LIST_COMMANDS =
    R"(
The Velox compiler %0

Command usage: velox <command> [path/name] [options]

Available commands:
  build [path] [options]    Compile the project.
                            If no path is provided, the current directory will be used.
                            (the compiler search velox.config in the current directory)

  create [name] [path]      Create a new workspace folder with the specified name.
                            If no name is provided, the program will prompt for input.
                            If no path is provided, the current directory will be used.

  gui [path]                Open the compiler interface.
                            If no path is provided, the current directory will be used.

Target options:     
  --abi=<abi>               Set the target ABI.
  --arch=<arch>             Set the target architecture.
  --bits=<number>           Set the target bits (e.g., 32, 64).
  --os=<os>                 Set the target operating system.
  --libc=<libc>             Set the target C library.
  --config=<path>           Set the config source file. 
  --config=self             Set the config on velox.config

Profile options:        
  --debug                   Compile in debug mode.
  -d        
  --release                 Compile in release mode.
  -r        
  --opt-level=<level>       Set optimization level (0-3).
  --size-opt                Enable size optimizations.\n

Logs options:       
  --log-all                 Log all passes
  --log-filesystem          Log the filesystem pass        
  --log-lexer               Log the lexer pass    
  --log-preprocessor        Log the preprocessor pass            
  --log-parser              Log the parser pass    
  --log-embinder            Log the EMBinder pass        
  --log-exporter            Log the exporter pass        
  --log-resolver            Log the resolver pass        
  --log-llvm                Log the LLVM IR code generation pass    
  --log-linker              Log the Linker pass    

Preprocessor options:       
  -D<name>[=value]          Define a macro (value defaults to 1).
  -U<name>                  Undefine a macro.\n

Code generation options:        
  --emit-obj|asm|bc|bin     Set the output format (object, assembly, bitcode, binary).\n
  --output=<dir>            Set the general output directory for all generated files (LLVM IR, ASM, OBJ, BC, BIN)
  --dest=<path>             Set the final output file or directory for the main compiled artifact (e.g., executable or LLVM module)

Project options:        
  --project=<dir>           Set the project directory.
  --src=<dir>               Set the source code directory.
  --third-party=<dir>       Set the third party source code directory.

Options:        
  --help                    Display this help message and exit.
  -h        

  --version                 Display the current version of the compiler.
  -v        

  Examples:
  > velox build ./my_project --debug --emit-asm
  > velox create MyProject
  > velox create # will prompt for project name
  > velox gui # will launch the compiler qt interface
)";