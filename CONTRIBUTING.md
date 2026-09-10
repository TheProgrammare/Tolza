<div align=center>
  
  ![🇫🇷 Français](docs/CONTRIBUTING_FR.md)

  # CONTRIBUTING

</div>

# How to contribute ?
Before consedering to contribute to this project, please read some documentations:
- ![Contributing](CONTRIBUTING.md) (this document)
- ![Project Presentation](README.md) (for all)
- ![Technical Implementation](MANIFEST.md) (for paradigms, concepts, syntax, behaviour)
- The source code preliminarily (for code contributors)

## Contribute to the code
1. Clone this repository to your machine
2. Follow ![Contributing Code](docs/CONTRIBUTING_CODE.md) software installation
3. Contribute to the code by following ![Pull Request Rules](docs/CONTRIBUTING_CODE.md#Pull-Request-Rules)
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

| my skill | my utility for tolza |
|-|-|
| junior programmer | better innocent understanding of overly complex concepts, better simplified approach, closer to spontaneous understanding |
| senior programmer | good for the code review, experience about paradigm and concepts, better user experience approach |
| non programmer | good for educative/novice approach, for explicit documentation |
| linguist | pertinent for the code syntax, the human readable approach |
| manager | good view for scalable projects, pertinent workflow and workspace | 
| marketing | competent for the language promotion, can create slogans and memory check for the learning | 

# Project Objectives
**Summary** This project aims to address practical and concrete needs in a reasonable way, through both the Tolza language and its toolchain. 

### Language (Tolza) 
- A **deterministic, predictable, and explicit** programming language with a **simple and modern syntax**.
- The language’s **native paradigms** should be useful without introducing excessive complexity.
- **Abstraction** is valuable, but programmers must understand broadly what is happening. Abstraction is therefore achieved through **code simplicity and clarity**.

### Feature Sectorization 
- **Highly practical and widespread features** should be considered **native to the language**. Example: fixed-point floats (deci, udeci) for specialized domains can be included natively.
- **Practical and widespread but complex features**, requiring intermediate scripts, should be included in the **stdlib** and integrated into the core to allow **pseudo-native usage**.
- **Widespread and optimized features**, useful in general domains, should go into the **stdlib**.
- **Specialized, heavy, or non-essential features** should be provided as **separate packages**.

### Toolchain
- Provide a **ready-to-use language**, suitable for **small projects** as well as **industrial-scale projects**.
- The toolchain should preferably be used in a **workspace environment**.
- `tolza.toml` is the **reference file**, specifying compiler behavior, target directories, profiles composition, and defining the workspace via folder locations.
- The toolchain is not basic but must remain **simple, practical, and intuitive**.

# Project Structure
This project contains several sub-projects:
- Compiler
  - Lexer
  - Parser
  - FFI Binder
  - Resolver
  - Codegen
  - ...
- Toolchain
  - Commands
  - Package Manager
  - Workspace Manager
- Common
  - Compiler Options
  - utils functions
  - some CLI commands
  - filesystem
  - some conventions
- Visual studio code plugin: [here](https://github.com/TheProgrammare/Tolza-VScode-Plugin)

Check documentation [here](/docs/wiki) for more information and schematic representation

# Governance
- **Founder**:  
  The founder has the final say on all decisions regarding the language, compiler, and toolchain, as well as design, concept, syntax, and pull requests. This ensures a **strong overall coherence** and clear project direction, avoiding excessive complexity as seen in some committees.

- **Community**:  
  The community can propose improvements or changes to any part of the project. Proposals are regularly reviewed by the most active or prominent members. Interesting proposals, even if less visible, may be put on the agenda if they appear sufficiently relevant.

- **Purpose of this governance**:  
  Prevent fragmentation in the development of the language, compiler, and toolchain, while maintaining **overall coherence**, simplicity, and alignment with the project’s objectives.
