# 🏁 Getting Started

To contribute to the Velox programming language, clone this repository and follow the instructions below.

## C++ standard
The C++ standard used here is **C++23**  
(C17 is supported but rarely used).

> Note: The full build configuration is defined in the `CMakeLists.txt` files provided in the repository.

## Recommended compiler
The project is designed to be built with **Clang**.

Velox generates LLVM IR and uses Clang for C language inspection during automatic binding generation.


## Install dependencies
This project depends on:
- `llvm` **version 19**
- `clang` **version 19**

If you have some troubles to install from this document, please, visit official websites:
- clangd: https://clangd.llvm.org/
- ninja: https://ninja-build.org/
- cmake: https://cmake.org/
- llvm: https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.7

### I.A Install on Windows

Download LLVM 19 for Windows
> Note: search for `clang+llvm`

Install the `.exe` installer and **make sure to check**:
- Add LLVM to `PATH`

### I.B Install on Linux / UNIX

**Debian-based**
```
sudo apt install llvm-19 llvm-19-dev clang-19 clangd ninja-build cmake
```
**Fedora-based**
```
sudo dnf install llvm19 llvm19-devel clang19 clangd ninja-build cmake
```
**Arch-based**
```
sudo pacman -S llvm clang clangd ninja cmake
```

> Arch Linux provides a single system-wide LLVM version

> Note: If LLVM 19 is not available in your distribution repositories, install it from the official LLVM releases.  
> Using a newer version may work, but the code **must remain compatible with LLVM 19**.

### I.C Install on macOS (Homebrew)
**brew-command**
```
brew install llvm@19 clang@19 clangd ninja cmake 
```

Or download LLVM 19 for macOS
> Note: search for `clang+llvm`

### II. Verify installation
Run the following commands:
```bash
llvm-config --version
clang --version
```
> It's must returns `19.*` version for both

## Configure your IDE
VS Code is recommended

Configure the compiler to use `clang++-19` (or the equivalent on your system)

Ensure LLVM include paths and libraries are visible to the IDE

### For VS Code IDE

**Install plugins:**
- clangd (LLVM) : C/C++ completion, navigation, and insights
- CMake Tools (Microsoft)
- GitHub Actions (GitHub)
- LLDB DAP (LLVM) : Debugging
- Clang-Format (Xaver Hellauer) : Code formatation for clang
- velox-vscode-plugin-1.0.0.vsix (project plugin) : coloration code + snippets only

**Settings**
Go to File > Preferences > Settings</br>
or go to down-left cogwheel > Settings

- "editor.formatOnSave": true
- "editor.defaultFormatter": "xaver.clang-format"

# Pull Request Rules

## General conventions
Any pull request must be made with some minimal requirements:
- No compilation errors
- No compilation warnings (excepts some LLVM deprecated source code and false positive include unused `pipeline_headers.hpp` `parser_headers.hpp` `ast_headers.hpp`)
- No undefined behavior under the C++23 standard.
- No memory leak
- No unformatted code (use `.clang-format` by running the bash script `format-all-command.sh`)
- No syntax and/or behaviour innovations without a ![RFC template](RFC_TEMPLATE.md) to explain and justify the innovation
- No mixed pull request: one pull request for one concept/innovation/domains, excepts necessary multi coverage
- English only comments (or french with english version)
- No pull request without a ![pull request template](PULL_REQUEST_TEMPLATE.md) to explain your pull request

## Code design
- No reinterpret_cast without a strong justification
- No unjustified and unlimited recursive call
- No unjustified or dangerous macros
- Avoid macros as constants
- Avoid boilerplate
- Avoid nested code logic
- Avoid raw pointer when it's possible
- Prefer the stack and smart pointers
- `weak_ptr` are most of time a reference bridge
- Functions must do one thing
- Too long functions are prohibited
- Use errors handler, see !(error handlers)[error_handlers]
- Avoid cryptic code
- Avoid micro-optimization (clang is smarter in optimization)
- Stay simple
- Code must be explicit: human readable, commented and use `[[nodiscard]]` `[[maybe_unused]]` when it's appropriate
- Use lambda when a function seems too specialized and too tiny (used only in one code section)
- Keep in mind that the multi-threading granularity is on each script `ScriptInfo`

## Naming convention
- files: use snake_case (`-` is for concept linked words like: `llvm-ir` `ffi-json`)
- functions, variables, namespace: use snake_case
- `constexpr` or `static const` constants: prefix `k_` + snake_case
- macros: UPPER_CASE
- Types: PascalCase
- parameters: prefix `p_` + snake_case
- generic types: `T` `U` `V`

## Error Handlers
The project have some error handling
> Note: most of the time, an error contains a error code, message, hint, code localisation (from line+column or from a token)

- lexer: use `add_error`
- parser: use `ctx.tok_v`, the token checking can handle error, you can add errors directly from current token `ctx.tok_v.add_error` or specific token `ctx.tok_v.add_error_tok`
- visitors: use `error_add` for classic visitor error or `error_two_lines` for visitor error on two scripts (e.g. when a imported function is called with wrong arguments)

For custom error report on code, use the class `Error_Diagnostic` from the script `error_output.hpp`
> Any error reported on code must have a unique error code, the `Error_Diagnostic` class only receive static error code

Keep in mind, that the toolchain and the compiler must never failed, they reports only the errors

To see how the pipeline manage errors, check ![Compiler Pipeline](Compiler_Pipeline.md)

### Errors Currently
Currently, any error added will throw a runtime error to inspect any possible error misinterpretation
