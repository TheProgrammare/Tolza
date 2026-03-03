# How to contribute ?
Before consedering to contribute to this project, please read some documentations:
- ![Contribute](CONTRIBUTE.md) (this document)
- ![Project Presentation](README.md) (for all)
- ![Technical Implementation](MANIFEST.md) (for paradigms, concepts, syntax, behaviour)
- The source code preliminarily (for code contributors)

## Contribute to the code
1. Clone this repository to your machine
2. Follow ![🏁 Getting Started](🏁_Getting_Started) software installation
3. Contribute to the code by following ![Pull Request Rules](Pull_Request_Rules)
4. Write a ![pull request template](docs/PULL_REQUEST_TEMPLATE.md) to explain the pull request
>  If you make an innovation, add a ![RFC template](docs/RFC_TEMPLATE.md) to explain and justify the innovation
5. Make your pull resquest

## Contribute to the philosophy
1. Makes sure that your suggestion is not duplicated or already rejected
2. Write a ![Philosophical template](docs/PHILOSOPHICAL_TEMPLATE.md) to explain your suggestion
3. Don't hesitate to integrate graphical representation, examples, pseudo code, ...
4. Exchange ideas and discuss with the community to refine your proposal, or even turn it into a community proposal.

# Who can contribute ?
Any volunteer, but here are a few ideas on how you can make yourself useful based on your skills

| my skill | my utility for velox |
|-|-|
| junior programmer | better innocent understanding of overly complex concepts, better simplified approach, closer to spontaneous understanding |
| senior programmer | good for the code review, experience about paradigm and concepts, better user experience approach |
| non programmer | good for educative/novice approach, for explicit documentation |
| linguist | pertinent for the code syntax, the human readable approach |
| manager | good view for scalable projects, pertinent workflow and workspace | 

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
- No syntax and/or behaviour innovations without a ![RFC template](docs/RFC_TEMPLATE.md) to explain and justify the innovation
- No mixed pull request: one pull request for one concept/innovation/domains, excepts necessary multi coverage
- English only comments (or french with english version)
- No pull request without a ![pull request template](docs/PULL_REQUEST_TEMPLATE.md) to explain your pull request

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

To see how the pipeline manage errors, check ![Compiler Pipeline](docs/Compiler_Pipeline.md)

### Errors Currently
Currently, any error added will throw a runtime error to inspect any possible error misinterpretation

# Project Objectives

## English
**Summary**

This project aims to address practical and concrete needs in a reasonable way, through both the Velox language and its toolchain.

### Language (Velox)
- A **deterministic, predictable, and explicit** programming language with a **simple and modern syntax**.
- The language’s **native paradigms** should be useful without introducing excessive complexity.
- **Abstraction** is valuable, but programmers must understand broadly what is happening. Abstraction is therefore achieved through **code simplicity and clarity**.

### Feature Sectorization
- **Highly practical and widespread features** should be considered **native to the language**.  
  Example: fixed-point floats (`deci`, `udeci`) for specialized domains can be included natively.
- **Practical and widespread but complex features**, requiring intermediate scripts, should be included in the **stdlib** and integrated into the core to allow **pseudo-native usage**.
- **Widespread and optimized features**, useful in general domains, should go into the **stdlib**.
- **Specialized, heavy, or non-essential features** should be provided as **separate packages**.

### Toolchain
- Provide a **ready-to-use language**, suitable for **small projects** as well as **industrial-scale projects**.
- The toolchain should preferably be used in a **workspace environment**.
- `velox.config` is the **reference file**, specifying compiler behavior, target directories, and defining the workspace via folder locations.
- The toolchain is not basic but must remain **simple, practical, and intuitive**.


## Français
**Résumé**

Ce projet a pour objectif de répondre à des besoins raisonnables sur des cas concrets et pratiques, à la fois via le langage Velox et sa toolchain.  

### Langage (Velox)
- Langage de programmation **déterministe, prévisible et explicite**, avec une **syntaxe simple et moderne**.
- Les **paradigmes natifs** du langage doivent être utiles sans introduire une complexité excessive.
- L’**abstraction** est appréciée, mais le programmeur doit pouvoir comprendre globalement ce qui se passe. L’abstraction passe donc par la **simplicité et la clarté du code**.

### Sectorisation des fonctionnalités
- Les fonctionnalités **très pratiques et répandues** doivent être considérées comme **natives au langage**.  
  Exemple : les flottants fixes (`deci`, `udeci`) pour un domaine spécialisé peuvent être intégrés directement.
- Les fonctionnalités **pratiques et répandues mais complexes**, nécessitant des scripts intermédiaires, doivent être intégrées dans la **stdlib** et **accessibles comme si elles étaient natives**.
- Les fonctionnalités **répandues et optimisées**, utiles dans des domaines généraux, doivent être mises dans la **stdlib**.
- Les fonctionnalités **spécialisées, lourdes ou non essentielles** doivent être mises dans des **packages séparés**.

### Toolchain
- Fournir un langage **clé en main**, adapté autant aux **petits projets** qu’aux **projets industriels**.
- La toolchain est à utiliser de préférence dans un **espace de travail**.
- `velox.config` est le **fichier de référence**, indiquant le comportement du compilateur, les dossiers cibles, et définissant l’espace de travail par la localisation des dossiers.
- La toolchain n’est pas basique mais doit demeurer **simple, pratique et intuitive**.


# Governance

## English

- **Founder**:  
  The founder has the final say on all decisions regarding the language, compiler, and toolchain, as well as design, concept, syntax, and pull requests. This ensures a **strong overall coherence** and clear project direction, avoiding excessive complexity as seen in some committees.

- **Community**:  
  The community can propose improvements or changes to any part of the project. Proposals are regularly reviewed by the most active or prominent members. Interesting proposals, even if less visible, may be put on the agenda if they appear sufficiently relevant.

- **Purpose of this governance**:  
  Prevent fragmentation in the development of the language, compiler, and toolchain, while maintaining **overall coherence**, simplicity, and alignment with the project’s objectives.

## Français

- **Fondateur** :  
  Le fondateur a le dernier mot sur toutes les décisions liées au langage, au compilateur, à la toolchain, ainsi qu’au design, au concept, à la syntaxe et aux pull requests. Cela garantit une **cohérence globale solide** et la direction claire du projet, évitant les complexifications excessives observées dans certains comités.

- **Communauté** :  
  La communauté peut proposer des améliorations ou des modifications sur n’importe quel aspect du projet. Ces propositions sont auditées régulièrement par les membres les plus actifs ou mis en avant. Des propositions intéressantes, même moins visibles, peuvent être mises à l’ordre du jour si elles semblent suffisamment pertinentes.

- **Objectif de cette gouvernance** :  
  Éviter la dispersion dans le développement du langage, du compilateur et de la toolchain, tout en maintenant la **cohérence globale**, la simplicité et l’alignement avec les objectifs du projet.

