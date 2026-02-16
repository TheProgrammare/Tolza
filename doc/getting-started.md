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

### I.A Install on Windows

Download LLVM 19 for Windows from:</br>
https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.7

> Note: search for `clang+llvm`

Install the `.exe` installer and **make sure to check**:
- Add LLVM to `PATH`

### I.B Install on Linux / UNIX

> Note: If LLVM 19 is not available in your distribution repositories, install it from the official LLVM releases.  
> Using a newer version may work, but the code **must remain compatible with LLVM 19**.

Official release:  
https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.7

> Note: search for `clang+llvm`

**Debian-based**
```
sudo apt install llvm-19 llvm-19-dev clang-19
```
**Fedora-based**
```
sudo dnf install llvm19 llvm19-devel clang19
```
**Arch-based**
```
sudo pacman -S llvm clang
```
> Arch Linux provides a single system-wide LLVM version

### I.C Install on macOS (Homebrew)

Download LLVM 19 for macOS from:</br>
https://github.com/llvm/llvm-project/releases/tag/llvmorg-19.1.7

> Note: search for `clang+llvm`

### II. Verify installation
Run the following commands:
```
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
Go to File > Preferences > Settings<\br>
or go to down-left cogwheel > Settings

- "editor.formatOnSave": true
- "editor.defaultFormatter": "xaver.clang-format"
