<div align="center">

![Velox logo](logo/velox-logo-128.svg) 

# Velox</br> TECHNICAL IMPLEMENTATION

</div>

A hybrid-level programming language designed to emphasize safety and simplicity through innovative syntax and paradigms.

This project has been under design and experimentation since 2024.  
The public repository reflects the current implementation.

Velox is currently in an experimental stage and should be considered a work in progress.

This TECHNICAL IMPLEMENTATION presents the vision and core ideas behind the language.  
Some features and syntax are still under development.

Velox is a next-generation programming language. Inspired by modern systems programming practices and component-oriented architectures, Velox enables developers to write efficient, maintainable, and scalable code with minimal boilerplate and minimal undefined behaviour through memory and code logic.

> Explicit Inspirations: ![Rust](https://github.com/rust-lang/rust) - ![C](https://github.com/llvm/llvm-project) - ![C++](https://github.com/llvm/llvm-project) - ![Python](https://github.com/python) - ![COBOL](https://github.com/dscobol/Cobol-Projects) - ![FORTAN](https://github.com/fortran-lang) - ![Lisp](https://github.com/topics/common-lisp?l=scheme&o=desc&s=) - ![Zen-C](https://github.com/z-libs/Zen-C)</br>
The Zen-C programming language demonstrates a clear commitment to modernizing C, offering a safer language that aligns with modern programming practices. We are following the progress of this project with great interest, as it shares some of the goals we are pursuing in our own language.

> To see the language naming and mangling, see ![NAMING](docs/NAMING.md)</br>
> To see the language metacode logic, see ![METACODE](docs/METACODE.md)

## Copiler Status
![Compiler](https://img.shields.io/badge/Compiler-Work_in_progress-brightgreen)</br>
![Compilation_Pipeline](https://img.shields.io/badge/Compilation_Pipeline-Done-darkgreen)
![Lexer](https://img.shields.io/badge/Lexer-Done-darkgreen)
![Preprocessor](https://img.shields.io/badge/Preprocessor-Done-darkgreen)
![Parser](https://img.shields.io/badge/Parser-Work_in_progress-brightgreen)
![EMBinder](https://img.shields.io/badge/EMBinder-Done-darkgreen)
![Exporter](https://img.shields.io/badge/Exporter-Done-darkgreen)
![ResolverSymbol](https://img.shields.io/badge/Resolver_Symbol-Work_in_progress-brightgreen)
![ResolverType](https://img.shields.io/badge/Resolver_Type-Work_in_progress-brightgreen)
![ResolverSemantic](https://img.shields.io/badge/Resolver_Semantic-Work_in_progress-brightgreen)
![LLVMGeneration](https://img.shields.io/badge/LLVM_Generation-Work_in_progress-brightgreen)</br>
## Tools Status
![Highlighter](https://img.shields.io/badge/Highlighter-Done-darkgreen)
![Snippet](https://img.shields.io/badge/Snippet-Done-darkgreen)
![Compilation_Dot](https://img.shields.io/badge/Compilation_debug_dot_graph-Done-darkgreen)

## Dependencies
This project uses the following open-source libraries:

- https://json.nlohmann.me – for reading and writing JSON syntax.
- https://github.com/marzer/tomlplusplus - for reading and writing toml syntax.
- https://github.com/CLIUtils/CLI11 - for CLI commands.


> Thanks to the authors for their excellent open-source work!</br>
> header-only, no installation required

# WIKI

## A. Procedural Programming
1. [variables](docs/wiki/variable.md)
2. [operators](docs/wiki/operators.md)	
3. [control flow](docs/wiki/flow.md)
4. [pattern matching](docs/wiki/pattern.md)

## B. Functional Programming
1. [functions](docs/wiki/function.md)
2. [parameters](docs/wiki/parameter.md)
2. [closures](docs/wiki/closure.md)

## C. Compositional Oriented Programming
0. [paradigm](docs/wiki/cop.md)
1. [components](docs/wiki/component.md)
2. [entities](docs/wiki/entity.md)
3. [systems](docs/wiki/system.md)

## D. Type System
1. [types](docs/wiki/type.md)
2. [textual](docs/wiki/textual.md)
3. [tuples](docs/wiki/tuple.md)
4. [tables](docs/wiki/table.md)
5. [flags](docs/wiki/flag.md)
6. [unions](docs/wiki/union.md)
7. [enumerators](docs/wiki/enumerator.md)
8. [generics](docs/wiki/generic.md)

## E. Memory Model
1. [stack](docs/wiki/memory_stack.md)
2. [heap](docs/wiki/memory_heap.md)

## F. Modules & Tooling
1. [modules](docs/wiki/module.md)
2. [metacodes](docs/wiki/metacode.md)
3. [script binding](docs/wiki/bindgen.md)

## G. Runtime
1. [input output](docs/wiki/io.md)
2. [threading](docs/wiki/threading.md)

## H. Low Level
1. [assembly](docs/wiki/asm.md)	       

